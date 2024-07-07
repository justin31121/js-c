#include <stdio.h>

#define HTTPSERVER_IMPLEMENTATION
#include <core/httpserver.h>

#define FTPSERVER_IMPLEMENTATION
#include <core/ftpserver.h>

#define CLIENTS 1024
#define HTTPSERVER_SOCKETS_COUNT \
  ((HTTPSERVER_SOCKETS_PER_CLIENT*CLIENTS) + 1)
#define FTPSERVER_SOCKETS_COUNT \
  ((FTPSERVER_SOCKETS_PER_CLIENT*CLIENTS) + 1)

int main() {

  str dir = str_fromd("./rsc/");
  str username = str_fromd("admin");
  str password = str_fromd("nimda");

  str_builder sb = {0};
  str_builder_appendf(&sb, str_fmt":"str_fmt, str_arg(username), str_arg(password));
  str authorization = str_from(sb.data, sb.len);

  str_builder_reserve(&sb, sb.len + base64_encode(NULL, 0,
						  authorization.data, authorization.len));
  str authorization_b64 = str_from( sb.data + authorization.len,
				   base64_encode(sb.data + authorization.len, sb.cap - authorization.len,
						 authorization.data, authorization.len));
  sb.len = 0;
  
  printf("'"str_fmt"' -b64-> '"str_fmt"'\n",
	 str_arg(authorization),
	 str_arg(authorization_b64));
  
  
  Ip_Sockets sockets;
  if(ip_sockets_open(&sockets, HTTPSERVER_SOCKETS_COUNT + FTPSERVER_SOCKETS_COUNT) != IP_ERROR_NONE) {
    return 1;
  }

  /////////////////////////////////////////////////////////

  u16 http_port = 8080;
  Http_Server server;
  if(!httpserver_open(&server, CLIENTS)) {
    return 1;
  }  
  if(ip_socket_sopen(&sockets.sockets[HTTPSERVER_SOCKETS_COUNT - 1], http_port, 0) != IP_ERROR_NONE) {
    return 1;
  }
  
  /////////////////////////////////////////////////////////

  u16 ftp_port = 8021;
  Ftp_Server ftp_server;
  if(!ftpserver_open(&ftp_server,
		     CLIENTS,
		     dir,
		     username,
		     password)) {
    return 1;
  }
  if(ip_socket_sopen(&sockets.sockets[HTTPSERVER_SOCKETS_COUNT + FTPSERVER_SOCKETS_COUNT - 1], ftp_port, 0) != IP_ERROR_NONE) {
    return 1;
  }
  
  /////////////////////////////////////////////////////////

  printf("Listening on http://localhost:%u\n", http_port);
  printf("Listening on ftp://localhost:%u\n", ftp_port);
  fflush(stdout);
  
  while(1) {

    u64 index;
    Ip_Mode mode;
    Ip_Error error = ip_sockets_next(&sockets, &index, &mode);

    if(index < HTTPSERVER_SOCKETS_COUNT) {
      Http_Server_Request request;
      if(httpserver_next(&server,
			 &sockets,
			 0,
			 HTTPSERVER_SOCKETS_COUNT,
			 error,
			 index,
			 mode,
			 &request)) {
	Http_Server_Session *s = &server.sessions[index];
	if(httpserver_is_authenticated(s,
				       &request,
				       username,
				       password,
				       &sb)) {
	  httpserver_serve_files(s, dir, &request, &sb);
	}
	
	if(server.sessions[index].queue_len > 0) sockets.sockets[index].flags |= IP_WRITING;
      }
      
    } else {
      ftpserver_next(&ftp_server,
		     &sockets,
		     HTTPSERVER_SOCKETS_COUNT,
		     FTPSERVER_SOCKETS_COUNT,
		     error,
		     index,
		     mode);
      
      
    }
    
  }

  httpserver_close(&server);
  ftpserver_close(&ftp_server);
  ip_sockets_close(&sockets);
  STR_FREE(sb.data);
  
  return 0;
}
