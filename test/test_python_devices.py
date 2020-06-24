import numpy as np
import sys

from dime import DimeClient

if __name__ != "__main__":
    raise RuntimeError()

d1 = DimeClient("ipc", sys.argv[1])
d2 = DimeClient("ipc", sys.argv[1])
d3 = DimeClient("ipc", sys.argv[1])
d4 = DimeClient("ipc", sys.argv[1])

d1.join("a", "b", "c", "d", "e")
d2.join("a", "b", "c")
d3.join("e", "f")
d4.join("g")

devices = d1.devices()
devices.sort()

assert devices == ["a", "b", "c", "d", "e", "f", "g"]

d1.leave("a", "b", "c", "d")
d2.leave("a", "b")
d3.leave("e", "f")

d1.join("b")
d3.join("h")
d4.join("f")

devices = d1.devices()
devices.sort()

assert devices == ["b", "c", "e", "f", "g", "h"]

d1.leave("b", "e")
d2.leave("c")
d3.leave("h")
d4.leave("f", "g")

devices = d1.devices()

assert devices == []
