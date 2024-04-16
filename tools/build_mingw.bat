mkdir bin 2> nul 
gcc -o bin\http_server src\http_server.c -lws2_32
gcc -o bin\color_picker src\color_picker.c -lgdi32 -lopengl32
