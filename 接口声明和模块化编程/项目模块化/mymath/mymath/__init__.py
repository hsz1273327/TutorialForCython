import cython

from .median_along_axis0 import median_along_axis0
from .normalize import normalize
from .inner.l2norm import l2norm

if cython.compiled:
    print("Yep, mymath.__init__ compiled.")

else:
    print("mymath.__init__ Just a lowly interpreted script.")
