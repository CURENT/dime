#!/usr/bin/env python3

import sys
import os
sys.path.append(os.path.join(os.getcwd(), "..", "client", "python"))

import multiprocessing as mp
import random
import signal
import string
import statistics
import subprocess
import tempfile
import time

from dime import DimeClient
import numpy as np
try:
    from tqdm import trange
except ModuleNotFoundError:
    trange = range

TEST_AVG_RUNS = 10
TEST_RUN_TIME = 45
TEST_NUM_PROCESSES = 8
TEST_SHARED_GROUPS = ["foo", "bar", "baz"]

class AlarmTimeout(Exception):
    pass

def timeout(signum, frame):
    raise AlarmTimeout()

def randstring(n):
    return "".join(random.choices(string.ascii_lowercase, k = n))

def stresstest(socketfile, counter, counter_mut, barrier):
    with DimeClient("ipc", socketfile) as client:
        client.join(randstring(20), random.choice(TEST_SHARED_GROUPS))

        barrier.wait()

        devices = client.devices()

        while True:
            sent = {}

            for _ in range(3):
                sent[randstring(20)] = np.random.randn(random.randint(2, 500), random.randint(2, 500))

            client.send_r(random.choice(devices), **sent)
            received = client.sync_r()

            ct = 0

            for _, v in received.items():
                ct += v.shape[0] * v.shape[1]

            with counter_mut:
                counter.value += ct

if __name__ == "__main__":
    counters = []
    signal.signal(signal.SIGALRM, timeout)

    for _ in trange(TEST_AVG_RUNS):
        socketfile = os.path.join(tempfile.gettempdir(), randstring(20))
        dimeserver = subprocess.Popen(["../server/dime", "-f", socketfile])
        time.sleep(0.1)

        counter = mp.Value("Q", 0, lock = False)
        counter_mut = mp.Lock()
        barrier = mp.Barrier(TEST_NUM_PROCESSES)

        procs = [mp.Process(target = stresstest, args = (socketfile, counter, counter_mut, barrier)) for _ in range(TEST_NUM_PROCESSES - 1)]
        for proc in procs:
            proc.start()

        signal.alarm(TEST_RUN_TIME)

        try:
            stresstest(socketfile, counter, counter_mut, barrier)
        except AlarmTimeout:
            pass

        for proc in procs:
            proc.terminate()
        dimeserver.terminate()

        counters.append(counter.value / TEST_RUN_TIME)

    print("Average of {} doubles/sec".format(statistics.mean(counters)))
