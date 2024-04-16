mkdir bin 2> nul 
gcc -o bin\http_server src\http_server.c -lws2_32
