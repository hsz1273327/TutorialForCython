cdef extern from "Rectangle.hpp" namespace "shapes":
    cdef cppclass Rectangle[T]:
        T x0
        T y0
        T x1
        T y1
        Rectangle() except +
        Rectangle(T, T, T, T) except +
        T getArea()
        void getSize(T* width, T* height)
        void move(T, T)

        
cdef class PyRectangleFloat:
    cdef Rectangle[float] * c_rect