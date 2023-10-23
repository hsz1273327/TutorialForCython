ctypedef unsigned int uint_t

cdef int GLOBAL_VAL

cdef uint_t mycfunc(uint_t x, uint_t y=*)

cpdef int mycpfunc(int x, int y=*)

cdef inline int int_min(int a, int b):
    return b if b < a else a

cdef double _helper(double a)


cdef class A:
    cdef public int a
    cdef int b
    cdef double foo(self,double x)
    cpdef double bar(self, double x)
