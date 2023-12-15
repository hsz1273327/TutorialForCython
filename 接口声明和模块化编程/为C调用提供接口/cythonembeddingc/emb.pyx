# distutils: language = c++
cdef int numargs = 0

cdef int get_numargsc():
    # global numargs
    return numargs


def get_numargs():
    # global numargs
    return numargs

cdef void set_numargsc(int i):
    global numargs
    numargs = i

def set_numargs(int i):
    cdef int _i = i
    set_numargsc(_i)