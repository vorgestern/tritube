
package.loaded.Workspace=mkworkspace()

premake.path="packages;"..premake.path
addpackage "userlocal"

require "src.tritube_demo"

info()

if package.loaded.Workspace.system~="windows" then
    local aliases=dedent [[
    # Aktiviere diese Abkuerzungen mit 'source alias'.
    alias "pp=clear && premake5 ACT && source alias"
    alias "bb=clear && make -C MP -j 16"
    alias "mm=clear && make -C MP clean && make -C MP -j 16"
    alias "jj=bear -- make -j 16 -BC gmake all"
    gg() {
        grep -rnw "$1" src concept include
    }
    ]]
    for k,v in pairs {
        ACT=package.loaded.Workspace.action,
        MP=path.getname(package.loaded.Workspace.location).." --no-print-directory"} do
        aliases=aliases:gsub(k, v)
    end
    io.writefile(package.loaded.Workspace.pm.."/alias", "\n"..aliases.."\n")
end
