mkdir bin 2> nul 
cl /Fe:bin\http_server src\http_server.c ws2_32.lib
cl /Fe:bin\color_picker src\color_picker.c gdi32.lib user32.lib opengl32.lib
cl /Fe:bin\music_player src\music_player.c ole32.lib
