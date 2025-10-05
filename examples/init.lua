
local Workspace=require "Workspace"

if Workspace.system=="windows" then
    error "These examples work on Linux only."
end

project "echoserver"
kind "ConsoleApp"
files "echoserver.cpp"
includedirs "../include"
cppdialect "C++20"
table.insert(Workspace.projects, ConsoleApp "echoserver")

if nil then
	project "echodemo"
	kind "ConsoleApp"
	files "echodemo.cpp"
	includedirs "../include"
	links "tritube"
	cppdialect "C++20"
	table.insert(Workspace.projects, ConsoleApp "echodemo")
end

project "compile1"
kind "ConsoleApp"
files "compile1.cpp"
includedirs "../include"
links "tritube"
cppdialect "C++20"
table.insert(Workspace.projects, ConsoleApp "compile1")
