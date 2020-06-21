import numpy as np
import sys

from dime import DimeClient

if __name__ != "__main__":
    raise RuntimeError()

d1 = DimeClient("ipc", sys.argv[1])
d2 = DimeClient("ipc", sys.argv[1])
d3 = DimeClient("ipc", sys.argv[1])

d1.join("d1")
d2.join("d2")
d3.join("d3")

d1["a"] = np.random.rand(500, 500)
d1["b"] = np.random.rand(500, 500)
d1["c"] = np.random.rand(500, 500)

d1.send("d2", "a", "b", "c")

d2.sync()

assert np.array_equal(d1["a"], d2["a"])
assert np.array_equal(d1["b"], d2["b"])
assert np.array_equal(d1["c"], d2["c"])

d3["a"] = None
d3["b"] = None
d3["c"] = None

d3.sync()

assert not np.array_equal(d1["a"], d3["a"])
assert not np.array_equal(d1["b"], d3["b"])
assert not np.array_equal(d1["c"], d3["c"])
