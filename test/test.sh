#!/bin/bash

../server/dime &
PIDSRV=$!

matlab -batch "run('test0.m')" &
PID0=$!
sleep 0.5

matlab -batch "run('test1.m')"  &
PID1=$!

matlab -batch "run('test2.m')" &
PID2=$!

wait $PID0
kill $PIDSRV $PID1 $PID2
