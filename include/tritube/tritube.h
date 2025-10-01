
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <filesystem>
#include <functional>

namespace tritube
{
    using rc_out_err=std::tuple<int,std::string,std::string>;
    using rc_Out_Err=std::tuple<int,std::vector<std::string>,std::vector<std::string>>;
    const enum class xfind {a,b} xfdirect=xfind::a, xfpath=xfind::b;

    std::string piper4_o(std::filesystem::path&fullpath, std::string_view args);
    rc_out_err piper4_roe(std::filesystem::path&fullpath, std::string_view args);
    rc_Out_Err piper4_ROE(std::filesystem::path&fullpath, std::string_view args);
    int piper4_linewise(std::filesystem::path&fullpath, std::string_view args,
        std::function<void(const std::string&)>process_stdout, std::function<void(const std::string&)>process_stderr);

    std::optional<std::filesystem::path> applpath(xfind, std::string_view exec_name);

    inline std::string exec_sync1(std::filesystem::path&exec, std::string_view args)
    {
        return piper4_o(exec, args);
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
        return piper4_o(exec, args);
    }
    template<> inline rc_out_err exec_simple<rc_out_err>(std::filesystem::path&exec, std::string_view args)
    {
        return piper4_roe(exec, args);
    }
    template<> inline rc_Out_Err exec_simple<rc_Out_Err>(std::filesystem::path&exec, std::string_view args)
    {
        return piper4_ROE(exec, args);
    }
}
