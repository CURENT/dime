import numpy as np
import signal
import sys
import time

from dime import DimeClient

if __name__ != "__main__":
    raise RuntimeError()

def timeout(signum, frame):
    raise RuntimeError("\"wait\" command timed out")

d = DimeClient("ipc", sys.argv[1])

if int(sys.argv[2]) == 0:
    d.join("d1")

    while "d2" not in d.devices():
        time.sleep(0.05)

    d["a"] = "ping"
    d.send("d2", "a")

    n = d.wait()
    assert n == 1 or n == 2
    d.sync(1)

    assert d["a"] == "pong"

    signal.signal(signal.SIGALRM, timeout)
    signal.alarm(10)

    d.wait()

    signal.alarm(0)

else:
    d.join("d2")

    while "d1" not in d.devices():
        time.sleep(0.05)

    n = d.wait()
    assert n == 1
    d.sync()

    assert d["a"] == "ping"

    d["a"] = "pong"
    d["b"] = 0xDEADBEEF
    d.send("d1", "a", "b")
