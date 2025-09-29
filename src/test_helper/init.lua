
local Workspace=require "Workspace"

project "test_helper"
kind "ConsoleApp"
files "*.cpp"

includedirs {}

if Workspace.system=="windows" then
    links "msvcrtd.lib"
    linkoptions "/NODEFAULTLIB:libcmt.lib"
    vpaths {
        [""]="*.cpp",
    }
end

optimize "on"
warnings "high"
debugger "GDB"
omitframepointer "on"
cppdialect "C++20"

table.insert(Workspace.projects, ConsoleApp "test_helper")
