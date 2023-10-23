# cython: embedsignature=True
# cython: embedsignature.format=python

GLOBAL_VAL = 100

cdef uint_t mycfunc(uint_t x, uint_t y=2):
    cdef uint_t a
    a = x-y
    return a + x * y

cpdef int mycpfunc(int x, int y=2):
    cdef int a
    a = x+y
    return a + x * y

def mypfuncstatic(int x, int y):
    return x*y

def mypfunc(x:int, y: int)->int:
    return 2*x*y


cdef double _helper(double a):
    return a + 1



cdef class A:
    def __init__(self, b:int = 0):
        self.a = 3
        self.b = b

    cdef double foo(self, double x):
        return x + _helper(1.0)
    
    cpdef double bar(self, double x):
        return x**2 + _helper(1.0)

    def foobar(self,x:float)->float:
        return x**2 + _helper(1.0)