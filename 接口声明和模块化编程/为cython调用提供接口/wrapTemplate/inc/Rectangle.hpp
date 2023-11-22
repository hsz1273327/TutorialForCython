#ifndef RECTANGLE_H
#define RECTANGLE_H

namespace shapes {
    template <class T>
    class Rectangle {
        public:
        T x0, y0, x1, y1;
        Rectangle() {};
        Rectangle(T x0, T y0, T x1, T y1) {
            this->x0 = x0;
            this->y0 = y0;
            this->x1 = x1;
            this->y1 = y1;
            };
        ~Rectangle() {};
        T getArea() {
            return (this->x1 - this->x0) * (this->y1 - this->y0);
            };
        void getSize(T* width, T* height) {
            (*width) = x1 - x0;
            (*height) = y1 - y0;
            };
        void move(T dx, T dy) {
            this->x0 += dx;
            this->y0 += dy;
            this->x1 += dx;
            this->y1 += dy;
            };
        };
    }
#endif
