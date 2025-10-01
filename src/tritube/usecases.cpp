
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

optional<fspath> tritube::applpath(xfind f, string_view appp)
{
    fspath appl(appp);
    switch (f)
    {
        case xfdirect:
        {
            if (appl.is_absolute()) return filesystem::exists(appl)?appl:optional<fspath>();
            error_code ec;
            auto here=filesystem::current_path(ec);
            if (ec) return {};
            auto versuch=here / appl;
            return filesystem::exists(versuch)?versuch:optional<fspath>();
        }
        case xfpath:
        {
            auto Paths=pathdirectories();
            for (auto&k: Paths)
            {
                const auto versuch=k / appl;
                if (filesystem::exists(versuch)) return versuch;
            }
            return {};
        }
        default: return {};
    }
}
