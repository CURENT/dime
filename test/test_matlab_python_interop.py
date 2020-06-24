import math
import numpy as np
import sys
import time

from dime import DimeClient

d = DimeClient("ipc", sys.argv[1]);
d.join("python");

while "matlab" not in d.devices():
    time.sleep(0.05)

time.sleep(1)

d.sync()

assert d["nothing"] is None
assert d["boolean"] is True
assert d["float"] == math.pi
assert d["int"] == 0xDEADBEEF
assert d["complexfloat"] == complex(math.sqrt(0.75), 0.5)
assert np.array_equal(d["matrix"], np.array([[1, 2, 3], [4, 5, 6], [7, 8, 9.01]]))
assert d["string"] == "Hello world!"
assert d["sequence"] == [-1, ":)", False, None]
assert d["mapping"] == {"foo": 2, "bar": "Green eggs and spam"}

d.send("matlab", "nothing", "boolean", "int", "float", "complexfloat", "matrix", "string", "sequence", "mapping");
