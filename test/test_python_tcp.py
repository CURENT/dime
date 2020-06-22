import numpy as np
import sys

from dime import DimeClient

if __name__ != "__main__":
    raise RuntimeError()

d1 = DimeClient("tcp", sys.argv[1], int(sys.argv[2]))
d2 = DimeClient("tcp", sys.argv[1], int(sys.argv[2]))

d1.join("d1")
d2.join("d2")

d1["a"] = np.random.rand(500, 500)

d1.send("d2", "a")

d2.sync()

assert np.array_equal(d1["a"], d2["a"])
