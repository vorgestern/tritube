
This is a helper to establish piped communication via all three stdio
channels (stdin, stdout, stderr) at the same time. It encapsulates the
fork call and the select call to handle stdin, stdout and stderr:

    forkpipes3 PP;
    if (ischild(PP))
    {
        const int argc=...;
        char*argv[argc];
        argv[0]=...;
        exit(execvp(argv[0], argv));
    }
    else if (isparent(PP))
    {
        write(parent_write(PP), A.data(), A.size());
        close(parent_write(PP));
        while (!isclosed(PP))
        {
            const auto [c,rc]=PP.readchar();
            switch (rc)
            {
                case 1: .. break; // input from stdin
                case 2: .. break; // input from stderr
            }
        }
    }

Examples:

- **compile1**: Call gcc to compile C++ source code.
  Supply source code via stdin. Process regular output and error messages.

# How to build (Linux only)

    make
