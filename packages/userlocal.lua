
local wks=require "Workspace"

local act=wks.action
if act:match "vs%d+" then act="vsxxxx" end

local bindir={
    gmake=os.getenv("HOME").."/.local/bin",
    vsxxxx=nil
}

local libdir={
    gmake=os.getenv("HOME").."/.local/lib",
    vsxxxx=nil
}

local includedir={
    gmake=os.getenv("HOME").."/.local/include/",
    vsxxxx=nil
}

-- ===============================================

local includedirs={
    gmake={os.getenv("HOME").."/.local/include"},
    vsxxxx=nil
}

local libdirs={
    gmake={os.getenv("HOME").."/.local/lib"},
    vsxxxx=nil
}

-- ===============================================

if act=="vsxxxx" then
    local ok,X=pcall(dofile, "packages/userlocal_vsxxxx.lua")
    if ok then
        bindir.vsxxxx=     X.bindir
        libdir.vsxxxx=     X.libdir
        includedir.vsxxxx= X.includedir
        includedirs.vsxxxx=X.includedirs
        libdirs.vsxxxx=    X.libdirs
    end
elseif act=="gmake" and wks.system=="linux" then
    local ok,X=pcall(dofile, "packages/userlocal_gmake.lua")
    if ok then
        bindir.gmake=     X.bindir
        libdir.gmake=     X.libdir
        includedir.gmake= X.gmake
        includedirs.gmake=X.includedirs
        libdirs.gmake=    X.libdirs
    end
end

return {
    bindir=     bindir[act],
    libdir=     libdir[act],
    includedir= includedir[act],

    includedirs=includedirs[act],
    libdirs=    libdirs[act],
}
