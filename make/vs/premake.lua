workspace "Liveemu"
	configurations { "Debug", "Release" }

project "Liveemu"
	kind "ConsoleApp"
	language "C++"
	targetdir "../../build/%{cfg.buildcfg}"

	files { 
	"../../src/modules/**.hpp",
	"../../src/modules/**.cpp",
	"../../src/programs/frontend/**.cpp",
	"../../src/programs/frontend/**.hpp"
	}
	libdirs {
		"../../src/externals/sdl/lib/windows/x86",
	}
	links {"SDL2.lib", "SDL2main.lib", "SDL2.lib"}
	includedirs {"../../src/externals/sdl/include"}

	filter "configurations:Debug"
		defines { "DEBUG" }
		flags { "Symbols" }

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"