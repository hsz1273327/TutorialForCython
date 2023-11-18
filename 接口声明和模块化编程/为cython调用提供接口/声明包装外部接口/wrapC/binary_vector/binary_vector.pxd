cdef extern from "binary_vector.h":
    cdef struct BINARY_VECTOR:
        float x
        float y

    ctypedef BINARY_VECTOR* BINARY_VECTOR_P

    cdef BINARY_VECTOR_P VEC_new()
    cdef BINARY_VECTOR_P VEC_init(float x,float y)
    cdef void VEC_del(BINARY_VECTOR_P v)

    cdef float VEC_mod(BINARY_VECTOR_P v)
    cdef BINARY_VECTOR_P VEC_add(BINARY_VECTOR_P x,BINARY_VECTOR_P y)
    cdef float VEC_mul(BINARY_VECTOR_P x,BINARY_VECTOR_P y)
