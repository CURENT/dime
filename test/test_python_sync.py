import numpy as np
import sys

from dime import DimeClient

if __name__ != "__main__":
    raise RuntimeError()

d1 = DimeClient("ipc", sys.argv[1])
d2 = DimeClient("ipc", sys.argv[1])

d1.join("d1")
d2.join("d2")

d1["a"] = np.random.rand(500, 500)
d1["b"] = np.random.rand(500, 500)
d1["c"] = np.random.rand(500, 500)
d1["d"] = np.random.rand(500, 500)
d1["e"] = np.random.rand(500, 500)

d2["a"] = None
d2["b"] = None
d2["c"] = None
d2["d"] = None
d2["e"] = None

d1.send("d2", "a", "b", "c")

d2.sync(2)

assert np.array_equal(d1["a"], d2["a"])
assert np.array_equal(d1["b"], d2["b"])
assert not np.array_equal(d1["c"], d2["c"])
assert not np.array_equal(d1["d"], d2["d"])
assert not np.array_equal(d1["e"], d2["e"])

d1.send("d2", "d", "e")
d2.sync()

assert np.array_equal(d1["a"], d2["a"])
assert np.array_equal(d1["b"], d2["b"])
assert np.array_equal(d1["c"], d2["c"])
assert np.array_equal(d1["d"], d2["d"])
assert np.array_equal(d1["e"], d2["e"])
