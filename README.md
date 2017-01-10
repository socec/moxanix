[![Build Status](https://travis-ci.org/socec/moxanix.svg?branch=master)](https://travis-ci.org/socec/moxanix)

moxanix
=======

A serial device server, provides console access to multiple serial devices through telnet connection.


moxerver
--------
- server application handling the session between a specific TCP port and a specific serial device
- allows bidirectional communication
- it is expected to run a separate instance for every serial device

moxanix.sh
----------
- starts, stops or displays status for different moxervers
- commands can handle one specific or all moxervers at once
