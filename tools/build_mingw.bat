mkdir bin 2> nul

::set FLAGS=-Wextra -Wall
set FLAGS=

gcc %FLAGS% -o bin\http_server src\http_server.c -lws2_32
gcc %FLAGS% -o bin\color_picker src\color_picker.c -lgdi32 -lopengl32
gcc %FLAGS% -o bin\music_player src\music_player.c -lole32 -lxaudio2_8
gcc %FLAGS% -o bin\image_converter src\image_converter.c
gcc %FLAGS% -o bin\image_viewer src\image_viewer.c -lgdi32 -lopengl32

