#!/lusr/bin/bash

# prepare to run
SCRIPT=$(readlink -f $0)
SCRIPT_DIR=`dirname $SCRIPT`
RUN=$1
cd $SCRIPT_DIR

set

# start child process
./OpenNERO --mod=Maze --mod-path=Maze:common --command="StartMe()" --no-window > /dev/null &
OPENNERO_PID=$!
echo STARTED OpenNERO with PID $OPENNERO_PID

# sleep for some time
sleep 30s

# kill child process
kill -HUP $OPENNERO_PID
echo KILLED OpenNERO PID $OPENNERO_PID
