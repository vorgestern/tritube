
#include <string>
#include <vector>
#include <optional>

namespace tritube
{
    const enum class usecase {a,b,c,d,e} ttwholetext=usecase::a, ttlinewise=usecase::b;
    const enum class xfind {a,b,c,d,e} xfliteral=xfind::a, xfrelative=xfind::b, xfpath=xfind::c;

    template<typename resultform, xfind where>resultform exec_sync(std::string_view exec, std::string_view args);
}
