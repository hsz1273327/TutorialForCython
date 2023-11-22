cdef extern from "Rectangle.h" namespace "shapes":
    cdef cppclass Rectangle:
        int x0
        int y0
        int x1
        int y1
        Rectangle() except +
        Rectangle(int, int, int, int) except +
        int getArea()
        void getSize(int* width, int* height)
        void move(int, int)

        
cdef class PyRectangle:
    cdef Rectangle * c_rect