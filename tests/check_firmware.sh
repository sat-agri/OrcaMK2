#!/bin/sh
set -eu
cd "$(dirname "$0")/.."

mavlink_dir=.pio/libdeps/portenta_h7_m7/MAVLink
if [ ! -f "$mavlink_dir/MAVLink_common.h" ]; then
    echo "Run 'pio run' first to install the MAVLink dependency." >&2
    exit 1
fi

build_dir=$(mktemp -d)
trap 'rm -rf "$build_dir"' EXIT HUP INT TERM
"${CXX:-c++}" -std=c++14 -Itests -Isrc -isystem "$mavlink_dir" \
    src/main.cpp src/motors.cpp src/pixhawk.cpp tests/firmware_check.cpp \
    -o "$build_dir/firmware_check"
"$build_dir/firmware_check"
