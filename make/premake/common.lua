newoption 
{
	trigger		= "vs_express",
	description	= "Build solution with Express limitation (no folders)"
}

newoption
{
	trigger		= "target_platform",
	description = "Build solution for a specified platform.",
	allowed = 
	{
		{	"windows",	"Windows platform"	},
		{	"linux",	"Linux platform" },
		{	"mac",		"Apple Mac Os platform" }
	}
}

newoption
{
	trigger		= "projects",
	description = "Choose which project to generate.",
	allowed = 
	{
		{	"program_frontend",	"Runtime program"	},
		{	"program_editor",	"Editor program"	},
		
		{	"modules",			"Module libraries (rendering, ui, ...)" },
		
		{	"emulators",		"Emulator projects" },
		
		{	"emulator_mame",	"Emulator Mame (Arcade)" },
		{	"emulator_demul",	"Emulator Demul (Sega Dreamcast, Naomi, Atomiswave)" },
		{	"emulator_dolphin",	"Emulator Dolphin (Nintendo Gamecube, Wii)" },
		{	"emulator_yabause",	"Emulator Yabause (Sega Saturn)" },
		
		{	"all",				"All projects (progams + modules + externals)" }
	}
}


function ProjectHas(_project, tag)
	if	_project[tag]  ~= nil then
		return true
	else
		return false
	end
end

c_src_root = "../../src/"
c_projectKindConsoleApp	="ConsoleApp"
c_projectKindWindowedApp="WindowedApp"
c_projectKindSharedLib	="SharedLib"
c_projectKindStaticLib	="StaticLib"

c_generateDir = "projects"
c_workspaceName = "Liveemu"
c_targetDir			="../../bin/"

 -- = scripts/genie.lua \
	-- scripts/src/lib.lua \
	-- scripts/src/emu.lua \
	-- scripts/src/machine.lua \
	-- scripts/src/main.lua \
	-- scripts/src/3rdparty.lua \
	-- scripts/src/cpu.lua \
	-- scripts/src/osd/modules.lua \
	-- scripts/src/osd/windows*.lua) \
	-- scripts/src/sound.lua \
	-- scripts/src/tools.lua \
	-- scripts/src/tests.lua \
	-- scripts/src/video.lua \
	-- scripts/src/bus.lua \
	-- scripts/src/netlist.lua \
	-- scripts/toolchain.lua \
	-- scripts/src/osd/modules.lua \
	

function GetProjectTargetDir(config, prj)
	if prj["kind"] == c_projectKindConsoleApp or prj["kind"] == c_projectKindWindowedApp then
		return "../../build/bin/"..config
	else
		return "../../build/bin/"..config.."/intermediate"
	end
end
function GetProjectTargetFullName(prj)
	if prj["kind"] == c_projectKindConsoleApp or prj["kind"] == c_projectKindWindowedApp then
		return prj["targetname"]..".exe"
	elseif prj["kind"] == c_projectKindStaticLib then
		return prj["targetname"]..".lib"
	elseif prj["kind"] == c_projectKindSharedLib then
		return prj["targetname"]..".dll"
	end
end

function GenerateProject(config, groupName, projectName)
	local prj = groups[groupName][projectName]
	
	configuration ""
	if ProjectHas(prj, "name")			then project (prj["name"]) 				end
	location(c_generateDir)
	
	
	configuration { config }
		flags("Symbols")
	
		if ProjectHas(prj, "kind")			then kind (prj["kind"])					end
		if ProjectHas(prj, "srcPath")		then files { prj["srcPath"] }			end
		if ProjectHas(prj, "includedirs")	then includedirs { prj["includedirs"] }	end
		if ProjectHas(prj, "defines")		then defines{ prj["defines"] }			end
		if ProjectHas(prj, "targetname")	then targetname(prj["targetname"])		end
		if ProjectHas(prj, "links")			then links(prj["links"])				end
		if ProjectHas(prj, "excludes")		then excludes(prj["excludes"])			end
		
	configuration { config }
		
		-- Fetch depdencies include dirs and lib names to link
		if ProjectHas(prj, "dependencies") then 
			local dependencies = prj["dependencies"]
			for i = 1, #dependencies do
				local dependency = groups[ dependencies[i][1] ] [ dependencies[i][2] ]
				
				if ProjectHas(dependency, "dependencyInclude") then 
					includedirs { dependency["dependencyInclude"] }
				end
				if ProjectHas(dependency, "links") then 
					links  { dependency["links"] }
				end
				
				links  { GetProjectTargetFullName(dependency) }
				libdirs { GetProjectTargetDir(config, dependency) }
			end
		end
	configuration { config }
		targetdir(GetProjectTargetDir(config, prj))
end

function GenerateProjects(config)
	for groupName,groupData in pairs(groups) do
		for projectName,projectData in pairs(groupData) do
			GenerateProject(config, groupName, projectName)
		end
	end
end


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
				c_src_root.."**/dummy/**",
				c_src_root.."**/bootstrap/**",
				c_src_root.."**/android/**",
			},
			links = { "winmm.lib", "imm32.lib", "version.lib" },
			includedirs = { c_src_root.."externals/sdl/include" },
			defines = nil,
			targetname = "sdl2"
		}
	},
	runtime = 
	{
		frontend = 
		{
			name = "frontend",
			kind = c_projectKindConsoleApp,
			srcPath = 
			{
				c_src_root.."programs/frontend/**.hpp",
				c_src_root.."programs/frontend/**.cpp",
			},
			includedirs = { c_src_root.."programs/frontend" },
			defines = nil,
			targetname = "frontend",
			dependencies = 
			{
				{"sdk_sdl", "sdl"}
			},
		}
	}
}
