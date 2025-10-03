
#include <utility>

class forkpipes
{
    static const int PIPE_READ=0, PIPE_WRITE=1;
    int pipe_in[2], pipe_out[2];
    int childpid {0};
    friend bool isparent(const forkpipes&X){ return X.childpid>0; }
    friend bool ischild(const forkpipes&X){ return X.childpid==0; }
    friend int parent_write(const forkpipes&X){ return X.pipe_in[PIPE_WRITE]; }
    friend int parent_read(const forkpipes&X){ return X.pipe_out[PIPE_READ]; }
public:
    forkpipes();
   ~forkpipes();
};

class forkpipes3
{
    static const int PIPE_READ=0, PIPE_WRITE=1;
    int pipe_in[2], pipe_out[2], pipe_err[2];
    int childpid {0};
    int closed {0};
    friend int childpid(const forkpipes3&X){ return X.childpid; }
    friend bool isparent(const forkpipes3&X){ return X.childpid>0; }
    friend bool ischild(const forkpipes3&X){ return X.childpid==0; }
    friend int parent_write(const forkpipes3&X){ return X.pipe_in[PIPE_WRITE]; }
    friend int parent_read(const forkpipes3&X){ return X.pipe_out[PIPE_READ]; }
    friend int parent_readerr(const forkpipes3&X){ return X.pipe_err[PIPE_READ]; }
    friend bool isclosed(const forkpipes3&X){ return X.closed==3; }
public:
    forkpipes3();
   ~forkpipes3();
    std::pair<char,int>readchar();
};
