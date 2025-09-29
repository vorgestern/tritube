
#include <iostream>
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

// ============================================================================

template<> optional<fspath> tritube::applpath<xfliteral>(string_view versuch)
{
    if (filesystem::exists(versuch)) return versuch;
    else return {};
}
template<> optional<fspath> tritube::applpath<xfrelative>(string_view simple_name)
{
    error_code ec;
    auto here=filesystem::current_path(ec);
    if (ec) return {};
    auto versuch=here / simple_name;
    if (filesystem::exists(versuch)) return versuch;
    else return {};
}
template<> optional<fspath> tritube::applpath<xfpath>(string_view relpath)
{
    auto Paths=pathdirectories();
    for (auto&k: Paths)
    {
        const auto versuch=k / relpath;
        if (filesystem::exists(versuch)) return versuch;
    }
    return {};
}
