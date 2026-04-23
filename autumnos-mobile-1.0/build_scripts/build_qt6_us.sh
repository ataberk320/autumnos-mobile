#!/bin/bash

BR_DIR=$(pwd)
HOST_BIN="$BR_DIR/output/host/bin"
SYSROOT="$BR_DIR/output/host/riscv64-buildroot-linux-gnu/sysroot"
UIC="$HOST_BIN/uic"

for ui_file in *.ui; do
    if [ -f "$ui_file" ]; then
        output_h="ui_${ui_file%.ui}.h"
        echo "   -> $ui_file  ===>  $output_h"
        $UIC "$ui_file" -o "$output_h"
    fi
done

$HOST_BIN/riscv64-linux-g++ -O3 pmain.cpp -o com.autumnos.phone.atm \
--sysroot=$SYSROOT \
-I$BR_DIR \
-I$SYSROOT/usr/include \
-I$SYSROOT/usr/include/QtWidgets \
-I$SYSROOT/usr/include/QtGui \
-I$SYSROOT/usr/include/QtCore \
-I$SYSROOT/usr/include/QtSerialPort \
-L$SYSROOT/usr/lib \
-lQt6Widgets -lQt6Gui -lQt6Core -lQt6SerialPort -lrt \
-fPIC -std=c++17
