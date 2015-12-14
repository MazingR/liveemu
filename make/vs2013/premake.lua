dofile('../premake/common.lua')
dofile('../premake/projects.lua')

c_workspaceName = "Liveemu_vs2013"

workspace(c_workspaceName)
	configurations { "Debug", "Release" }
	startproject "frontend"
	
	GenerateProjects("Debug")
	GenerateProjects("Release")
