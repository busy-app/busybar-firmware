#!/usr/bin/env sh

set -xe

if [ "$(dirname $0)" != "." ]; then
    echo "Please run the script from its directory"
    exit 1
fi

baked_fonts_dir="../../../../lib/lvgl_addons/fonts"

lv_font_conv --font busy_bold_7px.ttf -o ../busy_bold_7.font \
             --bpp 1 --size 16 --no-compress --format bin --range 0-65535

lv_font_conv --font busy_bold_10px.ttf -o ../busy_bold_10.font \
             --bpp 1 --size 16 --no-compress --format bin --range 0-65535

lv_font_conv --font busy_condensed_7px.ttf -o ../busy_condensed_7.font \
             --bpp 1 --size 16 --no-compress --format bin --range 0-65535

lv_font_conv --font busy_regular_5px.ttf -o ../busy_regular_5.font \
             --bpp 1 --size 16 --no-compress --format bin --range 0-65535
lv_font_conv --font busy_regular_5px.ttf -o "$baked_fonts_dir/lv_font_busy_regular_5.c" \
             --bpp 1 --size 16 --no-compress --format lvgl --range 0-65535

lv_font_conv --font busy_regular_7px.ttf -o ../busy_regular_7.font \
             --bpp 1 --size 16 --no-compress --format bin --range 0-65535
lv_font_conv --font busy_regular_7px.ttf -o "$baked_fonts_dir/lv_font_busy_regular_7.c" \
             --bpp 1 --size 16 --no-compress --format lvgl --range 0-65535
             
lv_font_conv --font busy_regular_7px.ttf -o ../busy_regular_14.font \
             --bpp 1 --size 32 --no-compress --format bin --range 0-65535

lv_font_conv --font busy_regular_9px.ttf -o ../busy_regular_9.font \
             --bpp 1 --size 16 --no-compress --format bin --range 0-65535
lv_font_conv --font busy_regular_9px.ttf -o "$baked_fonts_dir/lv_font_busy_regular_9.c" \
             --bpp 1 --size 16 --no-compress --format lvgl --range 0-65535

lv_font_conv --font busy_superscript_7px.ttf -o ../busy_superscript_7.font \
             --bpp 1 --size 16 --no-compress --format bin --range 0-65535

lv_font_conv --font busy_tiny.ttf -o ../busy_tiny.font \
             --bpp 1 --size 6 --no-compress --format bin --range 0-65535
