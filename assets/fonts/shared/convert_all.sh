#!/usr/bin/env sh

set -xe

lv_font_conv --font busy_bold_7px.ttf -o busy_bold_7.font \
             --bpp 1 --size 16 --no-compress --format bin --range 0-65535

lv_font_conv --font busy_bold_10px.ttf -o busy_bold_10.font \
             --bpp 1 --size 16 --no-compress --format bin --range 0-65535

lv_font_conv --font busy_condensed_7px.ttf -o busy_condensed_7.font \
             --bpp 1 --size 16 --no-compress --format bin --range 0-65535

lv_font_conv --font busy_regular_5px.ttf -o busy_regular_5.font \
             --bpp 1 --size 16 --no-compress --format bin --range 0-65535

lv_font_conv --font busy_regular_7px.ttf -o busy_regular_7.font \
             --bpp 1 --size 16 --no-compress --format bin --range 0-65535
lv_font_conv --font busy_regular_7px.ttf -o ../../applications/services/font_registry/baked/lv_font_busy_regular_7.c \
             --bpp 1 --size 16 --no-compress --format lvgl --range 0-65535
lv_font_conv --font busy_regular_7px.ttf -o busy_regular_14.font \
             --bpp 1 --size 32 --no-compress --format bin --range 0-65535

lv_font_conv --font busy_regular_9px.ttf -o busy_regular_9.font \
             --bpp 1 --size 16 --no-compress --format bin --range 0-65535
lv_font_conv --font busy_regular_9px.ttf -o ../../applications/services/font_registry/baked/lv_font_busy_regular_9.c \
             --bpp 1 --size 16 --no-compress --format lvgl --range 0-65535

lv_font_conv --font busy_superscript_7px.ttf -o busy_superscript_7.font \
             --bpp 1 --size 16 --no-compress --format bin --range 0-65535
