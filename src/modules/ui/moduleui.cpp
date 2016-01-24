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

uint32 FeModuleUi::LoadUnitTest(uint32 iTest)
{
	FeTArray<FePath> files;
	files.SetHeapId(RENDERER_HEAP);

	FeFileTools::ListFilesRecursive(files, "../data/test/textures/boxfronts", "*.jpg");
	FeFileTools::ListFilesRecursive(files, "../data/test/textures/bb2VFX", "*");
	uint32 iTexturesCount = files.GetSize();
	//iTexturesCount = 5;

	// Creat textures
	auto pResourcesHandler = FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();
	
	const int InstancesCount = 1;
	FeString szEffect = FeStringPool::GetInstance()->CreateString("Default");

	for (uint32 i = 0; i < InstancesCount; ++i)
	{
		auto pPanel = FE_NEW(FeUiPanel, 1);
		Panels.Add(pPanel);

		pPanel->SetEffect(szEffect);

		if (iTexturesCount>0)
		{
			FeRenderLoadingResource resLoading;
			resLoading.Type = FeEResourceType::Texture;
			uint32 iTexIdx = i % iTexturesCount;
			resLoading.Path = files[iTexIdx];
			
			pResourcesHandler->LoadResource(resLoading); 
		}
	}

	return FeEReturnCode::Success;
}
uint32 FeModuleUi::UpdateUnitTest(uint32 iTest, const FeDt& fDt)
{
	/*
	srand(1564);

	static float fRotX = 0, fRotY = 0, fRotZ = 0;
	static FeVector3 translation(0, 0, 0), scale(1, 1, 1);
	static float fIncrement = 0.0f;

	int32 iColomns = 60;
	fIncrement += 0.15f*fDt.TotalSeconds;

	float fOffset = (1.01f - abs(sin(fIncrement)))*3.0f;

	float fOffsetBetweenX = 1.f*fOffset;
	float fOffsetBetweenY = 1.0f*fOffset;

	translation.mData[0] = (float)-iColomns;
	translation.mData[1] = 0;
	translation.mData[2] = 50.0f;// -fOffset;

	scale.mData[0] = 1.0f + fOffset;
	scale.mData[1] = 1.0f + fOffset;

	fRotZ = fOffset;

	auto pRenderingModule = FeApplication::StaticInstance.GetModule<FeModuleRendering>();

	FeRenderBatch& renderBatch = pRenderingModule->CreateRenderBatch();
	renderBatch.GeometryInstances.Reserve(GeometryInstances.GetSize());

	for (uint32 iInstanceIdx = 0; iInstanceIdx < GeometryInstances.GetSize(); ++iInstanceIdx)
	{
		FeRenderGeometryInstance& geomInstance = GeometryInstances[iInstanceIdx];

		translation.mData[0] = -40 + (iInstanceIdx % iColomns) * fOffsetBetweenX;
		translation.mData[1] = -20 + (iInstanceIdx / iColomns) * fOffsetBetweenY;

		FeGeometryHelper::ComputeAffineTransform(geomInstance.Transform, translation, FeRotation(fRotX, fRotY, fRotZ), scale);

		renderBatch.GeometryInstances.Add(geomInstance);
	}
	*/
	return FeEReturnCode::Success;
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

	// Pre compute rendering instances
	struct UiElementNode
	{
		FeUiElement* Element;
		FeUiElement* Parent;
	};
	std::queue<UiElementNode> uiElements;

	for (auto& panel : Panels)
	{
		UiElementNode element;
		element.Element = panel;
		element.Parent = NULL;
		uiElements.push(element);
	}

	while (uiElements.size())
	{
		UiElementNode element = uiElements.front();
		FeUiElement* pElement = element.Element;

		uiElements.pop();

		FeUiRenderingInstance& renderingInstance = RenderingInstances.Add();

		renderingInstance.Owner = pElement;
		renderingInstance.Geometry.Effect = pElement->GetEffect().Id();
		renderingInstance.Geometry.Geometry = FeGeometryHelper::GetStaticGeometry(FeEGemetryDataType::Quad);
		renderingInstance.Geometry.Textures.Clear();

		FeVector3	t = pElement->GetTransform().Translation;
		FeRotation	r = pElement->GetTransform().Rotation;
		FeVector3	s = pElement->GetTransform().Scale;

		if (element.Parent)
		{
			auto& pT = element.Parent->GetTransform().Translation;
			auto& pR = element.Parent->GetTransform().Rotation;
			auto& pS = element.Parent->GetTransform().Scale;

			for (uint32 i = 0; i < 3; ++i)
			{
				t[i] += pT[i];
				r[i] += pR[i];
				s[i] *= pS[i];
			}
		}
		FeGeometryHelper::ComputeAffineTransform(renderingInstance.Geometry.Transform, t, r, s);

		for (auto& dataBind : pElement->GetBindings())
		{
			auto& source	= dataBind.GetSource();
			auto& target	= dataBind.GetTarget();

			uint32 iTargetPropertyId = target.GetProperty().Id();

			FeResourceId resourceId;

			switch (source.GetType())
			{
				case FeEUiBindingType::StaticImage:
				{
					FeRenderLoadingResource resLoading;
					resLoading.Type = FeEResourceType::Texture;
					resLoading.Path.Set(source.GetPath().Back().Cstr());
					pResourcesHandler->LoadResource(resLoading);
					resourceId = resLoading.Id;
				} break;
				default:
					break;
			}

			static FeString szTargetTexture = FeStringPool::GetInstance()->CreateString("Texture");

			if (iTargetPropertyId == szTargetTexture.Id())
			{
				FE_ASSERT(renderingInstance.Geometry.Textures.GetSize() < 1, "");
				renderingInstance.Geometry.Textures.Add(resourceId);
			}
		}

		for (auto& child : pElement->GetChildren())
		{
			UiElementNode childElement;
			childElement.Element = child.Ptr;
			childElement.Parent = pElement;

			uiElements.push(childElement);
		}
	}
	for (auto& instance : RenderingInstances)
	{
		FE_ASSERT(instance.Geometry.Textures.GetSize() < 8, "");
	}

	return FeEReturnCode::Success;
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

//extern "C"
//{
//	int z_verbose = 0;
//
//	void z_error(/* should be const */char* message)
//	{
//		FE_LOG(message);
//	}
//}
//
//#include <ft2build.h>
//#include <freetype/freetype.h>
//
//struct  FeFontData
//{
//
//};
//void FeModuleFontsHandler::FontLoadingCallback(FeRenderTexture* pTexture, void* _pUserData)
//{
//	FontLoadingData* pUserData = (FontLoadingData*)_pUserData;
//	FePath& fontPath = pUserData->FontPath;
//	FeModuleFontsHandler* pThis = pUserData->This;
//
//	FT_Face& face = pThis->FtFontFaces.Add();
//	bool bLoadingFailed = false;
//
//	//"../data/themes/common/fonts/Super Retro Italic M54.ttf"
//	auto error = FT_New_Face(pThis->FtLibrary, fontPath.Value, 0, &face);
//
//	if (error == FT_Err_Unknown_File_Format)
//	{
//		FE_LOG("the font file could be opened and read, but it appears that its font format is unsupported");
//		bLoadingFailed = true;
//	}
//	else if (error)
//	{
//		FE_LOG("that the font file could not be opened or read, or that it is broken..");
//		bLoadingFailed = true;
//	}
//
//	if (bLoadingFailed)
//	{
//		pThis->FtFontFaces.PopBack();
//	}
//	else // font loading succeeded
//	{
//		error = FT_Set_Char_Size(
//			face,	/* handle to face object           */
//			0,		/* char_width in 1/64th of points  */
//			16 * 64,/* char_height in 1/64th of points */
//			300,	/* horizontal device resolution    */
//			300);	/* vertical device resolution      */
//
//		error = FT_Set_Pixel_Sizes(
//			face,	/* handle to face object */
//			0,		/* pixel_width           */
//			16);	/* pixel_height          */
//
//
//		FT_GlyphSlot  slot = face->glyph;  /* a small shortcut */
//		//int           pen_x, pen_y, n;
//
//		//pen_x = 300;
//		//pen_y = 200;
//
//		char szTestTxt[] = "Hello World !";
//
//		size_t iTextSize = strlen(szTestTxt);
//
//		for (size_t n = 0; n < iTextSize; ++n)
//		{
//			FT_UInt  glyph_index;
//
//			/* retrieve glyph index from character code */
//			glyph_index = FT_Get_Char_Index(face, szTestTxt[n]);
//
//			/* load glyph image into the slot (erase previous one) */
//			error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
//			if (error)
//				continue;  /* ignore errors */
//
//			/* convert to an anti-aliased bitmap */
//			error = FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);
//			if (error)
//				continue;
//
//			/* now, draw to our target surface */
//			//my_draw_bitmap(&slot->bitmap,
//			//	pen_x + slot->bitmap_left,
//			//	pen_y - slot->bitmap_top);
//
//			/* increment pen position */
//			//pen_x += slot->advance.x >> 6;
//			//pen_y += slot->advance.y >> 6; /* not useful for now */
//		}
//	}
//
//	FE_DELETE(FontLoadingData, pUserData, 1);
//}
//uint32 FeModuleFontsHandler::LoadFontFromFile(const FePath& path)
//{
//	auto pResourcesHandler = FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();
//	
//	FeResourceId texId;
//	FontLoadingData* pCallbackData = FE_NEW(FontLoadingData, 1);
//
//	pCallbackData->FontPath = path;
//	pCallbackData->This = this;
//
//	pResourcesHandler->LoadTextureFromMemory("FontTest", &texId, FeModuleFontsHandler::FontLoadingCallback, pCallbackData);
//
//
//	return FeEReturnCode::Success;
//}
//
//uint32 FeModuleFontsHandler::Load(const FeModuleInit*)
//{
//	auto error = FT_Init_FreeType(&FtLibrary);
//	
//	if (error)
//	{
//		return FeEReturnCode::Failed;
//	}
//
//	return FeEReturnCode::Success;
//}
//uint32 FeModuleFontsHandler::Unload()
//{
//	return FeEReturnCode::Success;
//}
//uint32 FeModuleFontsHandler::Update(const FeDt& fDt)
//{
//	return FeEReturnCode::Success;
//}