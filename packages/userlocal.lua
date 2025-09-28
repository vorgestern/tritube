
local wks=require "Workspace"

local act=wks.action
if act:match "vs%d+" then act="vsxxxx" end

local targetdir={
    gmake=os.getenv("HOME").."/.local/bin",
    vsxxxx=nil
}

local includedirs={
    gmake=os.getenv("HOME").."/.local/include",
    vsxxxx=nil
}

local libdirs={
    gmake=os.getenv("HOME").."/.local/lib",
    vsxxxx=nil
}

return {
    targetdir=targetdir[act],
    includedirs=includedirs[act],
    libdirs=libdirs[act],
}
