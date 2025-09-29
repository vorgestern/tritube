
struct prochelper
{
    HANDLE process;
    HANDLE write_to_stdin;
    HANDLE     read_from_stdout, read_from_stderr;
    OVERLAPPED olout,            olerr;
};

int startpiped(prochelper&ph, const std::string&exec, std::string_view arguments);
