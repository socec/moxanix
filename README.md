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

moxerverctl
-----------
- starts, stops or displays status for different moxervers
- commands can handle one specific or all moxervers at once

moxerver.cfg
------------
- defines connections between serial devices and TCP ports
- each line corresponds to one micro server handling the defined connection

Build and install
=================

Run `make` to build the project and `make install` to install it.  

This will install executables in "/usr/bin" (default prefix is "usr") and the configuration script in /etc.  

You can install directly into some other directory with `make install INSTALL_ROOT=/some/dir`.  
You can change the default install prefix for executables with `make install BIN_PREFIX=someprefix`.  
These options can also be combined into `make install INSTALL_ROOT=/some/dir BIN_PREFIX=someprefix`
