#!/bin/sh
if [ -z "$CC" ]; then
	CC=g++
fi

if [ -z "$CFLAGS" ]; then
	CFLAGS=-O2
fi

$CC $CFLAGS -lboost_thread *.cpp -o cpptracer

