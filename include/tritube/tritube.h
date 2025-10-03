
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

    // Helper: Return the complete path of an existing executable.
    // If searchtype is xfpath, resolve exec with the PATH environment variable.
    // If searchtype is xfdirect, interpret exec relative to the current directory.
    // On Windows, if exec lacks an extension, check with each possible extension
    // from the PATHEXT environment variable.
    std::optional<std::filesystem::path> applpath(xfind searchtype, std::string_view exec);

    // Return output on stdout as a string (if exitcode==0), else an empty string. 
    std::string piper_o(std::filesystem::path&fullpath, const std::vector<std::string>&args);

    // Return [exitcode, output on stdout, output on stderr].
    // The outputs are represented as strings.
    rc_out_err piper_roe(std::filesystem::path&fullpath, const std::vector<std::string>&args);

    // Return [exitcode, output on stdout, output on stderr].
    // The outputs are represented as vector<string> with newlines removed.
    rc_Out_Err piper_roev(std::filesystem::path&fullpath, const std::vector<std::string>&args);

    // Return the exitcode only, pass output lines on stdout or stderr to corresponding handlers.
    // Handlers may be empty.
    int piper_linewise(std::filesystem::path&fullpath, const std::vector<std::string>&args,
        std::function<void(const std::string&)>process_stdout,
        std::function<void(const std::string&)>process_stderr);
}
