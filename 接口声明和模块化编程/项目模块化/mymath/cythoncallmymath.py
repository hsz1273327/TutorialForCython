# distutils: extra_compile_args=-fopenmp
# distutils: extra_link_args=-fopenmp
# distutils: extra_compile_args=-I/opt/homebrew/Cellar/gcc/13.2.0/lib/gcc/current/gcc/aarch64-apple-darwin22/13/include
import cython
from cython.cimports.mymath.normalize import _normalize
import numpy as np

if cython.compiled:
    print("Yep, callmymath compiled.")
else:
    print("callmymath Just a lowly interpreted script.")

def callmymath():
    output: cython.double[:] = _normalize(np.array([1.1, 2.2, 3.3, 4.4]))
    print(np.asarray(output))
