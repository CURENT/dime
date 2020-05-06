#!/bin/bash

matlab -batch "run('test1.m')" &
PID1=$!

matlab -batch "run('test2.m')" &
PID2=$!

matlab -batch "run('test0.m')"

kill $PID1 $PID2
kill $PID1
