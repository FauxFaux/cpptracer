#!/bin/sh

# Gentoo's suggestions:
# Intel (core2): CFLAGS='-march=nocona -O2 -pipe' ./build.sh
# Intel (i7):    CFLAGS='-march=core2 -msse4 -mcx16 -msahf -O2 -pipe' ./build.sh

# You may need non--mt boost_thread-mt.

if [ -z "$CC" ]; then
	CC=g++
fi

if [ -z "$CFLAGS" ]; then
	CFLAGS=-O2
fi

$CC $CFLAGS -lboost_thread-mt *.cpp -o cpptracer

