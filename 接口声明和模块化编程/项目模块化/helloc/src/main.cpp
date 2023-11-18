#include <stdio.h>
#include <string>
// #include <Python.h>
#include "helloc.h"

const char* version = "0.1.0";


int callmodelhello(char* name) {
    /** Add a built-in module, before Py_Initialize
     *  Py_Initialize执行前 前增加一个默认导入的模块
    */
    PyObject* helloc_module;
    if (PyImport_AppendInittab("helloc", PyInit_helloc) == -1) {
        fprintf(stderr, "Error: could not extend in-built modules table\n");
        exit(1);
        }
    Py_Initialize();
    PySys_SetPath(L".");
    helloc_module = PyImport_ImportModule("helloc");
    if (!helloc_module) {
        PyErr_Print();
        fprintf(stderr, "Error: could not import module 'helloc'\n");
        goto exit_with_error;
        }
    hello(name);
    /* Clean up after using CPython. */
    Py_Finalize();
    return 0;
    /**Clean up in the error cases above.*/
exit_with_error:
    Py_Finalize();
    return 1;
    }

int main(int argc, char** argv) {

    int ch;
    int result;
    opterr = 0;
    while ((ch = getopt(argc, argv, "s:v\n")) != -1) {
        switch (ch) {
            case 'v':
                printf("option v: %s\n", version);
                break;
            case 's':
                result = callmodelhello(optarg);
                if (result != 0)
                    printf("callmodelhello get error");
                break;
            default:
                printf("wrong args\n");
            }
        }
    }

