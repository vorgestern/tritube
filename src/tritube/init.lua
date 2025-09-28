
local Workspace=require "Workspace"

project "tritube"
kind "StaticLib"
files {"**.cpp", "../../include/tritube/*.h"}

includedirs {"../../include"}

if Workspace.system=="windows" then
    vpaths {
        [""]="*.cpp",
        win32="win32/*.cpp",
        include="../../include/tritube/*.h"
    }
end

optimize "on"
warnings "high"
debugger "GDB"
omitframepointer "on"
cppdialect "C++20"
disablewarnings {4100}

table.insert(Workspace.projects, StaticLib "tritube")
