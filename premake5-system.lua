
local mtprojectentry={
    __tostring=function(self) return self.typ.." '"..self.name.."'" end,
    __index=function(self, key)
        local publickeys={name=1, typ=2}
        key=publickeys[key]
        return self[key] or error("mtprojectentry has no key "..tostring(key))
    end
}
function WindowedApp(name) return setmetatable({name=name, typ="WindowedApp"}, mtprojectentry) end
function ConsoleApp(name) return setmetatable({name=name, typ="ConsoleApp"}, mtprojectentry) end
function SharedLib(name) return setmetatable({name=name, typ="SharedLib"}, mtprojectentry) end
function StaticLib(name) return setmetatable({name=name, typ="StaticLib"}, mtprojectentry) end

local function reste()
    dependson {}
    startproject {}
    undefines {}
    staticruntime "off" -- Sollte "off" sein per default
    buildoptions {}
    linkoptions {
        -- "/INCREMENTAL:NO" Das scheint nicht zu funktionieren
            -- Soll die Erzeugung von ilk Dateien verhindern.
            -- Diese sind angeblich ohnehin nur in Debug-Konfigurationen sinnvoll.
    }
end

function mkworkspace()
    local os={linux="linux", windows="windows"}
    local act={gmake="gmake", vs2022="vs2022"}
    local osact={
        linux={gmake="gmake"},
        windows={gmake="cygmake2", vs2022="vs2022"}
    }
    local Wks={
        system=os[_TARGET_OS] or error(string.format("Target System %s unknown", _TARGET_OS)),
        action=act[_ACTION] or error(string.format("Build System %s unknown", _ACTION)),
        configurations={"Debug"},
        location=path.getabsolute(osact[_TARGET_OS][_ACTION]),
        pm=path.normalize(path.getabsolute(_MAIN_SCRIPT_DIR)),
        obj=path.normalize(path.getabsolute(_MAIN_SCRIPT_DIR.."/oo/obj")),
        lib=path.normalize(path.getabsolute(_MAIN_SCRIPT_DIR.."/oo/lib")),
        projects={},
        packages={}
    }
    workspace "tritube"
    location (Wks.location)
    configurations (Wks.configurations)
    objdir (Wks.obj.."/%{prj.name}")
    filter "system:Windows"
        system "windows"
        architecture "x86_64"
        platforms "Win64"
        includedirs "buildsys/include"
        libdirs "buildsys/libdbg64"
        defines {"WIN32", "_WIN32", "_WINDOWS", "WINDOWS", "WIN32_LEAN_AND_MEAN", "_CRT_SECURE_NO_WARNINGS"}
        staticruntime "on"
        characterset "ASCII"
        -- Unter cygwin habe ich die Projekte nur als 64bit-Projete linken können:
        -- architecture "x86_64"
        -- platforms "Win64"
        -- Es kann natürlich sein, dass man unter cygwin 32bit-Unterstützung ausdrücklich installieren muss.
    filter "system:Unix"
        system "linux"
        architecture "x86_64"
        platforms "x86_64"
    filter "kind:StaticLib"
        targetdir (Wks.lib)
        defines "_LIB"
    filter "kind:SharedLib"
        targetdir "bin"
        libdirs (Wks.lib)
    filter {"kind:SharedLib", "system:Windows"}
        implibdir (Wks.lib)
        implibsuffix "_import"
    filter "kind:WindowedApp"
        targetdir "bin"
        libdirs (Wks.lib)
    filter "kind:ConsoleApp"
        targetdir "bin"
        libdirs (Wks.lib)
    filter "configurations:*"
        includedirs "src/include"
    filter "configurations:Release"
        defines "NDEBUG"
        flags "NoIncrementalLink"
    filter "configurations:Debug"
        defines "_DEBUG"
    filter {"system:Windows", "kind:SharedLib or kind:ConsoleApp or kind:WindowedApp"}
        symbolspath "$(IntDir)$(TargetName).pdb"
    -- filter {"system:Linux", "kind:SharedLib"}
        -- pic "on" -- Diese Einstellung ist eigentlich unnötig, weil default für DLLs.
    filter ()
    language "C"
    optimize "Speed"
    flags "MultiProcessorCompile"
    return Wks
end

function info()
    -- print("\n_ACTION         ", _ACTION)
    -- print("_ARGS           ", "{"..table.concat(_ARGS, ", ").."}")
    -- print("_MAIN_SCRIPT_DIR", _MAIN_SCRIPT_DIR)
    -- print("_MAIN_SCRIPT    ", _MAIN_SCRIPT)
    -- print("_OPTIONS        ", "{"..table.concat(_OPTIONS, ", ").."}")
    -- print("_TARGET_OS      ", _TARGET_OS)
    -- print("_PREMAKE_COMMAND", _PREMAKE_COMMAND)
    -- print("_PREMAKE_DIR    ", _PREMAKE_DIR)
    -- print("_PREMAKE_VERSION", _PREMAKE_VERSION)
    -- print("_WORKING_DIR    ", _WORKING_DIR)
    local wks=require "Workspace"
    print "\nSalient facts:"
    print("system   ", wks.system)
    print("location ", wks.location)
    print("pm       ", wks.pm)
    print("obj      ", wks.obj)
    print("lib      ", wks.lib)
    print "\nProjects:"
    for k,v in ipairs(require "Workspace" .projects) do print(k,v) end
    print ""
end

function dedent(text)
    local A={}
    local indentation
    for line in text:gmatch "([^\n]*)\n" do table.insert(A, line) end
    for j,k in ipairs(A) do
        local indent,rest=k:match "^( *)(.*)"
        local d=string.len(indent or "")
        if not indentation or (d<indentation and string.len(rest)>0) then indentation=d end
    end
    if not indentation or indentation==0 then return text end
    local B={}
    for j,k in ipairs(A) do table.insert(B, string.sub(k, indentation+1)) end
    return table.concat(B, "\n")
end

function addpackage(name)
    local wks=require "Workspace"
    if not wks.usepackage then
        wks.usepackage=function(p)
            includedirs(p.includedirs)
            defines(p.defines)
            links(p.links)
        end
    end
    if not wks.usepackages then
        wks.usepackages=function(P)
            for _,p in ipairs(P) do
                includedirs(p.includedirs)
                defines(p.defines)
                links(p.links)
            end
        end
    end
    if not wks.packages then wks.packages={} end
    local X=require(name)
    wks.packages[name]=X
    return X
end
