
local fpp=   require "luafpp"
local alltag=require "alltag"

-- Determine include- and lib-directories
local function installerinfo()
    local home=os.getenv "HOME"
    if not home then error "HOME not set" end
    local result={}
    for _,p in ipairs {".local/lib", ".local/robinson/lib"} do
        if not result.lib and fpp.exists(home.."/"..p) then
            result.lib=home.."/"..p
        end
    end
    for _,p in ipairs {".local/include", ".local/robinson/include"} do
        if not result.include and fpp.exists(home.."/"..p) then
            result.include=home.."/"..p
        end
    end
    if not result.lib or not result.include then
        error "installerinfo not complete"
    end
    return result
end

local info=installerinfo()

-- Copy src/forkpipes.h to include/tritube/forkpipes.h.
-- Copy libtritube.a    to lib/*

local result,code=alltag.pipe_lines("make", print)
if result==0 then
    local res,err=fpp.mkdir(info.include.."/tritube")
    if not res then error(err) end

    local text=io.open("src/forkpipes.h"):read("*a")
    if not text then
        error "Cannot find/read header"
    end
    io.output(info.include.."/tritube/forkpipes.h"):write(text)
    io.output():close()

    local content=io.open("libtritube.a", "rb"):read("*a")
    if not content then error "Cannot find/read libtritube.a" end
    io.open(info.lib.."/libtritube.a", "wb"):write(content):close()
end
