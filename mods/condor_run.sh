#!/lusr/bin/bash

# prepare to run
SCRIPT=$(readlink -f $0)
SCRIPT_DIR=`dirname $SCRIPT`
RUN=$1
cd $SCRIPT_DIR

# start child process
export RUN=${RUN}
./OpenNERO --log opennero_condor_${RUN}.log --mod Blocksworld --modpath Blocksworld:common --headless
OPENNERO_PID=$!
echo STARTED OpenNERO with PID $OPENNERO_PID
