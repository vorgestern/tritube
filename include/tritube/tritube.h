
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

    std::string piper_o(std::filesystem::path&fullpath, std::string_view args);
    rc_out_err piper_roe(std::filesystem::path&fullpath, std::string_view args);
    rc_Out_Err piper_roev(std::filesystem::path&fullpath, std::string_view args);
    int piper_linewise(std::filesystem::path&fullpath, std::string_view args,
        std::function<void(const std::string&)>process_stdout,
        std::function<void(const std::string&)>process_stderr);

    std::optional<std::filesystem::path> applpath(xfind, std::string_view exec_name);
}
