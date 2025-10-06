
local ok,ulutest=pcall(require, "ulutest")
if not ok then
    error([[

    This is a unit test using the framework ulutest.
    However, require "ulutest" failed."
    Install it from http:github.com/vorgestern/ulutest."]])
end

local function mytest(name) return function(list) list.name=name return list end end
local function tt(name) return function(func) return ulutest.TT(name, func) end end

local pipe_lines=function(command)
    local X={}
    local pipe=io.popen(command)
    if pipe then
        for line in pipe:lines() do
            table.insert(X, line)
            -- print("==",line,"==")
        end
        local flag,status,rc=pipe:close()
        if flag then return X, math.tointeger(rc), status
        else return {}, math.tointeger(rc), status
        end
    else
        error(string.format("Error running '%s'", command))
    end
end

local ut={

mytest "available" {
    tt "tritube_demo (system under test)" (function(t)
        local ok=io.open("bin/tritube_demo.exe", "r")
        if ok then ok:close() end
        t:ASSERT(ok)
    end),
    tt "helper (test_helper)" (function(t)
        local ok=io.open("bin/test_helper.exe", "r")
        if ok then ok:close() end
        t:ASSERT(ok)
    end),
},

mytest "tritube_demo" {
    tt "usecase_print" (function(t)
        local X=pipe_lines([[tritube_demo --usecase print -- test_helper]])
        -- print(table.concat(X,"\n"))
        local m=X[1]:match "test_helper%.EXE\""
        -- print(m)
        t:ASSERT_EQ("string", type(m))
    end),
    tt "usecase_stdout" (function(t)
        local X=pipe_lines([[tritube_demo --usecase stdout --raw -- test_helper]])
        t:ASSERT_EQ(9, #X) -- Zeile 5 geht nach stderr.
    end),
    tt "usecase_roe" (function(t)
        local X=pipe_lines([[tritube_demo --usecase roe --raw -- test_helper]])
        -- print(table.concat(X, "\n"))
        local rc,numout,numerr=X[1]:match "(%d+) (%d+) (%d+)"
        t:ASSERT_EQ(0, math.tointeger(rc))
        t:ASSERT_EQ(9, math.tointeger(numout))
        t:ASSERT_EQ(1, math.tointeger(numerr))
    end),
    tt "usecase_roev" (function(t)
        local X=pipe_lines([[tritube_demo --usecase roev --raw -- test_helper]])
        -- print(table.concat(X, "\n"))
        local rc,numout,numerr=X[1]:match "(%d+) (%d+) (%d+)"
        t:ASSERT_EQ(0, math.tointeger(rc))
        t:ASSERT_EQ(9, math.tointeger(numout))
        t:ASSERT_EQ(1, math.tointeger(numerr))
    end),
    tt "usecase_linewise" (function(t)
        local X=pipe_lines([[tritube_demo --usecase linewise --raw -- test_helper]])
        -- print(table.concat(X, "\n"))
        local rc,numout,numerr=X[1]:match "(%d+) (%d+) (%d+)"
        t:ASSERT_EQ(0, math.tointeger(rc))
        t:ASSERT_EQ(9, math.tointeger(numout))
        t:ASSERT_EQ(1, math.tointeger(numerr))
    end)
},

}

ulutest.RUN(ut)
