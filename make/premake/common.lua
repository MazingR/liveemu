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
c_projectKindExternal	="External"

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
	
	

function __genOrderedIndex( t )
    local orderedIndex = {}
    for key in pairs(t) do
        table.insert( orderedIndex, key )
    end
    table.sort( orderedIndex )
    return orderedIndex
end

function orderedNext(t, state)
    -- Equivalent of the next function, but returns the keys in the alphabetic
    -- order. We use a temporary ordered key table that is stored in the
    -- table being iterated.

    key = nil
    --print("orderedNext: state = "..tostring(state) )
    if state == nil then
        -- the first time, generate the index
        t.__orderedIndex = __genOrderedIndex( t )
        key = t.__orderedIndex[1]
    else
        -- fetch the next value
        for i = 1,table.getn(t.__orderedIndex) do
            if t.__orderedIndex[i] == state then
                key = t.__orderedIndex[i+1]
            end
        end
    end

    if key then
        return key, t[key]
    end

    -- no more value to return, cleanup
    t.__orderedIndex = nil
    return
end

function orderedPairs(t)
    -- Equivalent of the pairs() function on tables. Allows to iterate
    -- in order
    return orderedNext, t, nil
end

function GetProjectTargetDir(config, prj)
	if prj["kind"] == c_projectKindConsoleApp or prj["kind"] == c_projectKindWindowedApp then
		return "../../build/bin/"
	elseif prj["kind"] == c_projectKindExternal then
		return prj["dependencyLibDir"]
	else
		return "../../build/bin/libraries/"..config
	end
end
function GetProjectTargetFullName(config, prj)
	local targetConfig = "targetname_"..config
	local extension = ""
	
	if prj["kind"] == c_projectKindConsoleApp or prj["kind"] == c_projectKindWindowedApp then
		extension = ".exe"
	elseif prj["kind"] == c_projectKindStaticLib then
		extension = ".lib"
	elseif prj["kind"] == c_projectKindSharedLib then
		extension = ".dll"
	end
	
	
	if ProjectHas(prj, targetConfig) then
		return prj[targetConfig]..extension
	else
		return prj["targetname"]..extension
	end
end
function GetProjectTargetName(config, prj)
	local targetConfig = "targetname_"..config
	
	if ProjectHas(prj, targetConfig) then
		return prj[targetConfig]
	else
		return prj["targetname"]
	end
end

function ProcessDependencies(config, prj)
	if ProjectHas(prj, "dependencies") then 
		local dependencies = prj["dependencies"]
		for i = 1, #dependencies do
			local dependency = groups[ dependencies[i][1] ] [ dependencies[i][2] ]
			
			if ProjectHas(dependency, "dependencyInclude") then includedirs { dependency["dependencyInclude"] }	end
			if ProjectHas(dependency, "includedirs") then 		includedirs  { dependency["includedirs"] }		end
			if ProjectHas(dependency, "links") then 			links  { dependency["links"] }					end
			if ProjectHas(dependency, "libdirs") then 			libdirs  { dependency["libdirs"] }				end
			
			links  {dependency["name"], GetProjectTargetFullName(config, dependency) }
			libdirs { GetProjectTargetDir(config, dependency) }
			
			ProcessDependencies(config, dependency)
		end
	end
end
function GenerateProject(config, groupName, projectName)
	local prj = groups[groupName][projectName]
	
	if prj["kind"]~=c_projectKindExternal then
		configuration "Debug"
			defines {"DEBUG"}
		configuration "Release"
			defines {"NDEBUG"}
			flags {"OptimizeSpeed", "FloatFast", "EnableSSE2"}
		
		configuration ""
			if ProjectHas(prj, "name")			then project (prj["name"]) 				end
			location(c_generateDir)
		
		configuration { config }
			flags("Symbols")
		
			if ProjectHas(prj, "kind")			then kind (prj["kind"])					end
			if ProjectHas(prj, "srcPath")		then files { prj["srcPath"] }			end
			if ProjectHas(prj, "includedirs")	then includedirs { prj["includedirs"] }	end
			if ProjectHas(prj, "defines")		then defines{ prj["defines"] }			end
			if ProjectHas(prj, "links")			then links(prj["links"])				end
			if ProjectHas(prj, "excludes")		then excludes(prj["excludes"])			end
			if ProjectHas(prj, "libdirs")		then libdirs(prj["libdirs"])			end
		
			targetname (GetProjectTargetName(config, prj))
			ProcessDependencies(config, prj)		
			targetdir(GetProjectTargetDir(config, prj))
	end
end

function GenerateProjects(config)
	
	for groupName,groupData in orderedPairs(groups) do
		for projectName,projectData in orderedPairs(groupData) do
			print ("["..config.."] : "..projectName)
			GenerateProject(config, groupName, projectName)
		end
	end
end