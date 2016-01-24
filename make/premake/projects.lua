groups =
{
	emulator_mame = 
	{
		-- mame = 
		-- {
			-- name = "emulator_mame",
			-- kind = c_projectKindStaticLib,
			-- srcPath = 
			-- {
				-- c_src_root.."externals/emulators/mame/src/**.inl",
				-- c_src_root.."externals/emulators/mame/src/**.h",
				-- c_src_root.."externals/emulators/mame/src/**.hpp",
				-- c_src_root.."externals/emulators/mame/src/**.c",
				-- c_src_root.."externals/emulators/mame/src/**.cpp"
			-- },
			-- defines = nil,
			-- targetname = "mame"
		-- }
	},
	sdk_sdl = 
	{
		sdl = 
		{
			name = "external_sdl2",
			kind = c_projectKindStaticLib,
			dependencyInclude = 
			{
				c_src_root.."externals/sdl/include",
			},
			srcPath = 
			{
				c_src_root.."externals/sdl/include/**.h",
				c_src_root.."externals/sdl/src/**.h",
				c_src_root.."externals/sdl/src/**.c",
			},
			excludes =
			{
				-- c_src_root.."**/windows/**",
				
				c_src_root.."externals/sdl/src/thread/generic/SDL_sysmutex.c",
				c_src_root.."externals/sdl/src/thread/generic/SDL_syssem.c",
				c_src_root.."externals/sdl/src/thread/generic/SDL_systhread.c",
				c_src_root.."externals/sdl/src/thread/generic/SDL_systls.c",
				
				c_src_root.."**/*_main.c",
				
				c_src_root.."**/psp/**",
				c_src_root.."**/pthread/**",
				c_src_root.."**/stdcpp/**",
				-- c_src_root.."**/dummy/**",
				-- c_src_root.."**/bootstrap/**",
				c_src_root.."**/android/**",
			},
			links = { "winmm.lib", "imm32.lib", "version.lib" },
			includedirs = { c_src_root.."externals/sdl/include" },
			defines = {"HAVE_MALLOC"},
			targetname = "sdl2"
		}
	},
	externals = 
	{
		fontwrapper = 
		{
			kind = c_projectKindExternal,
			dependencyInclude = { c_src_root.."externals/windows/FW1FontWrapper/include" },
			dependencyLibDir = { c_src_root.."externals/windows/FW1FontWrapper/lib/x86" },
			targetname = "FW1FontWrapper",
		},
		directx = 
		{
			kind = c_projectKindExternal,
			dependencyInclude = { c_src_root.."externals/windows/directx/include" },
			dependencyLibDir = { c_src_root.."externals/windows/directx/lib/x86" },
			targetname = "directx",
		},
		pthreads = 
		{
			kind = c_projectKindExternal,
			dependencyInclude = { c_src_root.."externals/pthreads/include" },
			dependencyLibDir = { c_src_root.."externals/pthreads/lib/x86" },
			targetname = "pthreads",
		},
		freetype = 
		{
		name = "external_freetype",
			kind = c_projectKindStaticLib,
			dependencyInclude = { c_src_root.."externals/freetype/include" },
			srcPath = 
			{
				c_src_root.."externals/freetype/include/**.h",
				c_src_root.."externals/freetype/src/**.h",
				
				c_src_root.."externals/freetype/src/autofit/autofit.c",
				c_src_root.."externals/freetype/src/bdf/bdf.c",
				c_src_root.."externals/freetype/src/cff/cff.c",
				c_src_root.."externals/freetype/src/base/ftbase.c",
				c_src_root.."externals/freetype/src/base/ftbitmap.c",
				c_src_root.."externals/freetype/src/cache/ftcache.c",
				c_src_root.."externals/freetype/src/base/ftfstype.c",
				c_src_root.."externals/freetype/src/base/ftgasp.c",
				c_src_root.."externals/freetype/src/base/ftglyph.c",
				c_src_root.."externals/freetype/src/gzip/ftgzip.c",
				c_src_root.."externals/freetype/src/base/ftinit.c",
				c_src_root.."externals/freetype/src/lzw/ftlzw.c",
				c_src_root.."externals/freetype/src/base/ftstroke.c",
				c_src_root.."externals/freetype/src/base/ftsystem.c",
				c_src_root.."externals/freetype/src/smooth/smooth.c",
				c_src_root.."externals/freetype/src/base/ftbbox.c",
				c_src_root.."externals/freetype/src/base/ftfntfmt.c",
				c_src_root.."externals/freetype/src/base/ftmm.c",
				c_src_root.."externals/freetype/src/base/ftpfr.c",
				c_src_root.."externals/freetype/src/base/ftsynth.c",
				c_src_root.."externals/freetype/src/base/fttype1.c",
				c_src_root.."externals/freetype/src/base/ftwinfnt.c",
				c_src_root.."externals/freetype/src/base/ftlcdfil.c",
				c_src_root.."externals/freetype/src/base/ftgxval.c",
				c_src_root.."externals/freetype/src/base/ftotval.c",
				c_src_root.."externals/freetype/src/base/ftpatent.c",
				c_src_root.."externals/freetype/src/pcf/pcf.c",
				c_src_root.."externals/freetype/src/pfr/pfr.c",
				c_src_root.."externals/freetype/src/psaux/psaux.c",
				c_src_root.."externals/freetype/src/pshinter/pshinter.c",
				c_src_root.."externals/freetype/src/psnames/psmodule.c",
				c_src_root.."externals/freetype/src/raster/raster.c",
				c_src_root.."externals/freetype/src/sfnt/sfnt.c",
				c_src_root.."externals/freetype/src/truetype/truetype.c",
				c_src_root.."externals/freetype/src/type1/type1.c",
				c_src_root.."externals/freetype/src/cid/type1cid.c",
				c_src_root.."externals/freetype/src/type42/type42.c",
				c_src_root.."externals/freetype/src/winfonts/winfnt.c",
				
				c_src_root.."externals/freetype/builds/windows/ftdebug.c"
			},
			includedirs = { c_src_root.."externals/freetype/include" },
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
				c_src_root.."externals/gmtl/include",
				c_src_root.."externals/rapidjson/include"},
			targetname = "common",
			dependencies = 
			{
				{"sdk_sdl", "sdl"},
				{"externals", "freetype"},
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
			includedirs = { c_src_root.."modules/rendering", c_src_root.."externals/windows/directx/include" },
			libdirs = { c_src_root.."externals/windows/directx/lib/x86" },
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
