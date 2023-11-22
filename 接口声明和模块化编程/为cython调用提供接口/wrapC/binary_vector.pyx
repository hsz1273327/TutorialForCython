# distutils: extra_compile_args=-Wno-unreachable-code
# distutils: library_dirs=vec/lib
# distutils: libraries=vector
# distutils: include_dirs=vec/inc

cdef class Vector:
    @staticmethod
    cdef create(BINARY_VECTOR_P ptr):
        p = Vector()
        p.data = ptr
        return p

    @staticmethod
    def new(float x, float y):
        p = Vector()
        p.data = VEC_init(x, y)
        return p

    def init(self,float x, float y):
        self.data= VEC_init(x, y)
        
    cdef void init_from_point(self,BINARY_VECTOR_P ptr):
        self.data = ptr

    def __dealloc__(self):
        if self.data is not NULL:
            print(f"A dealloc")
            VEC_del(self.data)

    def mod(self)->float:
        if self.data is not NULL:
            return VEC_mod(self.data)
        raise Exception("vector not init")

    def __add__(self,other: Vector)->Vector:
        
        return Vector.create(VEC_add(self.data, other.data))

    def __mul__(self,other: Vector)->float:
        return VEC_mul(self.data, other.data)
