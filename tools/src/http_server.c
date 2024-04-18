#include <stdio.h>

#define IP_IMPLEMENTATION
#include <core/ip.h>

#define IO_IMPLEMENTATION
#include <core/io.h>

#define HTTP_IMPLEMENTATION
#include <core/http.h>

#define STR_IMPLEMENTATION
#include <core/str.h>

#include <core/types.h>

typedef unsigned long long int u64;

typedef enum {
  ERROR_NONE,
  
  ERROR_APP,
  ERROR_IP,
  ERROR_IO,
} Error_Type;

typedef enum {
  APP_ERROR_BUFFER_OVERFLOW,
  APP_ERROR_NOT_WRITTEN_ENOUGH,
} App_Error;

typedef struct {
  Error_Type type;
  union{
    Io_Error io_error;
    Ip_Error ip_error;
    App_Error app_error;
  }as;
} Error;

#define error_io(e) (Error) { .type = ERROR_IO , .as.io_error = (e) }
#define error_app(e) (Error) { .type = ERROR_APP , .as.app_error = (e) }
#define error_ip(e) (Error) { .type = ERROR_IP , .as.ip_error = (e) }
#define error_none() (Error) { .type = ERROR_NONE }

typedef struct {
  Error error;
  int response_code;
} Foo;

#define foo_from(e, r) (Foo) { .error = (e), .response_code = (r) }

char* path_to_content_type(str path) {

  if(str_index_ofc(path, ".html") >= 0) {
    return "text/html";
  } else {
    return "application/octet-stream";
  }
  
}

Foo handle_get(Ip_Socket *socket, str _path, str_builder *sb) {

  if(str_eqc(_path, "/")) {
    _path = str_fromc("/index.html");
  }

  str_builder_reserve(sb, 1 + _path.len);

  sb->data[sb->len++] = '.';
    
  for(u64 i=0;i<_path.len;i++) {
    char c = _path.data[i];
      
    if(c == '/') {
      c = IO_DELIM; 
    }

    sb->data[sb->len++] = c;
  }
  str path = str_from(sb->data, sb->len);

  u64 written;

  Io_File file;
  Io_Error io_error = io_file_ropens(&file, path);
  switch(io_error) {
  case IO_ERROR_NONE:
    break;
  case IO_ERROR_FILE_NOT_FOUND: {
    int code = 404;
    str message = str_fromc("HTTP/1.1 404 Not Found\r\n"
				  "Content-Length: 9\r\n"
				  "Content-Type: text/plain\r\n"
				  "\r\n"
				  "Not Found");

    Ip_Error error = ip_socket_writes(socket, message, &written);
    if(error != IP_ERROR_NONE) {
      return foo_from(error_ip(error), code);
    } else if(written != message.len) {
      return foo_from(error_app(APP_ERROR_NOT_WRITTEN_ENOUGH), code);
    } else {
      return foo_from(error_none(), code);
    }
  } break;
  default: {
    int code = 500;
    
    str message = str_fromc("HTTP/1.1 500 Internal Server Error\r\n"
				  "Content-Length: 21\r\n"
				  "Content-Type: text/plain\r\n"
				  "\r\n"
				  "Internal Server Error");

    Ip_Error error = ip_socket_writes(socket, message, &written);
    if(error != IP_ERROR_NONE) {
      return foo_from(error_ip(error), code);
    } else if(written != message.len) {
      return foo_from(error_app(APP_ERROR_NOT_WRITTEN_ENOUGH), code);
    } else {
      return foo_from(error_io(io_error), code);
    }
  } break;
  }

  int code = 200;

  unsigned char buf[1024];
  u64 len = (u64) snprintf((char *) buf, sizeof(buf),
				 "HTTP/1.1 200 OK\r\n"
				 "Content-Type: %s\r\n"
				 "Content-Length: %llu\r\n"
				 "Connection: keep-alive\r\n"
				 "\r\n",
				 path_to_content_type(path),
				 file.size);
  if(len > sizeof(buf)) {
    io_file_close(&file);
    return foo_from(error_app(APP_ERROR_BUFFER_OVERFLOW), code);
  }
  Ip_Error error = ip_socket_write(socket, buf, len, &written);
  if(error != IP_ERROR_NONE) {
    return foo_from(error_ip(error), code);
  }
  if(written != len) {
    return foo_from(error_app(APP_ERROR_NOT_WRITTEN_ENOUGH), code);
  }
  
  while((io_error = io_file_read(&file, buf, sizeof(buf), &len)) == IO_ERROR_NONE) {

    error = ip_socket_write(socket, buf, len, &written);
    if(error != IP_ERROR_NONE) {
      return foo_from(error_ip(error), code);
    }
    if(written != len) {
      return foo_from(error_app(APP_ERROR_NOT_WRITTEN_ENOUGH), code);
    }
    
  }
  if(io_error != IO_ERROR_EOF) {
    return foo_from(error_io(io_error), code);
  }
      
  io_file_close(&file);

  return foo_from(error_none(), code);
}

