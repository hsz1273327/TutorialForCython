# distutils: language = c++
# distutils: extra_compile_args=-fopenmp
# distutils: extra_link_args=-fopenmp

from .inner.l2norm cimport l2norm
from .normalize cimport _normalize

cdef double normalize_and_l2norm(double[:] x):
    return l2norm(_normalize(x))