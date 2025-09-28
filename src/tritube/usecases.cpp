
#include <iostream>
#include <filesystem>
#include <tritube/tritube.h>

using namespace std;
using namespace tritube;
using fspath=filesystem::path;

static vector<fspath> pathdirectories()
{
    vector<fspath>result;
    const string P=getenv("PATH");
    size_t pos=0;
    while (true)
    {
        auto s=P.find_first_of(';', pos);
        if (s!=P.npos){ result.push_back(P.substr(pos, s-pos)); pos=s+1; }
        else{ result.push_back(P.substr(pos)); break; }
    }
    return result;
}

static optional<fspath> findinpath(string_view relpath)
{
    auto Paths=pathdirectories();
    for (auto&k: Paths)
    {
        const auto versuch=k / relpath;
        if (filesystem::exists(versuch)) return versuch;
    }
    return {};
}

// ============================================================================

template<tritube::xfind X>static fspath applpath(string_view simple_name);

template<>fspath applpath<xfliteral>(string_view versuch)
{
    if (filesystem::exists(versuch)) return versuch;
    else return {};
}
template<>fspath applpath<xfrelative>(string_view simple_name)
{
    error_code ec;
    auto here=filesystem::current_path(ec);
    if (ec) return {};
    auto versuch=here / simple_name;
    if (filesystem::exists(versuch)) return versuch;
    else return {};
}
template<>fspath applpath<xfpath>(string_view simple_name)
{
    auto App=findinpath(simple_name);
    if (App.has_value()) return App.value();
    else return {};
}

// ============================================================================

template<>string tritube::exec_sync<string,xfliteral>(string_view exec, string_view args)
{
    auto app=applpath<xfliteral>(exec);
    auto commandline=string(exec)+" "+string(args);
    return app.string();
}

template<>string tritube::exec_sync<string,xfrelative>(string_view exec, string_view args)
{
    auto app=applpath<xfrelative>(exec);
    auto commandline=string(exec)+" "+string(args);
    return app.string();
}

template<>string tritube::exec_sync<string,xfpath>(string_view exec, string_view args)
{
    auto app=applpath<xfpath>(exec);
    auto commandline=string(exec)+" "+string(args);
    return app.string();
}
