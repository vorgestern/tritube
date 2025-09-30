
struct prochelper
{
    HANDLE process;
    HANDLE write_to_stdin;
    HANDLE     read_from_stdout, read_from_stderr;
    OVERLAPPED olout,            olerr;
};

int startpiped(prochelper&ph, const std::string&exec, std::string_view arguments);

struct inputchannel_async
{
    HANDLE read_from_child;
    OVERLAPPED ol;
    bool closed {false};
    char buffer[1024];
    DWORD nr {0};
    void read()
    {
        nr=0;
        if (!ReadFile(read_from_child, buffer, sizeof buffer, &nr, &ol))
        {
            switch (GetLastError())
            {
                case ERROR_IO_PENDING: break;
                default:
                case ERROR_BROKEN_PIPE: closed=true; break;
            }
        }
    }
    void closehandles()
    {
        CloseHandle(ol.hEvent);
        CloseHandle(read_from_child);
        closed=true;
    }
};

int entertain(HANDLE hprocess, inputchannel_async&out, std::function<void()>handle_out, inputchannel_async&err, std::function<void()>handle_err);
