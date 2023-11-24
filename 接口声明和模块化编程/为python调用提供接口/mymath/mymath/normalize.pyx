# distutils: extra_compile_args=-fopenmp
# distutils: extra_link_args=-fopenmp

from cython.parallel cimport parallel, prange
cimport cython
from libc.math cimport sqrt
import numpy as np


@cython.boundscheck(False)
@cython.wraparound(False)
cdef double[:] _normalize(double[:] x):
    cdef Py_ssize_t i
    cdef double total = 0
    cdef double norm
    cdef double[:] output = np.empty_like(x)
    with nogil, parallel():
        for i in prange(x.shape[0]):
            total += x[i]*x[i]
        norm = sqrt(total)
        for i in prange(x.shape[0]):
            output[i] = x[i]/norm
    return output


def normalize(double[:] x):
    cdef double[:] output = _normalize(x)
    return np.asarray(output)