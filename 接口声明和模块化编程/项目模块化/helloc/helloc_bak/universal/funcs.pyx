# distutils: language = c++
import time

cdef float get_nowtime():
    cdef float now = time.time()
    return now