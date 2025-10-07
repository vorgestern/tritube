
local wks=require "Workspace"
local userlocal=wks.packages.userlocal

project "tritube"
kind "StaticLib"
files "../../include/tritube/*.h"

includedirs {"../../include"}

if wks.system=="windows" then
    files {"win32/*.cpp", "win32/*.h"}
    vpaths {
        [""]={"../../include/tritube/*.h"},
        win32={"win32/*.cpp", "win32/*.h"}
    }
    disablewarnings {4100}
    local pbc={}
    if userlocal.libdir     then table.insert(pbc, string.format("{COPYFILE} %s %s", "%{cfg.linktarget.abspath}", userlocal.libdir)) end
    if userlocal.includedir then table.insert(pbc, string.format("{COPYDIR} %s %s",  wks.pm.."/include/tritube",  userlocal.includedir.."/tritube/")) end
    postbuildcommands(pbc)
elseif wks.system=="linux" then
    files {"linux/*.cpp", "linux/*.h"}
end

optimize "on"
warnings "high"
debugger "GDB"
omitframepointer "on"
cppdialect "C++20"

table.insert(wks.projects, StaticLib "tritube")
