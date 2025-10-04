
package.loaded.Workspace=mkworkspace()

premake.path="packages;"..premake.path
addpackage "userlocal"

require "src.tritube"
require "src.tritube_demo"
require "src.test_helper"

info()

if package.loaded.Workspace.system=="windows" then
    local aliases=dedent [[
    # Aktiviere diese Abkuerzungen mit 'source alias'.
    alias xx="PATH=`cygpath -a bin`:$PATH bin/tritube_demo test_helper.exe -p 10 -r 3"
    alias pp="clear && premake5 ACT && source alias"
    gg() {
        grep -rnw "$1" src include
    }
    ]]
    for k,v in pairs {
        ACT=package.loaded.Workspace.action,
        MP=path.getname(package.loaded.Workspace.location).." --no-print-directory"} do
        aliases=aliases:gsub(k, v)
    end
    io.writefile(package.loaded.Workspace.pm.."/alias", "\n"..aliases.."\n")
else
    local aliases=dedent [[
    # Aktiviere diese Abkuerzungen mit 'source alias'.
    alias "xx=PATH=`realpath -s bin`:$PATH bin/tritube_demo test_helper.exe -p 10 -r 3"
    alias "pp=clear && premake5 ACT && source alias"
    alias "bb=clear && make -C MP -j 16"
    alias "mm=clear && make -C MP clean && make -C MP -j 16"
    alias "jj=bear -- make -j 16 -BC gmake all"
    gg() {
        grep -rnw "$1" src include examples
    }
    ]]
    for k,v in pairs {
        ACT=package.loaded.Workspace.action,
        MP=path.getname(package.loaded.Workspace.location).." --no-print-directory"} do
        aliases=aliases:gsub(k, v)
    end
    io.writefile(package.loaded.Workspace.pm.."/alias", "\n"..aliases.."\n")
end
