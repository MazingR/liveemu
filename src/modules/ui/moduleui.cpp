#include <moduleui.hpp>
#include <rendering/modulerenderer.hpp>
#include <rendering/renderresourceshandler.hpp>

#include <common/serializable.hpp>
#include <common/maths.hpp>
#include <queue>

void OnScriptFileChanged(FeEFileChangeType::Type eChangeType, const char* szPath, void* pUserData)
{
	FeSetLastError("");
	((FeModuleUi*)pUserData)->ReloadScripts();
}

void FeModuleUi::TraverseElements(FeScriptFile& script, FeUiElementTraversalList& traversal)
{
	std::queue<FeUiElementTraversalNode*> uiElements;

	for (auto& panel : script.GetPanels())
	{
		FeUiElementTraversalNode& node = traversal.Nodes.Add();
		node.Current = &panel;
		node.Parent = NULL;

		uiElements.push(&node);
	}

	while (uiElements.size())
	{
		FeUiElementTraversalNode* pNode = uiElements.front();
		FeUiElement* pElement = pNode->Current;

		uiElements.pop();

		for (auto& child : pElement->GetChildren())
		{
			FeUiElementTraversalNode& childNode = traversal.Nodes.Add();

			childNode.Current = child.Ptr;
			childNode.Parent = pElement;

			uiElements.push(&childNode);
			pNode->Children.Add(&childNode);
		}
	}
}
uint32 FeModuleUi::ReloadScripts()
{
	auto pRenderingModule = FeApplication::StaticInstance.GetModule<FeModuleRendering>();
	auto pResourcesHandler = FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();

	// Clear stuff
	pRenderingModule->UnloadEffects();
	ScriptFiles.Clear();
	RenderingInstances.Clear();
	Panels.Clear();
	TraversalList.Nodes.Clear();

	pResourcesHandler->UnloadResources();

	// Load scripts from files
	FeTArray<FePath> files;
	files.SetHeapId(RENDERER_HEAP);

	FeFileTools::ListFilesRecursive(files, "themes/common", ".*\\.fes");
	FeFileTools::ListFilesRecursive(files, "themes/default", ".*\\.fes"); // load default theme

	for (auto& file : files)
	{
		FeScriptFile& scriptFile = ScriptFiles.Add();
		auto iRes = FeJsonParser::DeserializeObject(scriptFile, file);

		if (iRes == FeEReturnCode::Success)
		{
			pRenderingModule->LoadEffects(scriptFile.GetEffects());
			for (auto& panel : scriptFile.GetPanels())
			{
				Panels.Add(&panel);
			}
		}
		else
		{
			ScriptFiles.PopBack();
		}
	}
	for (auto& script : ScriptFiles)
	{
		TraverseElements(script, TraversalList); // add all ui elments to flat list

		// Create font resrouces
		for (auto& uiFont : script.GetFonts())
		{
			FeRenderLoadingResource resource;
			resource.Type = FeEResourceType::Font;
			resource.Path = uiFont.GetTrueTypeFile();
			
			resource.Id = FeStringTools::GenerateUIntIdFromString(uiFont.GetName().Cstr());
			resource.Interface = FeCreateRenderResourceInterface<FeRenderFont>();
			auto* pFontData = (FeRenderFont*)resource.Interface->GetData();
			pFontData->Size = uiFont.GetSize();
			pFontData->TrueTypeFile = uiFont.GetTrueTypeFile();

			pResourcesHandler->LoadResource(resource);
		}
	}

	// Pre compute rendering instances
	for (auto& uiNode : TraversalList.Nodes)
	{
		FeUiElement* pCurrent = uiNode.Current;
		FeUiElement* pParent = uiNode.Parent;

		FeUiRenderingInstance& renderingInstance = RenderingInstances.Add(); // create rendering instance
		uiNode.RenderInstance = &renderingInstance;

		renderingInstance.Owner = pCurrent;
		renderingInstance.Geometry.Effect = pCurrent->GetEffect().Id();
		renderingInstance.Geometry.Geometry = FeGeometryHelper::GetStaticGeometry(FeEGemetryDataType::Quad);
		renderingInstance.Geometry.Textures.Clear();

		FeVector3	t = pCurrent->GetTransform().Translation;
		FeRotation	r = pCurrent->GetTransform().Rotation;
		FeVector3	s = pCurrent->GetTransform().Scale;

		if (pParent) // compute world transformation using parent m matrix
		{
			auto& pT = pParent->GetTransform().Translation;
			auto& pR = pParent->GetTransform().Rotation;
			auto& pS = pParent->GetTransform().Scale;

			for (uint32 i = 0; i < 3; ++i)
			{
				t[i] += pT[i];
				r[i] += pR[i];
				s[i] *= pS[i];
			}
		}
		FeGeometryHelper::ComputeAffineTransform(renderingInstance.Geometry.Transform, t, r, s);
	}
	
	// Apply data binding (texture, font, text,...)
	for (auto& uiNode : TraversalList.Nodes)
	{
		for (auto& dataBind : uiNode.Current->GetBindings())
		{
			FeString sourceData = FetchBindingSourceData(dataBind.GetSource());

			if (!sourceData.IsEmpty())
			{
				ApplyBindingToTargetProperty(uiNode, sourceData, dataBind.GetTarget());
			}
		}
	}

	return FeEReturnCode::Success;
}
void FeModuleUi::ApplyBindingToTargetProperty(FeUiElementTraversalNode& node, const FeString& sourceData, const FeUiBinding& targetBinding)
{
	auto pResourcesHandler = FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();

	FeUiElementTraversalNode* pTarget = &node;
	
	const FeTArray<FeString>& targetPath = targetBinding.GetPath();
	const FeString& targetProperty = targetBinding.GetProperty();

	// find target element from path
	if (targetPath.GetSize() > 0)
	{
		uint32 iDepth = 0;

		while (iDepth < targetPath.GetSize())
		{
			bool bFoundElement = false;

			for (auto& child : pTarget->Children)
			{
				if (child->Current->GetName() == targetPath[iDepth])
				{
					iDepth++;
					pTarget = child;
					bFoundElement = true;
					break;
				}
			}
			FE_ASSERT(bFoundElement, "Couldn't find target element from binding path !");
			
			if (!bFoundElement)
				return;
		}
	}
	
	// static properties names
	struct ETargetProperty
	{
		enum Type
		{
			Text,
			Font,
			Image,
		};
	};
	std::map<uint32, ETargetProperty::Type> staticPropertiesMap;
	
#define DECLARE_TARGET_PROPERTY_ENTRY(_value_) staticPropertiesMap[FeStringPool::GetInstance()->CreateString( #_value_ ).Id()] = ETargetProperty::_value_;

	DECLARE_TARGET_PROPERTY_ENTRY(Text);
	DECLARE_TARGET_PROPERTY_ENTRY(Font);
	DECLARE_TARGET_PROPERTY_ENTRY(Image);

	if (staticPropertiesMap.find(targetProperty.Id()) != staticPropertiesMap.end())
	{
		auto iPropId = staticPropertiesMap[targetProperty.Id()];

		switch (iPropId)
		{
		case ETargetProperty::Font:
		case ETargetProperty::Image:
		{
			FeRenderLoadingResource resource;
			resource.Path.Set(sourceData.Cstr());
			resource.Type = iPropId == ETargetProperty::Font ? FeEResourceType::Font : FeEResourceType::Texture;
			pResourcesHandler->LoadResource(resource); // schedule resource loading

			FeRenderGeometryInstance& geomInstance = pTarget->RenderInstance->Geometry;

			if (geomInstance.Textures.GetSize() < (targetBinding.GetIndex() + 1))
				geomInstance.Textures.Resize(targetBinding.GetIndex() + 1);

			geomInstance.Textures.SetAt(targetBinding.GetIndex(), resource.Id);

		} break;
		case ETargetProperty::Text:
		{
			// todo : bind text value
		} break;
		}
	}
}
FeString FeModuleUi::FetchBindingSourceData(const FeUiBinding& binding)
{
	FeString strResult;

	switch (binding.GetType())
	{
		case FeEUiBindingType::Source_Static:
		{
			strResult = binding.GetValue();
		} break;
		case FeEUiBindingType::Source_List:
		{
			// todo : fetch value from database
		} break;
		case FeEUiBindingType::Source_Variable:
		{
			// todo : fetch value from database
		} break;
	}

	return strResult;
}
uint32 FeModuleUi::Load(const FeModuleInit* initBase)
{
	auto init = (FeModuleUiInit*)initBase;
	ReloadScripts();
	//LoadUnitTest(0);

	// register file watcher for script files
	auto pFileManagerModule = FeApplication::StaticInstance.GetModule<FeModuleFilesManager>();
	pFileManagerModule->WatchDirectory("../data/themes", OnScriptFileChanged, this);
		
	return FeEReturnCode::Success;
}
uint32 FeModuleUi::Unload()
{
	return FeEReturnCode::Success;
}
uint32 FeModuleUi::Update(const FeDt& fDt)
{
	//UpdateUnitTest(0, fDt);

	auto pRenderingModule = FeApplication::StaticInstance.GetModule<FeModuleRendering>();

	FeRenderBatch& renderBatch = pRenderingModule->CreateRenderBatch();
	renderBatch.GeometryInstances.Reserve(RenderingInstances.GetSize());

	for (auto& instance : RenderingInstances)
	{
		renderBatch.GeometryInstances.Add(instance.Geometry);
		FE_ASSERT(instance.Geometry.Textures.GetSize() < 8, "");
	}

	return FeEReturnCode::Success;
}
