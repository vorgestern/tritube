
local wks=require "Workspace"

local defines={
    gmake={"GTEST_HAS_PTHREAD=1", "UNITTEST"},
    vsxxxx={"GTEST_HAS_PTHREAD=0", "UNITTEST"}
}

local includedirs={
    vsxxxx={wks.pm.."/buildsys/googletest-1.17.0/googletest/include", wks.pm.."/buildsys/googletest-1.17.0/googletest"}

}

local links={
    gmake={"gtest", "pthread"},
    vsxxxx={"gtest"}
}
local libdirs={
    gmake=nil,
    vsxxxx={}
}

local act=wks.action
if act:match "vs%d+" then act="vsxxxx" end

return {
    defines=defines[act],
    includedirs=includedirs[act],
    links=links[act],
    libdirs=libdirs[act]
}
