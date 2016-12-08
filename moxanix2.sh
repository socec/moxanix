#!/bin/bash

CONFIGURATION_FILE="moxanix2.cfg"

LINES=()
CONNECTIONS=()
SIZE=0
ARGUMENTS=()

read_config()
{
	# read lines from the configuration file
	readarray LINES < $CONFIGURATION_FILE

	# count the number of configured connections and extract arguments
	# while skipping comment lines and empty lines
	count=0
	while [ "${LINES[count]}" != "" ]
	do
		LINE=${LINES[count]}
		# filter lines and arguments according to the configuration format:
		# tcp=<tcp_port> tty=<tty_device> baud=<tty_baudrate>
		LINE_VALID=$(echo $LINE | grep -E "^tcp=")
		if [ -n "$LINE_VALID" ]; then
			CONNECTIONS[SIZE]=$LINE
			TCP=$(echo $LINE | awk '{print $1}' | tr -d "tcp=")
			TTY=$(echo $LINE | awk '{print $2}' | tr -d "tty=")
			BAUD=$(echo $LINE | awk '{print $3}' | tr -d "baud=")
			ARGUMENTS[SIZE]="-p $TCP -t $TTY -b $BAUD"
			# increment size counter
			SIZE=$(( $SIZE + 1 ))
		fi
		# increment line counter
		count=$(( $count + 1 ))
	done
}

read_config
echo "size is $SIZE"
echo ${CONNECTIONS[3]}
echo ${ARGUMENTS[3]}
