# distutils: language = c++
from libcpp.string cimport string, to_string
from .universal.funcs cimport get_nowtime

cdef void hello(string name) except *:
    cdef string out = "hello "+ name + "now timestamp is " + to_string(get_nowtime())
    print(out)