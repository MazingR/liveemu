#include <moduleui.hpp>
#include <rendering/modulerenderer.hpp>
#include <rendering/modulerenderresourceshandler.hpp>

using namespace FeRendering;
const char imagesPath[][COMMON_PATH_SIZE] =
{
	//"../data/image.jpg",
	//"../data/add86.png",
	//"../data/cancel15.png",
	//"../data/download76.png",
	//"../data/gamecube (1).png",
	//"../data/gamemode-tv-1P.png",
	//"../data/gamemode-tv-coop.png",
	//"../data/gamemode-tv-online.png",
	//"../data/gamemode-tv-vs.png",
	//"../data/gamepad-arcade.png",
	//"../data/gamepad-dc.png",
	//"../data/gamepad-gc.png",
	//"../data/gamepad-genesis.png",
	//"../data/gamepad-joystick.png",
	//"../data/gamepad-n64.png",
	//"../data/gamepad-nes.png",
	//"../data/gamepad-ps2.png",
	//"../data/gamepad-ps4.png",
	//"../data/gamepad-snes.png",
	//"../data/gamepad-wiictrler.png",
	//"../data/gamepad-wiimote.png",
	//"../data/gamepad-wiiugamepad.png",
	//"../data/gamepad-x360.png",
	//"../data/gamepad-xb.png",
	//"../data/hardware-n64.png",
	//"../data/hardware-ps1.png",
	//"../data/nintendo2.png",
	//"../data/players-1P.png",
	//"../data/players2-1P.png",
	//"../data/players2-2P.png",
	//"../data/players2-3P.png",
	//"../data/players2-4P.png",
	//"../data/players-2P.png",
	//"../data/players-3P.png",
	//"../data/players-4P.png",
	//"../data/plus42.png",
	//"../data/ps31.png",
	//"../data/ps43.png",
	//"../data/rest2.png",
	//"../data/shooting6.png",
	//"../data/sony.png",
	//"../data/xbox10.png",
	//"../data/xbox14.png",
	//"../data/xbox20.png",
	//"../data/xbox8.png",

	"../data/test/textures/big/game_1000724910_0.jpg",
	"../data/test/textures/big/game_-1035590964_0.jpg",
	"../data/test/textures/big/game_1047251059_0.jpg",
	"../data/test/textures/big/game_-1081683598_0.jpg",
	"../data/test/textures/big/game_-1084408546_0.jpg",
	"../data/test/textures/big/game_-1089766832_0.jpg",
	"../data/test/textures/big/game_1098683321_0.jpg",
	"../data/test/textures/big/game_-1108183998_0.jpg",
	"../data/test/textures/big/game_-1108899335_0.jpg",
	"../data/test/textures/big/game_1141820803_0.jpg",
	"../data/test/textures/big/game_1153944611_0.jpg",
	"../data/test/textures/big/game_-117745912_0.jpg",
	"../data/test/textures/big/game_1178375866_0.jpg",
	"../data/test/textures/big/game_1178889506_0.jpg",
	"../data/test/textures/big/game_1179596059_0.jpg",
	"../data/test/textures/big/game_1188500942_0.jpg",
	"../data/test/textures/big/game_-1212560536_0.jpg",
	"../data/test/textures/big/game_-1217884311_0.jpg",
	"../data/test/textures/big/game_-1236151625_0.jpg",
	"../data/test/textures/big/game_1240678307_0.jpg",
	"../data/test/textures/big/game_-1259007621_0.jpg",
	"../data/test/textures/big/game_1266410210_0.jpg",
	"../data/test/textures/big/game_1269584910_0.jpg",
	"../data/test/textures/big/game_-1273029697_0.jpg",
	"../data/test/textures/big/game_130617774_0.jpg",
	"../data/test/textures/big/game_1317914904_0.jpg",
	"../data/test/textures/big/game_1343656939_0.jpg",
	"../data/test/textures/big/game_135736703_0.jpg",
	"../data/test/textures/big/game_-1441380732_0.jpg",
	"../data/test/textures/big/game_1456143959_0.jpg",
	"../data/test/textures/big/game_1474900663_0.jpg",
	"../data/test/textures/big/game_-1499725274_0.jpg",
	"../data/test/textures/big/game_1500802570_0.jpg",
	"../data/test/textures/big/game_-154047915_0.jpg",
	"../data/test/textures/big/game_-1548854930_0.jpg",
	"../data/test/textures/big/game_-1561767766_0.jpg",
	"../data/test/textures/big/game_-1566476248_0.jpg",
	"../data/test/textures/big/game_-1570470612_0.jpg",
	"../data/test/textures/big/game_158858897_0.jpg",
	"../data/test/textures/big/game_1592134245_0.jpg",
	"../data/test/textures/big/game_-1617139361_0.jpg",
	"../data/test/textures/big/game_1652416736_0.jpg",
	"../data/test/textures/big/game_-1673109957_0.jpg",
	"../data/test/textures/big/game_-1684880791_0.jpg",
	"../data/test/textures/big/game_-1689628266_0.jpg",
	"../data/test/textures/big/game_1700662483_0.jpg",
	"../data/test/textures/big/game_-1716457786_0.jpg",
	"../data/test/textures/big/game_1781446651_0.jpg",
	"../data/test/textures/big/game_1784740620_0.jpg",
	"../data/test/textures/big/game_1785658782_0.jpg",
	"../data/test/textures/big/game_1788330505_0.jpg",
	"../data/test/textures/big/game_-1799234640_0.jpg",
	"../data/test/textures/big/game_-1801449648_0.jpg",
	"../data/test/textures/big/game_-1805168287_0.jpg",
	"../data/test/textures/big/game_-1855107887_0.jpg",
	"../data/test/textures/big/game_1865217772_0.jpg",
	"../data/test/textures/big/game_1879833277_0.jpg",
	"../data/test/textures/big/game_-1887297557_0.jpg",
	"../data/test/textures/big/game_1908371898_0.jpg",
	"../data/test/textures/big/game_-1919785291_0.jpg",
	"../data/test/textures/big/game_-1920146435_0.jpg",
	"../data/test/textures/big/game_-1921941745_0.jpg",
	"../data/test/textures/big/game_-1936790254_0.jpg",
	"../data/test/textures/big/game_1948658079_0.jpg",
	"../data/test/textures/big/game_1949958691_0.jpg",
	"../data/test/textures/big/game_196833236_0.jpg",
	"../data/test/textures/big/game_1970474625_0.jpg",
	"../data/test/textures/big/game_-1990199580_0.jpg",
	"../data/test/textures/big/game_-1990617449_0.jpg",
	"../data/test/textures/big/game_-1994350211_0.jpg",
	"../data/test/textures/big/game_-2006979347_0.jpg",
	"../data/test/textures/big/game_-2018438620_0.jpg",
	"../data/test/textures/big/game_2028427492_0.jpg",
	"../data/test/textures/big/game_-2101938663_0.jpg",
	"../data/test/textures/big/game_-211913842_0.jpg",
	"../data/test/textures/big/game_-2127464580_0.jpg",
	"../data/test/textures/big/game_-2132336662_0.jpg",
	"../data/test/textures/big/game_2135214789_0.jpg",
	"../data/test/textures/big/game_-244055568_0.jpg",
	"../data/test/textures/big/game_245493983_0.jpg",
	"../data/test/textures/big/game_278869040_0.jpg",
	"../data/test/textures/big/game_-306222814_0.jpg",
	"../data/test/textures/big/game_-312678299_0.jpg",
	"../data/test/textures/big/game_-361563214_0.jpg",
	"../data/test/textures/big/game_-367514916_0.jpg",
	"../data/test/textures/big/game_-386690724_0.jpg",
	"../data/test/textures/big/game_391066832_0.jpg",
	"../data/test/textures/big/game_436039139_0.jpg",
	"../data/test/textures/big/game_-439197621_0.jpg",
	"../data/test/textures/big/game_47201178_0.jpg",
	"../data/test/textures/big/game_482448943_0.jpg",
	"../data/test/textures/big/game_496957369_0.jpg",
	"../data/test/textures/big/game_502202634_0.jpg",
	"../data/test/textures/big/game_-525363347_0.jpg",
	"../data/test/textures/big/game_-540410889_0.jpg",
	"../data/test/textures/big/game_-564972365_0.jpg",
	"../data/test/textures/big/game_570964198_0.jpg",
	"../data/test/textures/big/game_582351676_0.jpg",
	"../data/test/textures/big/game_-609547888_0.jpg",
	"../data/test/textures/big/game_-610573071_0.jpg",
	"../data/test/textures/big/game_654787779_0.jpg",
	"../data/test/textures/big/game_-669516346_0.jpg",
	"../data/test/textures/big/game_-692701470_0.jpg",
	"../data/test/textures/big/game_-69427610_0.jpg",
	"../data/test/textures/big/game_710303245_0.jpg",
	"../data/test/textures/big/game_-714332284_0.jpg",
	"../data/test/textures/big/game_-727100657_0.jpg",
	"../data/test/textures/big/game_742205197_0.jpg",
	"../data/test/textures/big/game_759059850_0.jpg",
	"../data/test/textures/big/game_793438072_0.jpg",
	"../data/test/textures/big/game_796811181_0.jpg",
	"../data/test/textures/big/game_800606818_0.jpg",
	"../data/test/textures/big/game_812140266_0.jpg",
	"../data/test/textures/big/game_-831307617_0.jpg",
	"../data/test/textures/big/game_832808098_0.jpg",
	"../data/test/textures/big/game_833585206_0.jpg",
	"../data/test/textures/big/game_847790880_0.jpg",
	"../data/test/textures/big/game_851743491_0.jpg",
	"../data/test/textures/big/game_881989136_0.jpg",
	"../data/test/textures/big/game_-885920656_0.jpg",
	"../data/test/textures/big/game_890537995_0.jpg",
	"../data/test/textures/big/game_909767654_0.jpg",
	"../data/test/textures/big/game_-960835690_0.jpg"
	
};
const int iTexturesCount = (sizeof(imagesPath) / COMMON_PATH_SIZE);


