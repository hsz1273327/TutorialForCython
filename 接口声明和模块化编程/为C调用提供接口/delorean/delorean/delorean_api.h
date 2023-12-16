/* Generated by Cython 3.0.6 */

#ifndef __PYX_HAVE_API__delorean
#define __PYX_HAVE_API__delorean
#ifdef __MINGW64__
#define MS_WIN64
#endif
#include "Python.h"
#include "delorean.h"

static int (*__pyx_api_f_8delorean_activate)(struct Vehicle *) = 0;
#define activate __pyx_api_f_8delorean_activate
#ifndef __PYX_HAVE_RT_ImportFunction_3_0_6
#define __PYX_HAVE_RT_ImportFunction_3_0_6
static int __Pyx_ImportFunction_3_0_6(PyObject *module, const char *funcname, void (**f)(void), const char *sig) {
    PyObject *d = 0;
    PyObject *cobj = 0;
    union {
        void (*fp)(void);
        void *p;
    } tmp;
    d = PyObject_GetAttrString(module, (char *)"__pyx_capi__");
    if (!d)
        goto bad;
    cobj = PyDict_GetItemString(d, funcname);
    if (!cobj) {
        PyErr_Format(PyExc_ImportError,
            "%.200s does not export expected C function %.200s",
                PyModule_GetName(module), funcname);
        goto bad;
    }
    if (!PyCapsule_IsValid(cobj, sig)) {
        PyErr_Format(PyExc_TypeError,
            "C function %.200s.%.200s has wrong signature (expected %.500s, got %.500s)",
             PyModule_GetName(module), funcname, sig, PyCapsule_GetName(cobj));
        goto bad;
    }
    tmp.p = PyCapsule_GetPointer(cobj, sig);
    *f = tmp.fp;
    if (!(*f))
        goto bad;
    Py_DECREF(d);
    return 0;
bad:
    Py_XDECREF(d);
    return -1;
}
#endif


static int import_delorean(void) {
  PyObject *module = 0;
  module = PyImport_ImportModule("delorean");
  if (!module) goto bad;
  if (__Pyx_ImportFunction_3_0_6(module, "activate", (void (**)(void))&__pyx_api_f_8delorean_activate, "int (struct Vehicle *)") < 0) goto bad;
  Py_DECREF(module); module = 0;
  return 0;
  bad:
  Py_XDECREF(module);
  return -1;
}

#endif /* !__PYX_HAVE_API__delorean */