#define N 1023

typedef struct {
  Http http;
  str_builder sb;
  int inactive_cycles;
} Session;

#define MAX_INACTIVE_CYCLES_MS 4 * 1000

#ifdef _WIN32
#  define MAX_INACTIVE_CYCLES MAX_INACTIVE_CYCLES_MS / 20 // why ?
#else
#  define MAX_INACTIVE_CYCLES MAX_INACTIVE_CYCLES_MS
#endif // _WIN32

Session sessions[N] = {0};

int main(int argc, char **argv) {

  unsigned short port = 8000;
  
  if(argc > 1) {

    char *port_arg = argv[1];

    long long int n;
    if(!http_parse_s64((unsigned char *) port_arg, strlen(port_arg), &n) || n < 1) {
      fprintf(stderr, "ERROR: Cannot parse port '%s'\n", port_arg);
      fprintf(stderr, "USAGE: %s <port> | port in range [1, 65536]\n", argv[0]);
      return 1;
    }

    port = (unsigned short) n;
  }   
  
  Ip_Server server;
  switch(ip_server_open(&server,
			port,
			N)) {
  case IP_ERROR_NONE:
    break;
  default:
    TODO();
    break;
  }

  printf("Serving HTTP on :: port %u (http://[::]:%u/) ...\n",
	 port,
	 port); fflush(stdout);

  unsigned char buf[1024];
  Ip_Address address;
  str_builder global_sb = {0};
  Io_Time time;
  
  while(1) {
    u64 index;

    int try_again = 0;
    switch(ip_server_next(&server, &index)) {
    case IP_ERROR_REPEAT:
      try_again = 1;
      break;
    case IP_ERROR_NONE:
      break;
    default:
      TODO();
      break;
    }

    assert(N == server.sockets_count - 1);
    for(u64 i=0;i<server.sockets_count - 1;i++) {
      Ip_Socket *socket = &server.sockets[i];
      if(!(socket->flags & IP_VALID)) {
	continue;
      }

      if(sessions[i].inactive_cycles >= MAX_INACTIVE_CYCLES) {

	if(ip_socket_address(socket, &address) != IP_ERROR_NONE) {
	  TODO();
	}
	char ip[INET6_ADDRSTRLEN];
	inet_ntop(address.addr.sin_family, &address.addr.sin_addr, ip, INET6_ADDRSTRLEN);

	io_time_get(&time);
	printf("%llu:%s - disconnected (forced) [%02d.%02d.%04d %02d:%02d:%02d]\n",
	       i, ip,

	       time.day,
	       time.month,
	       time.year,

	       time.hour,
	       time.min,
	       time.sec); fflush(stdout);

	ip_socket_close(socket);
	ip_server_discard(&server, index);
      } else {
	sessions[i].inactive_cycles += 1;
      }
    }

    
    if(try_again) {
      continue;
    }
    
    Ip_Socket *socket = &server.sockets[index];
    if(socket->flags & IP_SERVER) {

      u64 client_index;      
      switch(ip_server_accept(&server, &client_index, &address)) {
      case IP_ERROR_REPEAT:
	break;
      case IP_ERROR_NONE:

	io_time_get(&time);

	; char ip[INET6_ADDRSTRLEN];
	inet_ntop(address.addr.sin_family, &address.addr.sin_addr, ip, INET6_ADDRSTRLEN);

	printf("%llu:%s - connected [%02d.%02d.%04d %02d:%02d:%02d]\n",
	       client_index, ip,
	       
	       time.day,
	       time.month,
	       time.year,

	       time.hour,
	       time.min,
	       time.sec); fflush(stdout);

	sessions[client_index].http = http_default();
	sessions[client_index].sb.len = 0;
	sessions[client_index].inactive_cycles = 0;
	break;
      default:
	TODO();
	break;
      }
	
    } else { // socket->flags & IP_CLIENT

      Http *http = &sessions[index].http;
      str_builder *sb = &sessions[index].sb;
      sessions[index].inactive_cycles = 0;
	
      u64 read;
      switch(ip_socket_read(socket, buf, sizeof(buf), &read)) {
      case IP_ERROR_CONNECTION_ABORTED:
      case IP_ERROR_CONNECTION_CLOSED:
	
	io_time_get(&time);
	printf("%llu:xxx.x.x.x - disconnected [%02d.%02d.%04d %02d:%02d:%02d]\n",
	       index,
	       
	       time.day,
	       time.month,
	       time.year,

	       time.hour,
	       time.min,
	       time.sec); fflush(stdout);
	
	ip_server_discard(&server, index);
	break;
      case IP_ERROR_NONE:
	break;
      default:
	TODO();
	break;
      }

      if(!(socket->flags & IP_VALID)) {
	continue;
      }
	
      int bad_request = 0;

      unsigned char *buffer = buf;
      u64 len = read;
      while(len) {
	switch(http_process(http, &buffer, &len)) {
	case HTTP_EVENT_ERROR:
	  bad_request = 1;
	  break;	  
	case HTTP_EVENT_PATH: 
	  ;
	  str params = str_from(http->value, http->value_len);
	  str path;
	  str_chop_by(&params, "?", &path);	    
	  str_builder_appends(sb, path);
	    
	  break;
	case HTTP_EVENT_HEADER:
	case HTTP_EVENT_BODY:
	case HTTP_EVENT_NOTHING:
	  break;
	}
      }
    
      if(bad_request) {

	u64 written;
	str message = str_fromc("HTTP/1.1 400 Bad Request\r\n"
				      "Content-Length: 11\r\n"
				      "Content-Type: text/plain\r\n"
				      "\r\n"
				      "Bad Request");
	Ip_Error error = ip_socket_writes(socket, message, &written);
	if(error != IP_ERROR_NONE) {
	  TODO();
	}
	if(written != message.len) {
	  TODO();
	}

	ip_socket_close(socket);
	continue;
      }
	
      if(!(http->flags & HTTP_DONE)) {
	continue;
      }

      str path = str_from(sb->data, sb->len);

      Foo foo;
      switch(http->method) {
      case HTTP_METHOD_GET:
	foo = handle_get(socket, path, &global_sb);
	break;
      default:
	; int code = 501;

	u64 written;
	str message = str_fromc("HTTP/1.1 501 Not Implemented\r\n"
				      "Content-Length: 15\r\n"
				      "Content-Type: text/plain\r\n"
				      "\r\n"
				      "Not Implemented");
	Ip_Error ip_error = ip_socket_writes(socket, message, &written);
	if(ip_error != IP_ERROR_NONE) {
	  foo = foo_from(error_ip(ip_error), code);
	} else if(written != message.len) {
	  foo = foo_from(error_app(APP_ERROR_NOT_WRITTEN_ENOUGH), code);
	} else {
	  foo = foo_from(error_none(), code);
	}
	  
	break;
      }

      // ::ffff:127.0.0.1 - - [15/Apr/2024 08:21:43] "GET / HTTP/1.1" 200 -
      if(ip_socket_address(socket, &address) != IP_ERROR_NONE) {
        TODO();
      }
      char ip[INET6_ADDRSTRLEN];
      inet_ntop(address.addr.sin_family, &address.addr.sin_addr, ip, INET6_ADDRSTRLEN);

      io_time_get(&time);
      printf("%llu:%s - - [%02d.%02d.%04d %02d:%02d:%02d] \"%s "str_fmt" HTTP/1.1\" %d -\n",
	     index,
	     ip,
	     
	     time.day,
	     time.month,
	     time.year,

	     time.hour,
	     time.min,
	     time.sec,

	     HTTP_METHOD_NAME[http->method],

	     str_arg(path),
	     
	     foo.response_code); fflush(stdout);

      Error error = foo.error;
      switch(error.type) {

      case ERROR_NONE:
	// pass
	break;

      case ERROR_IO: {
	TODO();
      } break;

      case ERROR_APP: {
	TODO();
      } break;

      case ERROR_IP: {
	TODO();
      } break;
	  
      }

      *http = http_default();
      sb->len = 0;

      global_sb.len = 0;
    }
      
    
  }

  ip_server_close(&server);
  return 0;
}
