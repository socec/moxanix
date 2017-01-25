#!/bin/bash

# =====
# setup
# =====

# parameters
CONFIGURATION_FILE="./moxanix.cfg"
SERVER_BINARY="./moxerver"
LOG_DIRECTORY="./logs"


# global variables for configuration
CONF_SIZE=0
CONF_LINES=()
CONF_ARGS=()


# ================
# helper functions
# ================

# do_usage
# Shows instructions.
do_usage ()
{
	echo "Usage:"
	echo "  $(basename $0) <command> <id>"
	echo
	echo "  <command>"
	echo "      config      - displays current server configuration"
	echo "      start <id>  - starts server identified by <id>"
	echo "      stop <id>   - stops server identified by <id>"
	echo "      status <id> - displays status for server identified by <id>"
	echo "      log <id>    - prints the log for server identified by <id>"
	echo "  <id>"
	echo "      0 for all servers or [1..MAX] for a specific server,"
	echo "      where MAX is the number of configured servers"
	echo
}

# do_read_config
# Reads current configuration from a file and populates global variables
do_read_config()
{
	# read lines from the configuration file
	readarray lines < $CONFIGURATION_FILE

	# count the number of configured connections and extract arguments
	# - skip comment lines and empty lines
	# - use current configuration size as array index
	CONF_SIZE=0
	count=0
	while [ "${lines[count]}" != "" ]
	do
		line=${lines[count]}
		# filter lines and arguments according to the configuration format:
		# tcp=<tcp_port> tty=<tty_device> baud=<tty_baudrate>
		line_valid=$(echo $line | grep -E "^tcp=")
		if [ -n "$line_valid" ]; then
			# configuration lines
			CONF_LINES[$CONF_SIZE]=$(echo -n $line)
			# extract configuration arguments
			tcp=$(echo $line | awk '{print $1}' | tr -d "tcp=")
			tty=$(echo $line | awk '{print $2}' | tr -d "tty=")
			baud=$(echo $line | awk '{print $3}' | tr -d "baud=")
			# compose configuration argument lines for passing to the servers
			CONF_ARGS[$CONF_SIZE]="-p $tcp -t $tty -b $baud"
			# increment configuration size (array index)
			CONF_SIZE=$((CONF_SIZE + 1))
		fi
		# increment line counter
		count=$(( $count + 1 ))
	done
}

# run_config
# Shows current configuration based on the configuration file
run_config()
{
	echo "Reading configuration from: $CONFIGURATION_FILE"
	echo ""; echo "Configured servers:"
	# IDs start from 1, array index starts from 0
	for id in $(seq 1 $CONF_SIZE); do
		echo "|$id| ${CONF_LINES[((id - 1))]}"
	done
}

# foo=$(do_print_server_pid $ID)
# Prints server PID based on ID, capture the output in a variable to use as a function (foo=$(bar))
do_print_server_pid()
{
	ID=$1
	# find server PID by searching for the "start command" in the list of open processes
	START_COMMAND="$SERVER_BINARY ${CONF_ARGS[((ID - 1))]}"
	echo $(pgrep -f "$START_COMMAND")
}

# run_start $ID
# Starts a server based on ID, with output redirected to a logfile
run_start()
{
	ID=$1
	# use PID to check if a server is already running
	pid=$(do_print_server_pid $ID)
	if [ "$pid" == "" ]; then
		# IDs start from 1, array index starts from 0
		START_COMMAND="$SERVER_BINARY ${CONF_ARGS[((ID - 1))]}"
		# prepare log file
		LOG_FILE="$LOG_DIRECTORY/server_$ID.log"
		# create log directory if it doesn't exist
		if [ ! -d $LOG_DIRECTORY ]; then
			mkdir -p $LOG_DIRECTORY
		fi
		# start server, redirect stdout and stderr to the log file
		# nohup keeps it running when the script ends
		echo "Starting server $ID"
		nohup $START_COMMAND > $LOG_FILE 2>&1 &
	else
		echo "Server $ID is already up"
	fi
}

# run_stop $ID
# Stops a running server based on ID
run_stop()
{
	ID=$1
	# use PID to send SIGTERM to the server process
	pid=$(do_print_server_pid $ID)
	if [ "$pid" == "" ]; then
		echo "Server $ID is already down"
	else
		echo "Stopping server $ID"
		kill -s SIGTERM $pid
	fi
}

# run_status $ID
# Shows status of a server based on ID
run_status()
{
	ID=$1
	# use PID to check if a server is running
	pid=$(do_print_server_pid $ID)
	if [ "$pid" = "" ]; then
		echo "Server $ID is down"
	else
		echo "Server $ID is up"
	fi
}

# run_log $ID
# Prints the log file for a server based on ID
run_log()
{
	ID=$1
	LOG_FILE="$LOG_DIRECTORY/server_$ID.log"
	echo "Log of server $ID from \"$LOG_FILE\""
	echo "================"
	cat $LOG_FILE
	echo "================"
}

# run_command $COMMAND $ID
# Runs a given command for a single or all servers, based on ID
run_command()
{
	COMMAND=run_$1
	ID=$2
	# ID 0 runs the command for all servers
	if [ $ID -eq 0 ]; then
		# IDs start from 1, array index starts from 0
		for idx in $(seq 1 $CONF_SIZE); do
			$COMMAND $idx
		done
	# ID >= 1 runs the command for a single server
	elif [ $ID -ge 1 ] && [ $ID -le $CONF_SIZE ] ; then
		$COMMAND $ID
	# error if ID is out of range
	else
		echo "ID out of range: $ID"
	fi
}

# ==========
# entrypoint
# ==========

# read configuration
do_read_config

# execute command
COMMAND=$1
ID=$2

if [ "$COMMAND" == "start" ]; then
	if [ $# -ne 2 ]; then
		do_usage
		exit
	else
		run_command start $ID
	fi
elif [ "$COMMAND" == "stop" ]; then
	if [ $# -ne 2 ]; then
		do_usage
		exit
	else
		run_command stop $ID
	fi
elif [ "$COMMAND" == "status" ]; then
	if [ $# -ne 2 ]; then
		do_usage
		exit
	else
		run_command status $ID
	fi
elif [ "$COMMAND" == "log" ]; then
	if [ $# -ne 2 ]; then
		do_usage
		exit
	else
		run_command log $ID
	fi
elif [ "$COMMAND" == "config" ]; then
	if [ $# -ne 1 ]; then
		do_usage
		exit
	else
		run_config
	fi
else
	do_usage
fi
