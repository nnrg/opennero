#!/bin/bash

# prepare to run
cd $(dirname $(readlink -f $0))

# start child process
./OpenNERO \
  --log $4 \
  --mod NERO_Battle \
  --modpath NERO_Battle:_NERO:common \
  --headless \
  --command "Match('$1', '$2')" &
OPENNERO_PID=$!
echo STARTED OpenNERO with PID $OPENNERO_PID

# sleep for some time
sleep $3

# kill child process
kill -HUP $OPENNERO_PID
echo KILLED OpenNERO PID $OPENNERO_PID
