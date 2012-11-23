#!/bin/bash
export LDFLAGS="-L/usr/xenomai/lib -lpthread"
export CFLAGS="-I/usr/xenomai/include -D_GNU_SOURCE -D_REENTRANT -D__XENO__"
export LD_LIBRARY_PATH="/usr/xenomai/lib"
