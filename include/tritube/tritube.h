
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <filesystem>

std::string piper4(std::filesystem::path&fullpath, std::string_view args);
std::string piper5(std::filesystem::path&fullpath, std::string_view args);

namespace tritube
{
    const enum class xfind {a,b,c,d,e} xfliteral=xfind::a, xfrelative=xfind::b, xfpath=xfind::c;

    template<xfind X> std::optional<std::filesystem::path> applpath(std::string_view simple_name);

//  template<typename resulttype>resulttype exec_sync1(std::filesystem::path&, std::string_view args);

    inline std::string exec_sync1(std::filesystem::path&exec, std::string_view args)
    {
        return piper4(exec, args);
    }

    template<typename resulttype, xfind X>resulttype exec_sync(std::string_view exec, std::string_view args)
    {
        if (auto fullpath=applpath<X>(exec); fullpath.has_value())
        {
            std::filesystem::path P=fullpath.value();
            return exec_sync1(P, args);
        }
        else return {};
    }
}
