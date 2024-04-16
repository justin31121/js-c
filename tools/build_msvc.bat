mkdir bin 2> nul 
cl /Fe:bin\http_server src\http_server.c ws2_32.lib
