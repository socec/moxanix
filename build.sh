#!/bin/bash

# Builds MoxaNix project

#TODO switch to Makefile :)

name="moxerver"

include_dir="."
src_list="$name.c server.c client.c tty.c telnet.c"

gcc $src_list -o $name -I $include_dir
