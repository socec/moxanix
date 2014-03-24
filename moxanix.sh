#!/bin/bash

# Main script for controlling servers.


# Call: usage
# Shows help.
usage ()
{
	echo "Usage: ./$(basename $0) <command> <id>..."
	echo "	<command>"
	echo "		start	- starts server identified by <id>"
	echo "		stop	- stops server identified by <id>"
	echo "		status	- displays status for server identified by <id>"
	echo "	<id>"
	echo "		number 1-7 for specific server, 0 for all servers"
	echo
}

# Call: start ID
# Starts specific server with output redirected to a logfile.
start ()
{
	# grab parameters
	ID=$1
	
	# set variables
	TCP_PORT=$((4000 + $ID))
	TTY_PATH="/dev/ttyS$ID"
	
	# check if requested server is already up
	pid=$(ps axf | grep "[m]oxerver -p $TCP_PORT" | awk '{print $1}')
	if [ "$pid" != "" ]; then
		echo "server $ID is already up"
		return 0
	fi
	
	SERVER_RUN="./moxerver"
	
	LOGDIR="./logs"
	LOGFILE="$LOGDIR/moxerver$ID.log"

	# create log directory if it doesn't exist
	if [ ! -d $LOGDIR ]; then
		mkdir -p $LOGDIR
	fi
	
	# start server, redirect stdout and stderr to logfile
	# nohup keeps it running when the script ends
	nohup $SERVER_RUN -p $TCP_PORT -t $TTY_PATH > $LOGFILE 2>&1 &
	echo "server $ID started"
}

# Call: stop ID
# Stops specific server.
stop ()
{
	# grab parameters
	ID=$1
	
	# set variables
	TCP_PORT=$((4000 + $ID))
	
	# check if requested server is already down
	pid=$(ps axf | grep "[m]oxerver -p $TCP_PORT" | awk '{print $1}')
	if [ "$pid" = "" ]; then
		echo "server $ID is already down"
		return 0
	fi
	
	# kill requested server
	kill -s SIGTERM $pid
	echo "server $ID stopped"
}

# Call: status ID
# Displays if specific server is running or not.
status ()
{
	# grab parameters
	ID=$1
	
	# set variables
	TCP_PORT=$((4000 + $ID))
	
	# check if requested server is up or down
	pid=$(ps axf | grep "[m]oxerver -p $TCP_PORT" | awk '{print $1}')
	if [ "$pid" = "" ]; then
		echo "server $ID is down"
	else
		echo "server $ID is up"
	fi
}

##########

# check parameter count
if [ $# -lt 1 ]; then
	usage
	exit
fi

# grab parameter - command
COMMAND=$1

if [ "$COMMAND" = "start" ]; then
	if [ $# -ne 2 ]; then
		usage
		exit
	fi
	# grab parameter - id
	ID=$2
	# if ID is 1-7 run command for specific server
	if [ $ID -ge 1 ]  && [ $ID -le 7 ]; then
		start $ID
	# if ID is 0 run command for all servers
	elif [ $ID -eq 0 ]; then
		for NUM in {1..7}
		do
		start $NUM
		done
	# wrong ID value
	else
		usage
		exit
	fi
	
elif [ "$COMMAND" = "stop" ]; then
	if [ $# -ne 2 ]; then
		usage
		exit
	fi
	# grab parameter - id
	ID=$2
	# if ID is 1-7 run command for specific server
	if [ $ID -ge 1 ]  && [ $ID -le 7 ]; then
		stop $ID
	# if ID is 0 run command for all servers
	elif [ $ID -eq 0 ]; then
		for NUM in {1..7}
		do
		stop $NUM
		done
	# wrong ID value
	else
		usage
		exit
	fi
	
elif [ "$COMMAND" = "status" ]; then
	if [ $# -ne 2 ]; then
		usage
		exit
	fi
	# grab parameter - id
	ID=$2
	# if ID is 1-7 run command for specific server
	if [ $ID -ge 1 ]  && [ $ID -le 7 ]; then
		status $ID
	# if ID is 0 run command for all servers
	elif [ $ID -eq 0 ]; then
		for NUM in {1..7}
		do
		status $NUM
		done
	# wrong ID value
	else
		usage
		exit
	fi
	
else
	# unsupported command
	usage
fi
