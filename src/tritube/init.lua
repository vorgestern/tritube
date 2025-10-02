
local Workspace=require "Workspace"

project "tritube"
kind "StaticLib"
files {"*.cpp", "../../include/tritube/*.h"}

includedirs {"../../include"}

if Workspace.system=="windows" then
    files {"win32/*.cpp", "win32/*.h"}
    vpaths {
        [""]={"*.cpp", "../../include/tritube/*.h"},
        win32={"win32/*.cpp", "win32/*.h"}
    }
	disablewarnings {4100}
elseif Workspace.system=="linux" then
    files {"linux/*.cpp", "linux/*.h"}
	files "../forkpipes*"
end

optimize "on"
warnings "high"
debugger "GDB"
omitframepointer "on"
cppdialect "C++20"

table.insert(Workspace.projects, StaticLib "tritube")
