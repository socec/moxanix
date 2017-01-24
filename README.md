[![Build Status](https://travis-ci.org/socec/moxanix.svg?branch=master)](https://travis-ci.org/socec/moxanix)

Moxanix
=======

A serial device server, provides console access to multiple serial devices through telnet connection.

Architecture
============

The serial device server is broken down into multiple micro servers dedicated to a single serial device and TCP port pair.  
These micro servers are then managed by a control script. The control script allows the user to start and stop these micro servers or check their status.  
Connections between serial devices and TCP ports are configured in a separate file.  
This design allows scalability and customization based on the number of available serial connections and TCP port availability.

moxerver
--------
- a light server application handling the session between one TCP port and one serial device
- allows bidirectional communication
- it is expected to run a separate instance for every serial device and TCP port pair

moxanix.sh
----------
- starts, stops or displays status for different moxervers
- commands can handle one specific or all moxervers at once

moxanix.cfg
-----------
- defines connections between serial devices and TCP ports
- each line corresponds to one micro server handling the defined connection

Build and install
=================

Run "make" to build the project.  
The build artifacts can be found in the directory "install.dir" and should be copied from there.  
If you want to install directly to some directory run "make INSTALL_ROOT=/some/dir".
