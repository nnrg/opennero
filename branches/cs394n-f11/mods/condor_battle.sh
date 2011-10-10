#!/bin/bash

# prepare to run
SCRIPT=$(readlink -f $0)
SCRIPT_DIR=`dirname $SCRIPT`
RUN=$1
cd $SCRIPT_DIR

# start child process
./OpenNERO --mod NERO_Battle --modpath NERO_Battle:_NERO:common --headless --command "Match('$1', '$2')" &
OPENNERO_PID=$!
echo STARTED OpenNERO with PID $OPENNERO_PID

# sleep for some time
sleep $3

# kill child process
kill -HUP $OPENNERO_PID
echo KILLED OpenNERO PID $OPENNERO_PID
