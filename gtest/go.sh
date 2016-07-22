#!/bin/sh

# Ubuntu/Debian don't ship the binaries for this, for some reason.
# This trivial script will generate said binaries from the source in libgtest-dev

set -eu

cmake /usr/src/gtest
make
