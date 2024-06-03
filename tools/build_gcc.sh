#!/bin/sh -x
mkdir bin 2> /dev/null

FLAGS="-Wextra -Wall -ggdb"
# FLAGS=

gcc $FLAGS -o bin/http_server src/http_server.c
gcc $FLAGS -o bin/color_picker src/color_picker.c -lGLX -lX11 -lGL -lm
gcc $FLAGS -o bin/music_player src/music_player.c -lasound
gcc $FLAGS -o bin/image_converter src/image_converter.c -lm
gcc $FLAGS -o bin/image_viewer src/image_viewer.c -lGLX -lX11 -lGL -lm
