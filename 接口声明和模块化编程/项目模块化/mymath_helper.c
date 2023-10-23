#include <Python.h>
#include "mymath/mymath.h"
#include "stdio.h"

int main() {
    Py_Initialize();
    double in[4] = {1.1,2.2,3.3,4.4}
    double res = normalize_and_l2norm(in)
    printf("get result %f",res)
    Py_Finalize();
}