# distutils: language = c++
# distutils: extra_compile_args=-fopenmp
# distutils: extra_link_args=-fopenmp

from cython.parallel cimport parallel, prange
from libcpp.vector cimport vector
from libcpp.algorithm cimport nth_element
cimport cython
from cython.operator cimport dereference

import numpy as np

@cython.boundscheck(False)
@cython.wraparound(False)
cdef double[::1]  _median_along_axis0(const double[:,:] x):
    cdef double[::1] out = np.empty(x.shape[1])
    cdef Py_ssize_t i, j

    cdef vector[double] *scratch # 缓存的vector
    cdef vector[double].iterator median_it # 缓存的vector中中间位置迭代器
    with nogil, parallel():
        scratch = new vector[double](x.shape[0]) # 在线程中构造缓存
        try:
            for i in prange(x.shape[1]): # 按列拆分任务
                # 将当列的每一行元素复制到缓存的vector
                for j in range(x.shape[0]):
                    dereference(scratch)[j] = x[j, i]
                median_it = scratch.begin() + scratch.size()//2 # 计算中间位置的index
                nth_element(scratch.begin(), median_it, scratch.end()) #使用`std::nth_element`对中间位置进行部分排序
                # for the sake of a simple example, don't handle even lengths...
                out[i] = dereference(median_it)# 从迭代器中取出中位数
        finally:
            del scratch # 销毁缓存
    return out

def median_along_axis0(const double[:,:] x):
    cdef double[::1] out = _median_along_axis0(x)
    return np.asarray(out)