groups =
{
	emulator_mame = 
	{
		mame = 
		{
			name = "emulator_mame",
			kind = c_projectKindStaticLib,
			srcPath = 
			{
				c_externals_root.."emulators/common/mame/src/**.inl",
				c_externals_root.."emulators/common/mame/src/**.h",
				c_externals_root.."emulators/common/mame/src/**.hpp",
				c_externals_root.."emulators/common/mame/src/**.c",
				c_externals_root.."emulators/common/mame/src/**.cpp"
			},
			targetname = "mame"
		}
	},
	sdk_sdl = 
	{
		sdl = 
		{
			name = "external_sdl2",
			kind = c_projectKindStaticLib,
			dependencyInclude = 
			{
				c_externals_root.."libraries/common/sdl/include",
			},
			srcPath = 
			{
				c_externals_root.."libraries/common/sdl/include/**.h",
				c_externals_root.."libraries/common/sdl/src/**.h",
				c_externals_root.."libraries/common/sdl/src/**.c",
			},
			excludes =
			{
				-- c_src_root.."**/windows/**",
				
				c_externals_root.."libraries/common/sdl/src/thread/generic/SDL_sysmutex.c",
				c_externals_root.."libraries/common/sdl/src/thread/generic/SDL_syssem.c",
				c_externals_root.."libraries/common/sdl/src/thread/generic/SDL_systhread.c",
				c_externals_root.."libraries/common/sdl/src/thread/generic/SDL_systls.c",
				
				c_externals_root.."libraries/common/sdl/src/**/*_main.c",
				c_externals_root.."libraries/common/sdl/src/**/psp/**",
				c_externals_root.."libraries/common/sdl/src/**/pthread/**",
				c_externals_root.."libraries/common/sdl/src/**/stdcpp/**",
				-- c_externals_root.."libraries/common/sdl/src/**/dummy/**",
				-- c_externals_root.."libraries/common/sdl/src/**/bootstrap/**",
				c_externals_root.."libraries/common/sdl/src/**/android/**",
			},
			links = { "winmm.lib", "imm32.lib", "version.lib" },
			includedirs = { c_externals_root.."libraries/common/sdl/include" },
			defines = {"HAVE_MALLOC"},
			targetname = "sdl2"
		}
	},
	externals = 
	{
		sqllite = 
		{
			name = "external_sqllite",
			kind = c_projectKindStaticLib,
			dependencyInclude = { c_externals_root.."libraries/common/sqllite/src" },
			includedirs = { c_externals_root.."libraries/common/sqllite/src" },
			srcPath = 
			{
				c_externals_root.."libraries/common/sqllite/src/**.c",
				c_externals_root.."libraries/common/sqllite/src/**.h",
			},
			defines = {},
			targetname = "sqllite",
		},
		fontwrapper = 
		{
			kind = c_projectKindExternal,
			dependencyInclude = { c_externals_root.."libraries/windows/fw1fontwrapper/include" },
			dependencyLibDir = { c_externals_root.."libraries/windows/fw1fontwrapper/lib/x86" },
			targetname = "FW1FontWrapper",
		},
		directx = 
		{
			kind = c_projectKindExternal,
			dependencyInclude = { c_externals_root.."libraries/windows/directx/include" },
			dependencyLibDir = { c_externals_root.."libraries/windows/directx/lib/x86" },
			targetname = "directx",
		},
		pthreads = 
		{
			kind = c_projectKindExternal,
			dependencyInclude = { c_externals_root.."libraries/common/pthreads/include" },
			dependencyLibDir = { c_externals_root.."libraries/common/pthreads/lib/x86" },
			targetname = "pthreads",
		},
		freetype = 
		{
			name = "external_freetype",
			kind = c_projectKindStaticLib,
			dependencyInclude = { c_externals_root.."libraries/common/freetype/include" },
			srcPath = 
			{
				c_externals_root.."libraries/common/freetype/include/**.h",
				c_externals_root.."libraries/common/freetype/src/**.h",
				
				c_externals_root.."libraries/common/freetype/src/autofit/autofit.c",
				c_externals_root.."libraries/common/freetype/src/bdf/bdf.c",
				c_externals_root.."libraries/common/freetype/src/cff/cff.c",
				c_externals_root.."libraries/common/freetype/src/base/ftbase.c",
				c_externals_root.."libraries/common/freetype/src/base/ftbitmap.c",
				c_externals_root.."libraries/common/freetype/src/cache/ftcache.c",
				c_externals_root.."libraries/common/freetype/src/base/ftfstype.c",
				c_externals_root.."libraries/common/freetype/src/base/ftgasp.c",
				c_externals_root.."libraries/common/freetype/src/base/ftglyph.c",
				c_externals_root.."libraries/common/freetype/src/gzip/ftgzip.c",
				c_externals_root.."libraries/common/freetype/src/base/ftinit.c",
				c_externals_root.."libraries/common/freetype/src/lzw/ftlzw.c",
				c_externals_root.."libraries/common/freetype/src/base/ftstroke.c",
				c_externals_root.."libraries/common/freetype/src/base/ftsystem.c",
				c_externals_root.."libraries/common/freetype/src/smooth/smooth.c",
				c_externals_root.."libraries/common/freetype/src/base/ftbbox.c",
				c_externals_root.."libraries/common/freetype/src/base/ftfntfmt.c",
				c_externals_root.."libraries/common/freetype/src/base/ftmm.c",
				c_externals_root.."libraries/common/freetype/src/base/ftpfr.c",
				c_externals_root.."libraries/common/freetype/src/base/ftsynth.c",
				c_externals_root.."libraries/common/freetype/src/base/fttype1.c",
				c_externals_root.."libraries/common/freetype/src/base/ftwinfnt.c",
				c_externals_root.."libraries/common/freetype/src/base/ftlcdfil.c",
				c_externals_root.."libraries/common/freetype/src/base/ftgxval.c",
				c_externals_root.."libraries/common/freetype/src/base/ftotval.c",
				c_externals_root.."libraries/common/freetype/src/base/ftpatent.c",
				c_externals_root.."libraries/common/freetype/src/pcf/pcf.c",
				c_externals_root.."libraries/common/freetype/src/pfr/pfr.c",
				c_externals_root.."libraries/common/freetype/src/psaux/psaux.c",
				c_externals_root.."libraries/common/freetype/src/pshinter/pshinter.c",
				c_externals_root.."libraries/common/freetype/src/psnames/psmodule.c",
				c_externals_root.."libraries/common/freetype/src/raster/raster.c",
				c_externals_root.."libraries/common/freetype/src/sfnt/sfnt.c",
				c_externals_root.."libraries/common/freetype/src/truetype/truetype.c",
				c_externals_root.."libraries/common/freetype/src/type1/type1.c",
				c_externals_root.."libraries/common/freetype/src/cid/type1cid.c",
				c_externals_root.."libraries/common/freetype/src/type42/type42.c",
				c_externals_root.."libraries/common/freetype/src/winfonts/winfnt.c",
				
				c_externals_root.."libraries/common/freetype/builds/windows/ftdebug.c"
			},
			includedirs = { c_externals_root.."libraries/common/freetype/include" },
			targetname = "freetype",
			defines = {"WIN32", "_LIB", "_CRT_SECURE_NO_WARNINGS", "FT_DEBUG_LEVEL_ERROR", "FT_DEBUG_LEVEL_TRACE", "FT2_BUILD_LIBRARY"}
		},
	},
	modules = 
	{
		backend = 
		{
			name = "module_backend",
			kind = c_projectKindStaticLib,
			dependencyInclude = { c_src_root.."modules" },
			srcPath = 
			{
				c_src_root.."modules/backend/**.h",
				c_src_root.."modules/backend/**.c",
			},
			includedirs = { c_src_root.."modules/backend" },
			defines = {"FE_D3D9x"},
			targetname = "backend",
			dependencies = 
			{
				{"externals", "directx"},
				{"externals", "pthreads"},
			},
		},
		common = 
		{
			name = "module_common",
			kind = c_projectKindStaticLib,
			dependencyInclude = { c_src_root.."modules" },
			srcPath = 
			{
				c_src_root.."modules/common/**.hpp",
				c_src_root.."modules/common/**.cpp",
			},
			includedirs = { 
				c_src_root.."modules/common",
				c_externals_root.."libraries/common/gmtl/include",
				c_externals_root.."libraries/common/rapidjson/include"},
			targetname = "common",
			dependencies = 
			{
				{"sdk_sdl", "sdl"},
				{"externals", "freetype"},
				{"externals", "sqllite"},
			},
		},
		renderer = 
		{
			name = "module_renderer",
			kind = c_projectKindStaticLib,
			dependencyInclude = { c_src_root.."modules" },
			srcPath = 
			{
				c_src_root.."modules/rendering/**.hpp",
				c_src_root.."modules/rendering/**.cpp",
				c_src_root.."../build/data/themes/common/**.fx",
			},
			links = { "d3d11.lib", "d3dx11.lib", "d3dx11d.lib", "d3dx9.lib", "d3dx9d.lib", "dxguid.lib" },
			includedirs = { c_src_root.."modules/rendering", c_externals_root.."libraries/windows/directx/include" },
			libdirs = { c_externals_root.."libraries/windows/directx/lib/x86" },
			defines = nil,
			targetname = "renderer",
			dependencies = 
			{	
				{"modules", "common"},
				{"externals", "fontwrapper"},
			},
		},
		ui = 
		{
			name = "module_ui",
			kind = c_projectKindStaticLib,
			dependencyInclude = { c_src_root.."modules" },
			srcPath = 
			{
				c_src_root.."modules/ui/**.hpp",
				c_src_root.."modules/ui/**.cpp",
			},
			includedirs = { c_src_root.."modules/ui" },
			defines = nil,
			targetname = "ui",
			dependencies = 
			{
				{"modules", "renderer"},
			},
		}
	},
	runtime = 
	{
		frontend = 
		{
			name = "frontend",
			kind = c_projectKindWindowedApp,
			srcPath = 
			{
				c_src_root.."programs/frontend/**.hpp",
				c_src_root.."programs/frontend/**.cpp",
			},
			includedirs = { c_src_root.."programs/frontend" },
			targetname_Debug = "frontendd",
			targetname_Release = "frontend",
			flags { "WinMain" },
			dependencies = 
			{
				{"modules", "renderer"},
				{"modules", "ui"},
				{"modules", "common"},
			},
		}
	}
}
