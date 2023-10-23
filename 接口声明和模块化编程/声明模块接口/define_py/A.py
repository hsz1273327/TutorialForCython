# cython: embedsignature=True
# cython: embedsignature.format=python
import cython

if cython.compiled:
    print("Yep, I'm compiled.")

else:
    print("Just a lowly interpreted script.")
    uint_t = int

    def int_min(a: int, b: int) -> int:
        return b if b < a else a

GLOBAL_VAL = 100

# uint_t = cython.typedef(cython.uint)


def mycfunc(x: uint_t, y: uint_t = 2) -> uint_t:
    a: uint_t = x - y
    return a + x * y


def mycpfunc(x: cython.int, y: cython.int = 2) -> int:
    a: cython.int = x + y
    return a + x * y


@cython.locals(x=cython.int, y=cython.int)
def mypfuncstatic(x: int, y: int) -> int:
    return x * y


def mypfunc(x: int, y: int) -> int:
    return 2 * x * y


def _helper(a: cython.double) -> float:
    return a + 1


class A:
    def __init__(self, b: int = 0):
        self.a = 3
        self.b = b

    def foo(self, x: cython.double) -> cython.double:
        return x + _helper(1.0)

    def bar(self, x: float) -> float:
        return x**2 + _helper(1.0)

    def foobar(self, x: float) -> float:
        return x**2 + _helper(1.0)
