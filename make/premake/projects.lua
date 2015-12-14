groups =
{
	emulator_mame = 
	{
		-- vmame = 
		-- {
			-- name = "emulator_vmame",
			-- kind = c_projectKindStaticLib,
			-- srcPath = 
			-- {
				-- c_src_root.."externals/emulators/mame/src/**.h",
				-- c_src_root.."externals/emulators/mame/src/**.c",
				-- c_src_root.."externals/emulators/mame/src/**.cpp"
			-- },
			-- defines = nil,
			-- targetname = "vmame"
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
			defines = nil,
			targetname = "sdl2"
		}
	},
	modules = 
	{
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
			includedirs = { c_src_root.."modules/common"},
			targetname = "common",
			dependencies = 
			{
				{"sdk_sdl", "sdl"},
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
				{"sdk_sdl", "sdl"},
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
				{"modules", "common"},
				{"sdk_sdl", "sdl"},
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
				{"sdk_sdl", "sdl"},
				{"modules", "renderer"},
				{"modules", "ui"},
				{"modules", "common"},
			},
		}
	}
}