namespace FeUi
{

	uint32 FeModuleUi::Load(const ::FeCommon::FeModuleInit* initBase)
	{
		auto init = (FeModuleUiInit*)initBase;
		srand(1564);

		// Creat textures
		auto pResourcesHandler = FeCommon::FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();
		GeometryInstances.SetHeapId(RENDERER_HEAP);
		const int InstancesCount = 1024*16;

		GeometryInstances.Reserve(InstancesCount);
		FE_ASSERT(iTexturesCount < (sizeof(imagesPath) / COMMON_PATH_SIZE), "");

		for (uint32 i = 0; i < InstancesCount; ++i)
		{
			FeRenderTextureId textureId;
			uint32 iTexIdx = rand() % iTexturesCount;

			pResourcesHandler->LoadTexture(imagesPath[iTexIdx], &textureId);

			
			FeRenderGeometryInstance* pGeomInstance = FE_NEW(FeRenderGeometryInstance, RENDERER_HEAP);
			GeometryInstances.Add(pGeomInstance);

			pGeomInstance->Effect = RENDERER_DEFAULT_EFFECT_ID;
			pGeomInstance->Geometry = FeGeometryHelper::GetStaticGeometry(FeEGemetryDataType::Quad);
			pGeomInstance->Textures.Add(textureId);
		}

		
		return EFeReturnCode::Success;
	}
	uint32 FeModuleUi::Unload()
	{
		return EFeReturnCode::Success;
	}
	uint32 FeModuleUi::Update(const FeDt& fDt)
	{
		static float fRotX = 0, fRotY = 0, fRotZ = 0;
		static FeVector3 translation(0, 0, 0), scale(1, 1, 1);
		static float fIncrement = 0.0f;
		
		int32 iColomns = 60;
		fIncrement += 0.15f*fDt.TotalSeconds;

		float fOffset = (1.01f-abs(sin(fIncrement)))*30.0f;

		float fOffsetBetweenX = 1.f*fOffset;
		float fOffsetBetweenY = 1.0f*fOffset;

		translation.mData[0] = (float)-iColomns;
		translation.mData[1] = 0;
		translation.mData[2] = 50.0f;// -fOffset;

		scale.mData[0] = 1.0f + fOffset;
		scale.mData[1] = 1.0f + fOffset;

		fRotZ = fOffset;

		auto pRenderingModule = FeCommon::FeApplication::StaticInstance.GetModule<FeModuleRendering>();

		const int iBatchMaxSize = 2048;
		uint32 iBatchesCount = (uint32)ceil(GeometryInstances.GetSize() / (float)iBatchMaxSize);
		FeTArray<FeRenderBatch*> batches;

		for (uint32 i = 0; i < iBatchesCount; ++i)
		{
			FeRenderBatch& renderBatch = pRenderingModule->CreateRenderBatch();
			renderBatch.GeometryInstances.Reserve(2048);
			batches.Add(&renderBatch);
		}

		for (uint32 iInstanceIdx = 0; iInstanceIdx < GeometryInstances.GetSize(); ++iInstanceIdx)
		{
			FeRenderGeometryInstance& geomInstance = *GeometryInstances[iInstanceIdx];

			translation.mData[0] = -40 + (iInstanceIdx % iColomns) * fOffsetBetweenX;
			translation.mData[1] = -20 + (iInstanceIdx / iColomns) * fOffsetBetweenY;

			FeGeometryHelper::ComputeAffineTransform(geomInstance.Transform, translation, FeRotation(fRotX, fRotY, fRotZ), scale);

			batches[iInstanceIdx/iBatchMaxSize]->GeometryInstances.Add(geomInstance);
		}
		return EFeReturnCode::Success;
	}

} // namespace FeUi
