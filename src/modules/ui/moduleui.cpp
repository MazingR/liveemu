#include <moduleui.hpp>

#include <rendering/modulerenderer.hpp>
#include <rendering/renderresourceshandler.hpp>

#include <common/database.hpp>
#include <common/serializable.hpp>
#include <common/maths.hpp>
#include <queue>

#define FE_HEAPID_UI 4

FeUiRootPanel::FeUiRootPanel()
{
	RenderBatch.GeometryInstances.SetHeapId(FE_HEAPID_UI);
	DefferedApplyBindingData.SetHeapId(FE_HEAPID_UI);
}
FeUiRootPanel::~FeUiRootPanel()
{
	RenderBatch.GeometryInstances.Free();
	DefferedApplyBindingData.Free();
}
FeUiElementTraversalList::FeUiElementTraversalList()
{
	Nodes.SetHeapId(FE_HEAPID_UI);
}
FeUiElementTraversalList::~FeUiElementTraversalList()
{
	Nodes.Free();
}
FeUiElementTraversalNode* FeUiElementTraversalList::AddNode()
{
	FeUiElementTraversalNode* pNode = FE_NEW(FeUiElementTraversalNode, FE_HEAPID_UI);
	Nodes.Add(pNode);
	return pNode;
}

void OnScriptFileChanged(FeEFileChangeType::Type eChangeType, const char* szPath, void* pUserData)
{
	FeSetLastError("");
	((FeModuleUi*)pUserData)->ReloadScripts();
}

