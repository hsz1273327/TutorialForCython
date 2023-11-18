# distutils: language = c++

from libcpp.string cimport string, to_string
import time

cdef float get_nowtime():
    cdef float now = time.time()
    return now

cdef void hello(char* name) except *:
    cdef string ts = to_string(get_nowtime())
    cdef string out = b"hello "
    out.append(name)
    out.append(b" now timestamp is ")
    out.append(ts)
    print(out.decode("utf-8"))