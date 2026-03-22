#!/bin/bash




BR_PATH="/home/ataberk/buildroot"
CC="$BR_PATH/output/host/bin/riscv64-linux-gcc"
LVGL_SRC="$BR_PATH/output/build/lvgl-8.3.11"
LV_DRV_SRC="$BR_PATH/output/build/lv_drivers-v8.3.0"
STAGING="$BR_PATH/output/staging"


$CC -o sysui *.c \
$(find $LVGL_SRC/src -name "*.c") \
$(find $LV_DRV_SRC -name "*.c" ! -name "lv_drv_conf_template.c") \
-I. \
-I$LVGL_SRC \
-I$BR_PATH/output/build \
-I$LV_DRV_SRC/indev \
-I$STAGING/usr/include/freetype2 \
-I$STAGING/usr/include/freetype2/freetype \
-L$STAGING/usr/lib \
-lfreetype -lpthread -lm \
-D LV_CONF_INCLUDE_SIMPLE \
-D LV_LVGL_H_INCLUDE_SIMPLE \
-D LV_DRV_CONF_INCLUDE_SIMPLE \
-D USE_FBDEV=1 \
-D USE_EVDEV=1 \
-D LV_USE_FREETYPE=1 \
-D LV_CONF_PATH=lv_conf.h

if [ $? -eq 0 ]; then
    echo "Successfully compiled!"
else
    echo "An error occurred!"
fi
