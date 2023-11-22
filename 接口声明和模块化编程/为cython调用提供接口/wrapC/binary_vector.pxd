cdef extern from "./vec/inc/binary_vector.h":

    cdef struct BINARY_VECTOR:
        float x
        float y

    ctypedef BINARY_VECTOR* BINARY_VECTOR_P

    BINARY_VECTOR_P VEC_new()

    BINARY_VECTOR_P VEC_init(float x, float y)

    void VEC_del(BINARY_VECTOR_P)

    float VEC_mod(BINARY_VECTOR_P)

    BINARY_VECTOR_P VEC_add(BINARY_VECTOR_P, BINARY_VECTOR_P)

    float VEC_mul(BINARY_VECTOR_P, BINARY_VECTOR_P)


cdef class Vector:
    cdef BINARY_VECTOR_P data

    @staticmethod
    cdef create(BINARY_VECTOR_P ptr)

    cdef void init_from_point(self,BINARY_VECTOR_P ptr)