void FeModuleUi::TraverseElements(FeUiRootPanel& RootPanel)
{
	std::queue<FeUiElementTraversalNode*> nodesToTraverse;
	auto pRenderingModule = FeApplication::StaticInstance.GetModule<FeModuleRendering>();

	// Create root pNode
	{
		FeUiElementTraversalNode* pNode = RootPanel.TraversalList.AddNode();
		pNode->Current = RootPanel.Panel;
		pNode->Parent = nullptr;
		nodesToTraverse.push(pNode);
	}

	while (nodesToTraverse.size())
	{
		FeUiElementTraversalNode* pNode = nodesToTraverse.front();
		FeUiElement* pElement = pNode->Current;
		
		pElement->PreTraversal(RootPanel, pNode);

		nodesToTraverse.pop();
		FeTArray<FeUiElement*> ElementChildren;
		
		for (auto& child : pElement->GetChildren())
			ElementChildren.Add(child.Get());

		for (auto& child : pElement->Transient.Children)
			ElementChildren.Add(child);

		for (auto& childElement : ElementChildren)
		{
			FeUiElementTraversalNode* pChildNode = RootPanel.TraversalList.AddNode();;

			pChildNode->Current = childElement;
			pChildNode->Parent = pElement;

			if (pChildNode->Current->GetFontEffect().IsEmpty())
				pChildNode->Current->SetFontEffect(pChildNode->Parent->GetFontEffect());

			nodesToTraverse.push(pChildNode);
			pNode->Children.Add(pChildNode);
		}
	}
}
void FeModuleUi::ComputeRenderingInstances()
{
	// Pre compute rendering instances
	for (auto& panel : RootPanels)
	{
		for (auto& pNode : panel.TraversalList.Nodes)
		{
			FeUiElement* pCurrent= pNode->Current;
			FeUiElement* pParent = pNode->Parent;

			FeRenderGeometryInstance& geomInstance = panel.RenderBatch.GeometryInstances.Add();

			geomInstance.Reset();

			// two sided ref
			pNode->GeometryInstance = &geomInstance;
			
			geomInstance.Owner = &pNode;
			geomInstance.Effect = pCurrent->GetEffect().Id();
			geomInstance.Geometry = FeGeometryHelper::GetStaticGeometry(FeEGemetryDataType::Quad);

			//pRenderingInstance->Geometry.Textures.Clear();

			FeVector3	t = pCurrent->Transient.Transform.Translation;
			FeRotation	r = pCurrent->Transient.Transform.Rotation;
			FeVector3	s = pCurrent->Transient.Transform.Scale;

			if (pParent) // compute world transformation using parent m matrix
			{
				auto& pT = pParent->Transient.Transform.Translation;
				auto& pR = pParent->Transient.Transform.Rotation;
				auto& pS = pParent->Transient.Transform.Scale;

				for (uint32 i = 0; i < 3; ++i)
				{
					t[i] += pT[i];
					r[i] += pR[i];
					s[i] *= pS[i];

					if (s[i] == 0)
						__debugbreak();
				}
			}
			pNode->TraversalTransform.Translation	= t;
			pNode->TraversalTransform.Rotation		= r;
			pNode->TraversalTransform.Scale			= s;

			FeGeometryHelper::ComputeAffineTransform(geomInstance.Transform, t, r, s);
		}
	}
}
uint32 FeModuleUi::ReloadScripts()
{
	auto pRenderingModule = FeApplication::StaticInstance.GetModule<FeModuleRendering>();
	auto pResourcesHandler = FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();

	// Clear stuff
	{
		RootPanels.Free();
		ScriptFiles.Free();

		RootPanels.Reserve(128);
		ScriptFiles.Reserve(128);

		pRenderingModule->UnloadEffects();
		pResourcesHandler->UnloadResources();
	}

	// Load scripts from files
	{
		FeTArray<FePath> files;
		files.SetHeapId(FE_HEAPID_UI);

		FeFileTools::ListFilesRecursive(files, "themes/common", ".*\\.fes");
		FeFileTools::ListFilesRecursive(files, "themes/default", ".*\\.fes"); // load default theme

		ScriptFiles.Reserve(files.GetSize());

		for (auto& file : files)
		{
			FeScriptFile& scriptFile = ScriptFiles.Add();
			auto iRes = FeJsonParser::DeserializeObject(scriptFile, file, FE_HEAPID_UI);

			if (iRes == FeEReturnCode::Success)
			{
				pRenderingModule->LoadEffects(scriptFile.GetEffects());
				for (auto Template : scriptFile.GetTemplates())
				{
					if (!Template->GetName().IsEmpty())
					{
						if (Templates.find(Template->GetName().Id()) == Templates.end())
						{
							Templates[Template->GetName().Id()] = Template.Get();
						}
					}
				}
				for (auto& Panel : scriptFile.GetPanels())
				{
					FeUiRootPanel& NewPanel = RootPanels.Add();
					
					NewPanel.RenderBatch.GeometryInstances.Reserve(1024);
					NewPanel.RenderBatch.Viewport = &pRenderingModule->GetDefaultViewport();
					NewPanel.Panel = Panel.Get();
				}
			}
			else
			{
				ScriptFiles.PopBack();
			}
		}
	}

	// Compute nodes traversal
	for (auto& panel : RootPanels)
		TraverseElements(panel);

	for (auto& panel : RootPanels)
	{
		for (auto& pNode : panel.TraversalList.Nodes)
			pNode->Current->PostTraversal(panel, pNode);
	}

	ComputeRenderingInstances();
	
	// Create font resouces
	for (auto& script : ScriptFiles)
	{
		for (auto& uiFont : script.GetFonts())
		{
			FeRenderLoadingResource resource;
			resource.Resource = FE_NEW(FeRenderFont, FE_HEAPID_RENDERER);
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
	
	for (auto& panel : RootPanels)
	{
		for (auto& pNode : panel.TraversalList.Nodes)
			pNode->Current->PreApplyBindingByType(panel, pNode);
	}

	// Apply data binding (texture, font, text,...)
	ApplyBindingByType(FeETargetPropertyType::Font);
	ApplyBindingByType(FeETargetPropertyType::Image);
	ApplyBindingByType(FeETargetPropertyType::Other);
	ApplyBindingByType(FeETargetPropertyType::Text);

	for (auto& panel : RootPanels)
	{
		for (auto& pNode : panel.TraversalList.Nodes)
			pNode->Current->PostInitialize(panel, pNode);
	}

	return FeEReturnCode::Success;
}
void FeModuleUi::ApplyBindingByType(FeETargetPropertyType::Type type)
{

	for (auto& panel : RootPanels)
	{
		for (auto& pNode : panel.TraversalList.Nodes)
		{
			for (auto& dataBind : pNode->Current->GetBindings())
			{
				auto eTargetPropertyType = GetTargetPropertyType(dataBind.GetTarget());

				if (eTargetPropertyType == type)
				{
					FeString sourceData = FetchBindingSourceData(dataBind.GetSource());

					if (!sourceData.IsEmpty())
					{
						auto iRes = ApplyBindingToTargetProperty(panel, pNode, sourceData, dataBind.GetTarget(), eTargetPropertyType);
						if (iRes != FeEReturnCode::Success)
						{
							FeUiDefferedApplyBinding& deffered = panel.DefferedApplyBindingData.Add();

							deffered.SourceData = sourceData;
							deffered.BindingTarget = &dataBind.GetTarget();
							deffered.TargetPropertyType = type;
							deffered.TraversalNode = pNode;
						}
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
uint32 FeModuleUi::ApplyBindingToTargetProperty(FeUiRootPanel& RootPanel, FeUiElementTraversalNode* pNode, const FeString& sourceData, const FeUiBinding& targetBinding, FeETargetPropertyType::Type type)
{
	auto pResourcesHandler = FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();
	auto pRenderer = FeApplication::StaticInstance.GetModule<FeModuleRendering>();

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
			resource.Resource = FE_NEW(FeRenderTexture, FE_HEAPID_RENDERER);
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
			return GenerateTextRenderingNodes(RootPanel, pNode, sourceData);
		} break;
		case FeETargetPropertyType::Other:
		{
			// todo : bind text value
		} break;
	}

	return FeEReturnCode::Success;
}
uint32 FeModuleUi::GenerateTextRenderingNodes(FeUiRootPanel& RootPanel, FeUiElementTraversalNode* pNode, const FeString& sourceData)
{
	auto pResourcesHandler = FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();
	auto pRenderer = FeApplication::StaticInstance.GetModule<FeModuleRendering>();

	const FeRenderResource* pResource = pResourcesHandler->GetResource(pNode->FontResource);
	
	if (!pResource || pResource->LoadingState != FeEResourceLoadingState::Loaded)
		return FeEReturnCode::Failed;

	auto* pFontData = (FeRenderFont*)pResource;
	const char* szText = sourceData.Cstr();
	
	uint32 iTextLen = strlen(szText);

	FeUiElement* pCurrent = pNode->Current;
	auto& resolution = pRenderer->GetDevice().GetNativeResolution();

	FeVector2 vRes(1.0f / (float)resolution.w, 1.0f / (float)resolution.h);
	FeVector2 vMapSize(1.0f / (float)pFontData->MapSize[0], 1.0f / (float)pFontData->MapSize[1]);
	float fSize = pFontData->Size*vRes[1];
	float fLineSpace = pFontData->LineSpace*vRes[1];
	float fSpace = pFontData->Space*vRes[0];
	float fInterval = pFontData->Interval*vRes[0];
	float fLineSizeX = pNode->TraversalTransform.Scale.getData()[0];
	
	FeRotation	r;
	FeVector3 tOffset;

	for (uint32 iCharIdx = 0; iCharIdx < iTextLen; ++iCharIdx)
	{
		char szChar = szText[iCharIdx];
		
		if (szChar == ' ')
		{
			uint32 iNextSpace = FeStringTools::IndexOf(szText, ' ', iCharIdx + 1, iTextLen);
			if (iNextSpace == FE_INVALID_ID)
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

		FeRenderGeometryInstance& geomInstance = RootPanel.RenderBatch.GeometryInstances.Add();
		geomInstance.Reset();

		// todo@mazingr : create text pre rendered batch
		geomInstance.Owner = &pNode;

		geomInstance.Effect = pCurrent->GetFontEffect().Id();
		geomInstance.Geometry = FeGeometryHelper::GetStaticGeometry(FeEGemetryDataType::Quad);
		//geomInstance.Textures.Add(pNode.RenderInstance->FontResource);
		geomInstance.Textures[0] = pNode->FontResource;
		geomInstance.UserData = vCharData;

		float fCharWidth = charData.Width *vRes[0];
		float fCharHeight = charData.Height * vRes[1];

		float fCharX = charData.OffsetLeft*vRes[0];
		float fCharY = fSize-charData.OffsetTop*vRes[1];
		
		FeVector3	t = pNode->TraversalTransform.Translation;
		FeVector3	s(fCharWidth, fCharHeight, 1.0f);

		t[0] += tOffset[0] + fCharX;
		t[1] += tOffset[1] + fCharY;

		tOffset[0] += fCharWidth + fInterval;

		FeGeometryHelper::ComputeAffineTransform(geomInstance.Transform, t, r, s);
	}
	return FeEReturnCode::Success;
}
int DbResultCallback(void *userData, int argc, char **argv, char **azColName)
{
	FeString* result = (FeString*)userData;

	if (argc >= 1)
	{
		*result = argv[0];
	}
	return 0;
};

FeString FeModuleUi::FetchBindingSourceData(const FeUiBinding& binding)
{
	FeString strResult;
	
	switch (binding.GetType())
	{
		case FeEUiBindingType::Static:
		{
			strResult = binding.GetValue();
		} break;
		case FeEUiBindingType::Database:
		{
			static char szSql[512];

			memset(szSql, 0, 512);
			sprintf_s(szSql, binding.GetValue().Cstr());
			FeDatabase::StaticInstance.Execute(szSql, DbResultCallback, &strResult);

			// todo : fetch value from database
		} break;
		case FeEUiBindingType::Variable:
		{
			// todo : fetch value from database
		} break;
	}

	return strResult;
}
uint32 FeModuleUi::Load(const FeModuleInit* initBase)
{
	auto init = (FeModuleUiInit*)initBase;

	RootPanels.Reserve(128);

	ReloadScripts(); // <------ all the stuff is done here
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

	for (auto& panel : RootPanels)
	{
		for (auto& apply : panel.DefferedApplyBindingData)
		{
			auto iRes = ApplyBindingToTargetProperty(panel, apply.TraversalNode, apply.SourceData, *apply.BindingTarget, apply.TargetPropertyType);

			if (iRes != FeEReturnCode::Success)
				failedApplies.Add(apply);
		}

		panel.DefferedApplyBindingData.Clear();
		panel.DefferedApplyBindingData.Add(failedApplies);

		// Compute inherited states
		for (auto& pNode : panel.TraversalList.Nodes)
		{
			for (auto child : pNode->Children)
			{
				auto pCurrent = child->Current;

				#define PROPAGATE_STATE(a) pCurrent->SetState(pCurrent->HasState(a) && pNode->Current->HasState(a), a);

				PROPAGATE_STATE(FeEUiElementState::Visible);
				PROPAGATE_STATE(FeEUiElementState::Enabled);
				PROPAGATE_STATE(FeEUiElementState::Collapsed);
				PROPAGATE_STATE(FeEUiElementState::Focused);
				PROPAGATE_STATE(FeEUiElementState::Selected);
			}
		}


		// Cull empty nodes (conainers)
		//for (auto& pNode : panel.TraversalList.Nodes)
		//{
		//	pNode->IsCulled = true;

		//	for (uint32 i = 0; i < FE_RENDERER_TEXTURES_CHANNELS_COUNT; ++i)
		//	{
		//		if (pNode->GeometryInstance->Textures[i] != 0)
		//		{
		//			pNode->IsCulled = false;
		//			break;
		//		}
		//	}
		//}

		bool bHasNonCulledNode = false;

		// Register all geometry to renderer
		for (FeRenderGeometryInstance& geom : panel.RenderBatch.GeometryInstances)
		{
			geom.IsCulled = true;

			for (uint32 i = 0; i < FE_RENDERER_TEXTURES_CHANNELS_COUNT; ++i)
			{
				if (geom.Textures[i] != 0)
				{
					geom.IsCulled = false;
					bHasNonCulledNode = true;
					break;
				}
			}

			//FeUiElementTraversalNode* pNode = (FeUiElementTraversalNode*)geom.Owner;
			//geom.IsCulled = pNode->IsCulled;
			//
			//if (!geom.IsCulled)
			//	bHasNonCulledNode = false;
		}
		if (bHasNonCulledNode)
		{
			pRenderingModule->RegisterRenderBatch(&panel.RenderBatch);
		}
	}


	return FeEReturnCode::Success;
}
