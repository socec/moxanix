#!/bin/bash

# Main script for controlling servers.


usage ()
{
echo "Usage: ./$(basename $0) COMMAND ID"
echo "	COMMAND: start, stop"
echo "	ID: 1-7"
}

##########

# check parameter count
if [ $# != 2 ]; then
	usage
	exit
fi

# grab parameters
COMMAND=$1
ID=$2

# set variables
TCP_PORT=$((4000 + $ID))
TTY_PATH="/dev/ttyS$ID"

LOGDIR="./logs"
LOGFILE="$LOGDIR/moxerver$ID.log"

# execute commands
if [ "$COMMAND" = "start" ]; then
	# create log directory if it doesn't exist
	if [ ! -d $LOGDIR ]; then
		mkdir -p $LOGDIR
	fi
	# start moxerver, redirect stdout and stderr to logfile
	# nohup keeps it running when the script ends
	nohup ./moxerver -p $TCP_PORT -t $TTY_PATH > $LOGFILE 2>&1 &
	
elif [ "$COMMAND" = "stop" ]; then
	# ps axf 				-> list all processes with PID as first field
	# grep [m]oxerver...	-> [] trick eliminates the actual grep process from results
	# awk '{print $1}' 		-> print the first field from the line, in this case PID
	pid=$(ps axf | grep "[m]oxerver -p $TCP_PORT" | awk '{print $1}')
	if [ $pid = "" ]; then
		echo "nothing to kill"
	fi
	kill -s SIGTERM $pid
	
else
	# unsupported command
	usage
fi
