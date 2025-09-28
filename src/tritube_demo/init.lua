
local Workspace=require "Workspace"

project "tritube_demo"
kind "ConsoleApp"
files {"*.cpp", "../../include/tritube/*.h"}

includedirs {"../../include"}

if Workspace.system=="windows" then
    links "msvcrtd.lib"
    linkoptions "/NODEFAULTLIB:libcmt.lib"
    vpaths {
        [""]="*.cpp",
        include="../../include/tritube/*.h"
    }
else
    links {}
end

optimize "on"
warnings "high"
debugger "GDB"
omitframepointer "on"
cppdialect "C++20"
disablewarnings {4100}

table.insert(Workspace.projects, ConsoleApp "tritube_demo")
