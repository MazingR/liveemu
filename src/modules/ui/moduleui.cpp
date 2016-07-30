#include <moduleui.hpp>
#include <rendering/modulerenderer.hpp>
#include <rendering/renderresourceshandler.hpp>

#include <common/serializable.hpp>
#include <common/maths.hpp>
#include <queue>

#define UI_HEAP 2

void OnScriptFileChanged(FeEFileChangeType::Type eChangeType, const char* szPath, void* pUserData)
{
	FeSetLastError("");
	((FeModuleUi*)pUserData)->ReloadScripts();
}

void FeModuleUi::TraverseElements(FeScriptFile& script, FeUiElementTraversalList& traversal)
{
	std::queue<FeUiElementTraversalNode*> uiElements;

	// Count entries first

	for (auto& panel : script.GetPanels())
	{
		FeUiElementTraversalNode& node = traversal.Nodes.Add();
		node.Current = &panel;
		node.Parent = nullptr;

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

			if (childNode.Current->GetFontEffect().IsEmpty())
				childNode.Current->SetFontEffect(childNode.Parent->GetFontEffect());

			uiElements.push(&childNode);
			pNode->Children.Add(&childNode);
		}
	}
}
void FeModuleUi::ComputeRenderingInstances()
{
	// Pre compute rendering instances
	for (auto& node : TraversalList.Nodes)
	{
		FeUiElement* pCurrent = node.Current;
		FeUiElement* pParent = node.Parent;

		FeRenderGeometryInstance& geomInstance = RenderBatches[0].GeometryInstances.Add();
		geomInstance.Reset();
		
		// two sided ref
		node.GeometryInstance = &geomInstance;
		geomInstance.Owner = &node;

		geomInstance.Effect = pCurrent->GetEffect().Id();
		geomInstance.Geometry = FeGeometryHelper::GetStaticGeometry(FeEGemetryDataType::Quad);

		//pRenderingInstance->Geometry.Textures.Clear();

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

				if (s[i] == 0)
					__debugbreak();
			}
		}
		FeGeometryHelper::ComputeAffineTransform(geomInstance.Transform, t, r, s);
	}

}
uint32 FeModuleUi::ReloadScripts()
{
	auto pRenderingModule = FeApplication::StaticInstance.GetModule<FeModuleRendering>();
	auto pResourcesHandler = FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();

	// Clear stuff
	for (auto& batch : RenderBatches)
	{
		batch.GeometryInstances.Clear();
	}

	pRenderingModule->UnloadEffects(); 
	ScriptFiles.Clear();

	RenderBatches.Clear();
	RenderBatches.Reserve(64);
	RenderBatches.Add(); // Defeault batch

	RenderBatches[0].GeometryInstances.Reserve(1024);

	RenderBatches[0].Viewport = &pRenderingModule->GetDefaultViewport();

	Panels.Clear();
	TraversalList.Nodes.Clear();

	pResourcesHandler->UnloadResources();

	// Load scripts from files
	FeTArray<FePath> files;
	files.SetHeapId(UI_HEAP);

	FeFileTools::ListFilesRecursive(files, "themes/common", ".*\\.fes");
	FeFileTools::ListFilesRecursive(files, "themes/default", ".*\\.fes"); // load default theme

	ScriptFiles.Reserve(files.GetSize());

	for (auto& file : files)
	{
		FeScriptFile& scriptFile = ScriptFiles.Add();
		auto iRes = FeJsonParser::DeserializeObject(scriptFile, file, UI_HEAP);

		if (iRes == FeEReturnCode::Success)
		{
			pRenderingModule->LoadEffects(scriptFile.GetEffects());

			for (auto& panel : scriptFile.GetPanels())		Panels.Add(&panel);
			for (auto& font : scriptFile.GetFonts())		Fonts.Add(&font);
			for (auto& effect : scriptFile.GetEffects())	Effects.Add(&effect);
		}
		else
		{
			ScriptFiles.PopBack();
		}
	}

	for (auto& script : ScriptFiles)
	{
		TraverseElements(script, TraversalList); // add all ui elments to flat list
	}

	ComputeRenderingInstances();
	
	for (auto& script : ScriptFiles)
	{
		// Create font resouces
		for (auto& uiFont : script.GetFonts())
		{
			FeRenderLoadingResource resource;
			resource.Resource = FE_NEW(FeRenderFont, RENDERER_HEAP);
			resource.Path = uiFont.GetTrueTypeFile();
			resource.Type = FeEResourceType::Font;
			resource.Id = FeStringTools::GenerateUIntIdFromString(uiFont.GetName().Cstr());

			auto* pFontData = (FeRenderFont*)resource.Resource;

			pFontData->Size = uiFont.GetSize();
			pFontData->Interval = uiFont.GetInterval();
			pFontData->Space = uiFont.GetSpace();
			pFontData->LineSpace = uiFont.GetLineSpace();
			pFontData->TrueTypeFile = uiFont.GetTrueTypeFile();

			pResourcesHandler->LoadResource(resource);
		}
	}
	// Apply data binding (texture, font, text,...)
	ApplyBindingByType(FeETargetPropertyType::Font);
	ApplyBindingByType(FeETargetPropertyType::Image);
	ApplyBindingByType(FeETargetPropertyType::Other);
	ApplyBindingByType(FeETargetPropertyType::Text);

	for (auto& node : TraversalList.Nodes)
	{
		if (node.FontResource)
		{
			if (node.GeometryInstance->Textures[0] == 0 &&
				node.GeometryInstance->Textures[1] == 0)
				node.IsCulled = true;
		}
	}

	return FeEReturnCode::Success;
}
void FeModuleUi::ApplyBindingByType(FeETargetPropertyType::Type type)
{
	for (auto& node : TraversalList.Nodes)
	{
		for (auto& dataBind : node.Current->GetBindings())
		{
			auto eTargetPropertyType = GetTargetPropertyType(dataBind.GetTarget());

			if (eTargetPropertyType == type)
			{
				FeString sourceData = FetchBindingSourceData(dataBind.GetSource());

				if (!sourceData.IsEmpty())
				{
					auto iRes = ApplyBindingToTargetProperty(node, sourceData, dataBind.GetTarget(), eTargetPropertyType);
					if (iRes != FeEReturnCode::Success)
					{
						FeUiDefferedApplyBinding& deffered = DefferedApplyBindingData.Add();

						deffered.SourceData = sourceData;
						deffered.BindingTarget = &dataBind.GetTarget();
						deffered.TargetPropertyType = type;
						deffered.TraversalNode = &node;
					}
				}
			}
		}
	}
}
FeETargetPropertyType::Type FeModuleUi::GetTargetPropertyType(const FeUiBinding& targetBinding)
{
	static std::map<uint32, FeETargetPropertyType::Type> staticPropertiesMap;
	static bool bIsStaticPropsMapInit = false;

	if (!bIsStaticPropsMapInit)
	{
		bIsStaticPropsMapInit = true;
#define DECLARE_TARGET_PROPERTY_ENTRY(_value_) staticPropertiesMap[FeStringPool::GetInstance()->CreateString( #_value_ ).Id()] = FeETargetPropertyType::_value_;

		DECLARE_TARGET_PROPERTY_ENTRY(Text);
		DECLARE_TARGET_PROPERTY_ENTRY(Font);
		DECLARE_TARGET_PROPERTY_ENTRY(Image);
	}
	if (staticPropertiesMap.find(targetBinding.GetProperty().Id()) != staticPropertiesMap.end())
	{
		return staticPropertiesMap[targetBinding.GetProperty().Id()];
	}
	return FeETargetPropertyType::Other;
}
uint32 FeModuleUi::ApplyBindingToTargetProperty(FeUiElementTraversalNode& node, const FeString& sourceData, const FeUiBinding& targetBinding, FeETargetPropertyType::Type type)
{
	auto pResourcesHandler = FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();
	auto pRenderer = FeApplication::StaticInstance.GetModule<FeModuleRendering>();

	FeUiElementTraversalNode* pNode = &node;
	const FeTArray<FeString>& targetPath = targetBinding.GetPath();
	const FeString& targetProperty = targetBinding.GetProperty();

	// find target element from path
	if (targetPath.GetSize() > 0)
	{
		uint32 iDepth = 0;

		while (iDepth < targetPath.GetSize())
		{
			bool bFoundElement = false;

			for (auto& child : pNode->Children)
			{
				if (child->Current->GetName() == targetPath[iDepth])
				{
					iDepth++;
					pNode = child;
					bFoundElement = true;
					break;
				}
			}
			if (!bFoundElement)
			{
				FE_LOG("Couldn't find target element from binding path !");
			}
			if (!bFoundElement)
				return FeEReturnCode::Failed;
		}
	}
	
	switch (type)
	{
		case FeETargetPropertyType::Image:
		{
			FeRenderLoadingResource resource;
			resource.Path.Set(sourceData.Cstr());
			resource.Resource = FE_NEW(FeRenderTexture, RENDERER_HEAP);
			resource.Type = FeEResourceType::Texture;
			pResourcesHandler->LoadResource(resource); // schedule resource loading
			
			//if (geomInstance.Textures.GetSize() < (targetBinding.GetIndex() + 1))
			//	geomInstance.Textures.Resize(targetBinding.GetIndex() + 1);

			//geomInstance.Textures.SetAt(targetBinding.GetIndex(), resource.Id);

			pNode->GeometryInstance->Textures[targetBinding.GetIndex()] = resource.Id;

		} break;
		case FeETargetPropertyType::Font:
		{
			auto resourceId = FeStringTools::GenerateUIntIdFromString(sourceData.Cstr());
			pNode->FontResource = resourceId;
		} break;
		case FeETargetPropertyType::Text:
		{
			return GenerateTextRenderingNodes(*pNode, sourceData);
		} break;
		case FeETargetPropertyType::Other:
		{
			// todo : bind text value
		} break;
	}

	return FeEReturnCode::Success;
}
uint32 FeModuleUi::GenerateTextRenderingNodes(FeUiElementTraversalNode& node, const FeString& sourceData)
{
	auto pResourcesHandler = FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();
	auto pRenderer = FeApplication::StaticInstance.GetModule<FeModuleRendering>();

	const FeRenderResource* pResource = pResourcesHandler->GetResource(node.FontResource);
	
	if (!pResource || pResource->LoadingState != FeEResourceLoadingState::Loaded)
		return FeEReturnCode::Failed;

	auto* pFontData = (FeRenderFont*)pResource;
	const char* szText = sourceData.Cstr();
	
	uint32 iTextLen = strlen(szText);

	FeUiElement* pCurrent = node.Current;
	auto& resolution = pRenderer->GetDevice().GetNativeResolution();

	FeVector2 vRes(1.0f / (float)resolution.w, 1.0f / (float)resolution.h);
	FeVector2 vMapSize(1.0f / (float)pFontData->MapSize[0], 1.0f / (float)pFontData->MapSize[1]);
	float fSize = pFontData->Size*vRes[1];
	float fLineSpace = pFontData->LineSpace*vRes[1];
	float fSpace = pFontData->Space*vRes[0];
	float fInterval = pFontData->Interval*vRes[0];
	float fLineSizeX = pCurrent->GetTransform().Scale.mData[0];
	
	FeRotation	r;
	FeVector3 tOffset;

	for (uint32 iCharIdx = 0; iCharIdx < iTextLen; ++iCharIdx)
	{
		char szChar = szText[iCharIdx];
		
		if (szChar == ' ')
		{
			uint32 iNextSpace = FeStringTools::IndexOf(szText, ' ', iCharIdx + 1, iTextLen);
			if (iNextSpace == (uint32)-1)
				iNextSpace = iTextLen;

			if ((iNextSpace - iCharIdx)*fSpace + tOffset[0] > fLineSizeX)
			{
				tOffset[0] = 0;
				tOffset[1] += fSize + fLineSpace;
			}
			else
			{
				tOffset[0] += fSpace;
			}

			continue;
		}

		if (szChar == '\n')
		{
			tOffset[0] = 0;
			tOffset[1] += fSize + fLineSpace;
			continue;
		}

		if (pFontData->Chars.find(szChar) == pFontData->Chars.end())
			continue;

		const FeRenderFontChar& charData = pFontData->Chars[szChar];
		FeVector4 vCharData(
			charData.Left*vMapSize[0],
			charData.Top*vMapSize[1],
			charData.Width*vMapSize[0],
			charData.Height*vMapSize[1]);

		FeRenderGeometryInstance& geomInstance = RenderBatches[0].GeometryInstances.Add();
		geomInstance.Reset();

		// todo@mazingr : create text pre rendered batch
		geomInstance.Owner = &node;

		geomInstance.Effect = pCurrent->GetFontEffect().Id();
		geomInstance.Geometry = FeGeometryHelper::GetStaticGeometry(FeEGemetryDataType::Quad);
		//geomInstance.Textures.Add(node.RenderInstance->FontResource);
		geomInstance.Textures[0] = node.FontResource;
		geomInstance.UserData = vCharData;

		float fCharWidth = charData.Width *vRes[0];
		float fCharHeight = charData.Height * vRes[1];

		float fCharX = charData.OffsetLeft*vRes[0];
		float fCharY = fSize-charData.OffsetTop*vRes[1];
		
		FeVector3	t = pCurrent->GetTransform().Translation;
		FeVector3	s(fCharWidth, fCharHeight, 1.0f);

		t[0] += tOffset[0] + fCharX;
		t[1] += tOffset[1] + fCharY;

		tOffset[0] += fCharWidth + fInterval;

		FeGeometryHelper::ComputeAffineTransform(geomInstance.Transform, t, r, s);
	}
	return FeEReturnCode::Success;
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

	FeTArray<FeUiDefferedApplyBinding> failedApplies;

	for (auto& apply : DefferedApplyBindingData)
	{
		auto iRes = ApplyBindingToTargetProperty(*apply.TraversalNode, apply.SourceData, *apply.BindingTarget, apply.TargetPropertyType);

		if (iRes != FeEReturnCode::Success)
			failedApplies.Add(apply);
	}

	DefferedApplyBindingData.Clear();
	DefferedApplyBindingData.Add(failedApplies);

	// Register all geometry to renderer
	for (auto& batch : RenderBatches)
	{
		pRenderingModule->RegisterRenderBatch(&batch);
	}

	return FeEReturnCode::Success;
}
