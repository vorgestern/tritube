
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <filesystem>

std::string piper4(std::filesystem::path&fullpath, std::string_view args);
std::string piper5(std::filesystem::path&fullpath, std::string_view args);

namespace tritube
{
    using rc_out_err=std::tuple<int,std::string,std::string>;
    const enum class xfind {a,b,c,d,e} xfliteral=xfind::a, xfrelative=xfind::b, xfpath=xfind::c;

    rc_out_err piper4_roe(std::filesystem::path&fullpath, std::string_view args);

    template<xfind X> std::optional<std::filesystem::path> applpath(std::string_view exec_name);

    inline std::string exec_sync1(std::filesystem::path&exec, std::string_view args)
    {
        return piper4(exec, args);
    }

    template<typename resulttype, xfind X>resulttype exec_sync(std::string_view exec_name, std::string_view args)
    {
        if (auto fullpath=applpath<X>(exec_name); fullpath.has_value())
        {
            std::filesystem::path P=fullpath.value();
            return exec_sync1(P, args);
        }
        else return {};
    }

    template<typename resulttype> resulttype exec_simple(std::filesystem::path&exec, std::string_view args);
    template<> inline std::string exec_simple<std::string>(std::filesystem::path&exec, std::string_view args)
    {
        return piper4(exec, args);
    }
    template<> inline rc_out_err exec_simple<rc_out_err>(std::filesystem::path&exec, std::string_view args)
    {
        return piper4_roe(exec, args);
    }
}
