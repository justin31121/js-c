#ifndef IP_H
#define IP_H

// MIT License
// 
// Copyright (c) 2024 Justin Schartner
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifdef _WIN32
#  include <ws2tcpip.h>
#  include <windows.h>
#else 
#  include <unistd.h>
#  include <netinet/in.h>
#  include <netdb.h>
#  include <arpa/inet.h>
#  include <errno.h>
#  include <fcntl.h>
#  include <sys/epoll.h>
#  include <string.h>
#endif // _WIN32

#ifndef IP_ALLOC
#  include <stdlib.h>
#  define IP_ALLOC malloc
#endif // IP_ALLOC

#ifndef IP_FREE
#  include <stdlib.h>
#  define IP_FREE free
#endif // IP_FREE

typedef unsigned char Ip_u8;
typedef unsigned short Ip_u16;
typedef int Ip_s32;
typedef unsigned long long Ip_u64;
#define u8 Ip_u8
#define u16 Ip_u16
#define s32 Ip_s32
#define u64 Ip_u64

#ifndef IP_DEF
#  define IP_DEF static inline
#endif // IP_DEF

typedef enum {
  IP_ERROR_NONE = 0,
  IP_ERROR_REPEAT,  
  IP_ERROR_WSA_STARTUP_FAILED,
  IP_ERROR_UNKNOWN_HOSTNAME,
  IP_ERROR_ALLOC_FAILED,
  IP_ERROR_SOCKET_OVERFLOW,
  IP_ERROR_CONNECTION_CLOSED,
  IP_ERROR_CONNECTION_REFUSED,
  IP_ERROR_CONNECTION_ABORTED,
  IP_ERROR_NOT_A_SOCKET,
  IP_ERROR_NO_SUCH_FILE_OR_DIRECTORY,
  IP_ERROR_BROKEN_PIPE,
} Ip_Error;

IP_DEF Ip_Error ip_error_last();
IP_DEF u64 ip_strlen(u8 *cstr);

typedef struct {
  unsigned int fd_count;
#ifdef _WIN32
  SOCKET *fd_array;  
#endif // _WIN32  
} Ip_Fd_Set;

#define IP_VALID    0x01
#define IP_CLIENT   0x02
#define IP_SERVER   0x04
#define IP_BLOCKING 0x08
#define IP_WRITING  0x10

typedef struct {
#ifdef _WIN32
  SOCKET _socket;
#else 
  int _socket; 
#endif // _WIN32
  u64 flags;
} Ip_Socket;

#define ip_socket_invalid() (Ip_Socket) { .flags = 0 }

typedef struct {
  struct sockaddr_in addr;
#ifdef _WIN32
  int addr_len;
#else
  socklen_t addr_len;
#endif // _WIN32  
} Ip_Address;

IP_DEF Ip_Error ip_socket_copen(Ip_Socket *s, char *hosntame, u16 port);
IP_DEF Ip_Error ip_socket_sopen(Ip_Socket *s, u16 port, int blocking);

IP_DEF Ip_Error ip_socket_read(Ip_Socket *s, u8 *buf, u64 buf_len, u64 *read);
#define ip_socket_reads(s, str) ip_socket_read((s), (str).data, (str).len, &((str).len))

IP_DEF Ip_Error ip_socket_write(Ip_Socket *s, u8 *buf, u64 buf_len, u64 *written);
#define ip_socket_writec(s, cstr, n) ip_socket_write((s), (cstr), ip_strlen(cstr), (n))
#define ip_socket_writes(s, str, n) ip_socket_write((s), (str).data, (str).len, (n))

IP_DEF Ip_Error ip_socket_accept(Ip_Socket *s, Ip_Socket *client, Ip_Address *a);
IP_DEF void ip_socket_set_blocking(Ip_Socket *s, int blocking);
IP_DEF Ip_Error ip_socket_address(Ip_Socket *s, Ip_Address *a);

IP_DEF void ip_socket_close(Ip_Socket *s);

typedef enum {
  IP_MODE_READ,
  IP_MODE_WRITE,
  IP_MODE_DISCONNECT,
} Ip_Mode;

/* #define IP_SERVER_EP_EVENTS 12 */

/* typedef struct { */

/* #ifdef _WIN32 */
/*   fd_set *set_reading; */
/*   u64 off; */
/*   fd_set *set_writing; */
  
/* #else */
/*   struct epoll_event ep_events[IP_SERVER_EP_EVENTS]; */
/*   s32 ep_events_off; */
/*   int epfd; */
/* #endif // _WIN32 */

/*   Ip_Socket *sockets; // [client1, client2, ..., server]   */
/*   u64 sockets_count; */
/*   s32 ret; */

/* } Ip_Server; */

/* // Ip_Server is by default nonblocking */
/* IP_DEF Ip_Error ip_server_open(Ip_Server *s, u16 port, u64 number_of_clients); */

/* IP_DEF Ip_Error ip_server_next(Ip_Server *s, u64 *index, Ip_Mode *mode); */
/* IP_DEF int ip_server_client(Ip_Server *s, u64 *index); */
/* IP_DEF Ip_Error ip_server_accept(Ip_Server *s, u64 *index, Ip_Address *a); */
/* IP_DEF void ip_server_register_for_writing(Ip_Server *s, u64 index); */
/* IP_DEF void ip_server_unregister_for_writing(Ip_Server *s, u64 index); */
/* IP_DEF Ip_Error ip_server_discard(Ip_Server *s, u64 index); */

/* IP_DEF void ip_server_close(Ip_Server *s); */

typedef struct {
  Ip_Socket *sockets;
  u64 sockets_count;
  
  s32 ret;
#ifdef _WIN32
  u64 off;
  fd_set *set_reading;
  fd_set *set_writing;
#endif // _WIN32

} Ip_Sockets;

IP_DEF Ip_Error ip_sockets_open(Ip_Sockets *s, u64 n);
IP_DEF Ip_Error ip_sockets_next(Ip_Sockets *s, u64 *index, Ip_Mode *m);
IP_DEF void ip_sockets_close(Ip_Sockets *s);

#ifdef IP_IMPLEMENTATION

#define ip_return_defer(n) do { result = (n); goto defer; }while(0)

IP_DEF u64 ip_strlen(u8 *cstr) {
  u64 len = 0;
  while(*cstr++) len++;
  return len;
}

#ifdef _WIN32

IP_DEF Ip_Error ip_error_last() {
  DWORD last_error = GetLastError();
  
  switch(last_error) {
  case 0:
    return IP_ERROR_NONE;
  case 10035:
    return IP_ERROR_REPEAT;
  case 10014:
  case 10038:
    return IP_ERROR_NOT_A_SOCKET;
  case 10054:
    return IP_ERROR_CONNECTION_CLOSED;
  case 10053:
    return IP_ERROR_CONNECTION_ABORTED;
  case 10061:
    return IP_ERROR_CONNECTION_REFUSED;
  default:
    fprintf(stderr, "IP_ERROR: Unhandled last_error: %ld\n", last_error); fflush(stderr);
    exit(1);
  }

}

static int ip_wsa_inited = 0;

IP_DEF Ip_Error ip_socket_copen(Ip_Socket *s, char *hostname, u16 port) {

  Ip_Error result = IP_ERROR_NONE;
  struct addrinfo* addr = NULL;
  s->_socket = INVALID_SOCKET;
  s->flags = 0;

  if(!ip_wsa_inited) {

    WSADATA wsaData;   
    if(WSAStartup(0x202, (void *) &wsaData) != 0) {
      ip_return_defer(IP_ERROR_WSA_STARTUP_FAILED);
    }

    ip_wsa_inited = 1;
  }

  s->_socket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);
  if(s->_socket == INVALID_SOCKET) {
    ip_return_defer(ip_error_last());
  }

  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  u8 port_buf[32];
  u8 *port_buf_ptr = port_buf + sizeof(port_buf) - 2;
  while(port) {
    *port_buf_ptr-- = port % 10 + '0';
    port /= 10;
  }
  port_buf_ptr++;
  port_buf[31] = 0;
  
  if(getaddrinfo(hostname, (char *) port_buf_ptr, &hints, &addr) != 0) {
    ip_return_defer(IP_ERROR_UNKNOWN_HOSTNAME);
  }

  if(connect(s->_socket, addr->ai_addr, (int) addr->ai_addrlen) != 0) {
    ip_return_defer(ip_error_last());
  }

  s->flags = IP_VALID | IP_CLIENT;

 defer:
  if(addr != NULL) freeaddrinfo(addr);
  if(result != IP_ERROR_NONE && s->_socket != INVALID_SOCKET) {
    DWORD last_error = GetLastError();
    closesocket(s->_socket);
    SetLastError(last_error);
  }
  return result;
}

IP_DEF Ip_Error ip_socket_sopen(Ip_Socket *s, u16 port, int blocking) {

  Ip_Error result = IP_ERROR_NONE;
  s->_socket = INVALID_SOCKET;
  s->flags = 0;

  if(!ip_wsa_inited) {

    WSADATA wsaData;   
    if(WSAStartup(0x202, (void *) &wsaData) != 0) {
      ip_return_defer(IP_ERROR_WSA_STARTUP_FAILED);
    }

    ip_wsa_inited = 1;
  }

  s->_socket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);
  if(s->_socket == INVALID_SOCKET) {
    ip_return_defer(ip_error_last());
  } 

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  if(bind(s->_socket, (SOCKADDR*) &addr, sizeof(addr)) == SOCKET_ERROR) {
    ip_return_defer(ip_error_last());
  }

  ip_socket_set_blocking(s, blocking);

  if(listen(s->_socket, SOMAXCONN) < 0) {
    ip_return_defer(ip_error_last());
  }  
  
  s->flags = IP_VALID | IP_SERVER;

 defer:
  if(result != IP_ERROR_NONE && s->_socket != INVALID_SOCKET) {
    closesocket(s->_socket);
  }
  return result;
}

IP_DEF Ip_Error ip_socket_address(Ip_Socket *s, Ip_Address *a) {
  s32 addr_len = sizeof(a->addr);
  if(getpeername(s->_socket, (struct sockaddr *) &a->addr, &addr_len) == SOCKET_ERROR) {
    return ip_error_last();
  }
  return IP_ERROR_NONE;
}

IP_DEF Ip_Error ip_socket_read(Ip_Socket *s, u8 *buf, u64 buf_len, u64 *read) {
  s32 ret = recv(s->_socket, (char *) buf, (s32) buf_len, 0);
  if(ret < 0) {
    return ip_error_last();
  } else if(ret == 0) {
    return IP_ERROR_CONNECTION_CLOSED;
  } else {
    *read = (u64) ret;
    return IP_ERROR_NONE;
  }
}

IP_DEF Ip_Error ip_socket_write(Ip_Socket *s, u8 *buf, u64 buf_len, u64 *written) {
  s32 ret = send(s->_socket, (char *) buf, (s32) buf_len, 0);
  if(ret < 0) {
    return ip_error_last();
  } else if(ret == 0) {
    return IP_ERROR_CONNECTION_CLOSED;
  } else {
    *written = (u64) ret;
    return IP_ERROR_NONE;
  }
}

IP_DEF Ip_Error ip_socket_accept(Ip_Socket *s, Ip_Socket *client, Ip_Address *a) {

  a->addr_len = (int) sizeof(a->addr);
  SOCKET _socket = accept(s->_socket, (struct sockaddr *) &a->addr, &a->addr_len);
  if(_socket == INVALID_SOCKET) {
    return ip_error_last();
  } else if(GetLastError() == WSAEWOULDBLOCK) {
    return IP_ERROR_REPEAT;
  }
  client->_socket = _socket;
  client->flags = IP_VALID | IP_CLIENT;
  if(s->flags & IP_BLOCKING) {
    client->flags |= IP_BLOCKING;
  }
  
  return IP_ERROR_NONE;
}

IP_DEF void ip_socket_set_blocking(Ip_Socket *s, int blocking) {
  
  unsigned long mode;
  if(blocking) {
    mode = 1;
    s->flags |= IP_BLOCKING;
  } else {
    mode = 0;
    s->flags &= ~IP_BLOCKING;
  }
  ioctlsocket(s->_socket, FIONBIO, &mode);

}

IP_DEF void ip_socket_close(Ip_Socket *s) {
  closesocket(s->_socket);
  s->flags = 0;
}

/* IP_DEF Ip_Error ip_server_open(Ip_Server *s, */
/* 			       u16 port, */
/* 			       u64 number_of_clients) { */

/*   Ip_Error result = IP_ERROR_NONE; */
/*   s->ret = -1; */
/*   s->sockets = NULL; */
/*   s->set_reading = NULL; */
/*   s->set_writing = NULL; */

/*   u64 n = number_of_clients + 1; */
/*   // s->sockets_count = n + (16 - (n % 16)); */
/*   s->sockets_count = n; */
/*   s->sockets = IP_ALLOC(s->sockets_count * sizeof(*s->sockets)); */
/*   if(!s->sockets) ip_return_defer(IP_ERROR_ALLOC_FAILED); */
/*   for(u64 i=0;i<s->sockets_count;i++) { */
/*     s->sockets[i] = ip_socket_invalid(); */
/*   } */

/*   u64 sizeof_fd_set = 8 + s->sockets_count * sizeof(SOCKET); */
  
/*   u8 *memory = IP_ALLOC(sizeof_fd_set * 2); */
/*   if(!memory) ip_return_defer(IP_ERROR_ALLOC_FAILED);   */
  
/*   s->set_reading = (fd_set *) memory; */
/*   s->set_writing = (fd_set *) (memory + sizeof_fd_set); */

/*   Ip_Error error = ip_socket_sopen(&s->sockets[s->sockets_count - 1], port, 0); */
/*   if(error != IP_ERROR_NONE) { */
/*     ip_return_defer(error); */
/*   } */
  
/*  defer: */
/*   if(result != IP_ERROR_NONE) { */
/*     if(s->sockets) IP_FREE(s->sockets); */
/*     if(s->set_reading) IP_FREE(memory); */
/*   } */
/*   return result; */
  
/* } */

/* IP_DEF Ip_Error ip_server_next(Ip_Server *s, */
/* 			       u64 *index, */
/* 			       Ip_Mode *mode) { */
/*   if(s->ret == -2) { */
/*     return IP_ERROR_REPEAT; */
/*   } */
  
/*   if(s->ret == -1) { */

/*     s->set_reading->fd_count = 0; */
/*     s->set_writing->fd_count = 0; */

/*     u64 sockets_reading = 0; */
/*     u64 sockets_writing = 0; */
/*     for(u64 i=0;i<s->sockets_count;i++) { */
/*       Ip_Socket *socket = &s->sockets[i]; */
      
/*       if(!(socket->flags & IP_VALID)) { */
/* 	continue; */
/*       } */
/*       s->set_reading->fd_array[s->set_reading->fd_count++] = socket->_socket; */

/*       if((socket->flags & IP_SERVER) || !(socket->flags & IP_WRITING)) { */
/* 	continue; */
/*       } */
/*       s->set_writing->fd_array[s->set_writing->fd_count++] = socket->_socket; */
/*     } */
    
/*     struct timeval timeout; */
/*     timeout.tv_sec  = 0; */
/*     timeout.tv_usec = 1; */

/*     s->ret = select(0, */
/* 		    s->set_reading, */
/* 		    s->set_writing, */
/* 		    NULL, */
/* 		    &timeout); */
/*     s->off = 0; */

/*     if(s->ret == SOCKET_ERROR) {       */
/*       s->ret = -2; */
/*       return ip_error_last(); */
/*     } */
/*     if(s->ret == 0) { */
/*       s->ret = -1; */
/*       return IP_ERROR_REPEAT; */
/*     } */

/*   } */
  
/*   if(s->ret == 0) { */
/*     s->ret = -1; */
/*     return IP_ERROR_REPEAT; */
/*   } */

/*   // s->off, iterates from 0 to s->sockets_count, in 2 modes */
/*   // s->off: 0, index: 0, mode: reading */
/*   // s->off: 1, index: 0, mode: writing */
/*   // s->off: 2, index: 1, mode: reading */
/*   // s->off: 3, index: 1, mode: writing */
/*   // ... */
/*   for(;s->off<s->sockets_count*2;) { */
/*     u64 i = s->off / 2; */

/*     if(!(s->sockets[i].flags & IP_VALID)) { */
/*       s->off += 2; */
/*       continue; */
/*     } */

/*     if(!(s->off & 0x1)) { */
/*       s->off++; */

/*       int found = 0; */
/*       for(int j = s->set_reading->fd_count - 1;!found && j>=0;j--) { */
/* 	found = found || s->set_reading->fd_array[j] == s->sockets[i]._socket; */
/*       } */

/*       // if(FD_ISSET(s->sockets[i]._socket, (fd_set *) &s->set_reading)) { */
/*       if(found) { */
/* 	*index = i; */
/* 	*mode = IP_MODE_READ; */
/* 	s->ret -= 1; */
/* 	return IP_ERROR_NONE; */
	
/*       } */
/*     } */
/*     s->off++; */
    
/*     if(!(s->sockets[i].flags & IP_SERVER) */
/*        && (s->sockets[i].flags & IP_WRITING)) { */

/*       int found = 0; */
/*       for(int j = s->set_writing->fd_count - 1;!found && j>=0;j--) { */
/* 	found = (s->set_writing->fd_array[j] == s->sockets[i]._socket); */
/*       } */

/*       if(found) { */
/* 	*index = i; */
/* 	*mode = IP_MODE_WRITE; */
/* 	s->ret -= 1; */
/* 	return IP_ERROR_NONE;	 */
/*       } */
      
/*     } */
    

/*   } */

/*   s->ret = -2; */
/*   return ip_error_last(); */
/* } */

/* IP_DEF int ip_server_client(Ip_Server *s, u64 *index) { */
  
/*   for(u64 i=0;i<s->sockets_count - 1;i++) { */
/*     if(s->sockets[i].flags & IP_VALID) { */
/*       continue; */
/*     } */

/*     *index = i; */
/*     return 1; */
/*   } */

/*   return 0; */
/* } */

/* IP_DEF Ip_Error ip_server_accept(Ip_Server *s, u64 *index, Ip_Address *a) { */
    
/*   if(!ip_server_client(s, index)) { */
/*     return IP_ERROR_SOCKET_OVERFLOW; */
/*   } */

/*   Ip_Socket *server = &s->sockets[s->sockets_count - 1]; */
/*   Ip_Socket *client = &s->sockets[*index]; */
  
/*   Ip_Error error = ip_socket_accept(server, client, a); */
/*   if(error != IP_ERROR_NONE) { */
/*     return error; */
/*   } */

/*   return IP_ERROR_NONE; */
/* } */

/* IP_DEF void ip_server_close(Ip_Server *s) { */

/*   for(u64 i=0;i<s->sockets_count;i++) { */
/*     Ip_Socket *socket = &s->sockets[i]; */
/*     if(!(socket->flags & IP_VALID)) continue; */
/*     ip_socket_close(socket);   */
/*   } */
  
/*   IP_FREE(s->sockets); */
/*   IP_FREE(s->set_reading); */
/* } */

IP_DEF Ip_Error ip_sockets_open(Ip_Sockets *s, u64 n) {

  u64 sockets_size = n * sizeof(*s->sockets);
  u64 fd_size = 8 + n * sizeof(SOCKET);

  u8 *memory = IP_ALLOC(sockets_size + 2 * fd_size);
  if(!memory) {
    return IP_ERROR_ALLOC_FAILED;
  }
  s->sockets = (Ip_Socket *) memory;
  s->set_reading = (fd_set *) (memory + sockets_size);
  s->set_writing = (fd_set *) (memory + sockets_size + fd_size);

  for(u64 i=0;i<n;i++) {
    s->sockets[i] = ip_socket_invalid();
  }
  s->sockets_count = n;
  s->ret = -1;
  
  return IP_ERROR_NONE;
}

IP_DEF Ip_Error ip_sockets_next(Ip_Sockets *s, u64 *index, Ip_Mode *m) {

  if(s->ret == -2) {
    return ip_error_last();
  }

  if(s->ret == -1) {
    s->set_reading->fd_count = 0;
    s->set_writing->fd_count = 0;

    for(u64 i=0;i<s->sockets_count;i++) {
      Ip_Socket *socket = &s->sockets[i];
      
      if(socket->flags & IP_VALID) {
	s->set_reading->fd_array[s->set_reading->fd_count++] = socket->_socket;
      }
      if(socket->flags & IP_SERVER) continue;
      if(socket->flags & IP_WRITING) {
	s->set_writing->fd_array[s->set_writing->fd_count++] = socket->_socket;
      }
    }

    struct timeval timeout;
    timeout.tv_sec  = 0;
    timeout.tv_usec = (s->set_writing->fd_count == 0);

    s->ret = select(0,
		    s->set_reading,
		    s->set_writing,
		    NULL,
		    &timeout);
    s->off = 0;
    if(s->ret == SOCKET_ERROR) {
      s->ret = -2;
      return ip_error_last();
    }

  }

  if(s->ret == 0) {
    s->ret = -1;
    return IP_ERROR_REPEAT;
  }

  while(s->off < s->sockets_count*2) {
    u64 i = s->off / 2;

    Ip_Socket *socket = &s->sockets[i];
    if(socket->flags & IP_VALID) {

      if((s->off & 0x1) == 0) {
	s->off++;

	int found = 0;
	for(u64 j=s->set_reading->fd_count - 1;!found && j<s->set_reading->fd_count;j--) {
	  found = found || (s->set_reading->fd_array[j] == s->sockets[i]._socket);
	}
	
	if(found) {
	  *index = i;
	  *m = IP_MODE_READ;
	  s->ret -= 1;
	  return IP_ERROR_NONE;
	}
      }
      s->off++;

      // if((s->off & 0x1) == 1) {
      if(!(socket->flags & IP_SERVER) && (socket->flags & IP_WRITING)) {
	
	int found = 0;
	for(u64 j=s->set_writing->fd_count - 1;!found && j<s->set_writing->fd_count;j--) {
	  found = found || (s->set_writing->fd_array[j] == s->sockets[i]._socket);
	}

	if(found) {
	  *index = i;
	  *m = IP_MODE_WRITE;
	  s->ret -= 1;
	  return IP_ERROR_NONE;
	}
      }
      // }
      
    } else {
      s->off += 2;
    }

    
  }

  s->ret = -2;
  return ip_error_last();
}

IP_DEF void ip_sockets_close(Ip_Sockets *s) {

  for(u64 i=0;i<s->sockets_count;i++) {
    Ip_Socket *socket = &s->sockets[i];
    if(!(socket->flags & IP_VALID)) continue;
    ip_socket_close(socket);
  }

  IP_FREE(s->sockets);
}

/* IP_DEF Ip_Error ip_server_discard(Ip_Server *s, u64 index) { */
/*   s->sockets[index] = ip_socket_invalid(); */
/*   s->ret = -1; */
/*   return IP_ERROR_NONE; */
/* } */

#else // _WIN32

IP_DEF Ip_Error ip_error_last() {
  switch(errno) {
  case 2:
    return IP_ERROR_NO_SUCH_FILE_OR_DIRECTORY;
  case 9:
    return IP_ERROR_CONNECTION_ABORTED;
  case 111:
    return IP_ERROR_CONNECTION_REFUSED;
  case 11:
    return IP_ERROR_REPEAT;
  case 104:
    return IP_ERROR_CONNECTION_CLOSED;
  case 88:
    return IP_ERROR_NOT_A_SOCKET;
  case 32:
    return IP_ERROR_BROKEN_PIPE;
  default:
    fprintf(stderr, "IP_ERROR: Unhandled last_error: %d\n", errno);
    fprintf(stderr, "IP_ERROR: '%s'\n", strerror(errno));
    fflush(stderr);
    exit(1);
  }
}

IP_DEF Ip_Error ip_socket_copen(Ip_Socket *s, char *hostname, u16 port) {
  Ip_Error result = IP_ERROR_NONE;
  s->_socket = -1; 
  s->flags = 0;  

  s->_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(s->_socket < 0) {
    ip_return_defer(ip_error_last());
  }
 
  struct hostent *hostent = gethostbyname(hostname);
  if(!hostent) {
    ip_return_defer(IP_ERROR_UNKNOWN_HOSTNAME);
  }

  in_addr_t in_addr = inet_addr(inet_ntoa(*(struct in_addr *) *(hostent->h_addr_list)));
  if(in_addr == (in_addr_t) -1) {
    ip_return_defer(IP_ERROR_UNKNOWN_HOSTNAME);
  }

  struct sockaddr_in addr = {0};
  addr.sin_addr.s_addr = in_addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  
  if(connect(s->_socket, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    ip_return_defer(ip_error_last());
  }

  s->flags = IP_VALID | IP_CLIENT; 

 defer:
  if(result != IP_ERROR_NONE && s->_socket < 0) {
    close(s->_socket);
  }
  return result; 
}

IP_DEF Ip_Error ip_socket_sopen(Ip_Socket *s, u16 port, int blocking) {
  
  Ip_Error result = IP_ERROR_NONE;
  s->_socket = -1;
  s->flags = 0;

  s->_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(s->_socket < 0) {
    ip_return_defer(ip_error_last());
  }
  
  s32 enable = 1;
  if(setsockopt(s->_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(s32)) < 0) {
    ip_return_defer(ip_error_last());
  }

  if(setsockopt(s->_socket, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(s32)) < 0) {
    ip_return_defer(ip_error_last());
  }
  
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);
  
  if(bind(s->_socket, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    ip_return_defer(ip_error_last());
  }

  ip_socket_set_blocking(s, blocking);

  if(listen(s->_socket, 128) < 0) {
    ip_return_defer(ip_error_last()); 
  } 

  s->flags = IP_VALID | IP_SERVER; 

 defer:
  if(result != IP_ERROR_NONE && s->_socket < 0) {
    close(s->_socket);
  }
  return result;
}

IP_DEF void ip_socket_set_blocking(Ip_Socket *s, int blocking) {

  int flags = fcntl(s->_socket, F_GETFL, 0);
  if(blocking) {
    flags &= ~O_NONBLOCK;
    s->flags |= IP_BLOCKING; 
  } else {
    flags |= O_NONBLOCK;
    s->flags &= ~IP_BLOCKING;
  }
  fcntl(s->_socket, F_SETFL, flags);
}

IP_DEF Ip_Error ip_socket_address(Ip_Socket *s, Ip_Address *a) {
  socklen_t addr_len = (socklen_t) sizeof(a->addr);
  if(getpeername(s->_socket, (struct sockaddr *) &a->addr, &addr_len) != 0) {
    return ip_error_last();
  }
  return IP_ERROR_NONE;
}

IP_DEF Ip_Error ip_socket_read(Ip_Socket *s, u8 *buf, u64 buf_len, u64 *_read) {
  s32 ret = read(s->_socket, (char *) buf, buf_len);
  if(ret < 0) {
    return ip_error_last();
  } else if(ret == 0) {
    return IP_ERROR_CONNECTION_CLOSED;
  } else {
    *_read = (u64) ret;
    return IP_ERROR_NONE;
  }

}

IP_DEF Ip_Error ip_socket_write(Ip_Socket *s, u8 *buf, u64 buf_len, u64 *written) {
  
  s32 ret = write(s->_socket, (char *) buf, buf_len);
  if(ret < 0) {
    return ip_error_last();
  } else if(ret == 0) {
    return IP_ERROR_CONNECTION_CLOSED;
  } else {
    *written = (u64) ret;
    return IP_ERROR_NONE;
  }
}

IP_DEF void ip_socket_close(Ip_Socket *s) {
  close(s->_socket);
  s->flags = 0;
}

IP_DEF Ip_Error ip_socket_accept(Ip_Socket *s, Ip_Socket *client, Ip_Address *a) {
  a->addr_len = (socklen_t) sizeof(a->addr);
  int fd = accept(s->_socket, (struct sockaddr *) &a->addr, &a->addr_len);
  if(fd <= 0) {
    if(errno == EAGAIN || errno == EWOULDBLOCK) {
      return IP_ERROR_REPEAT;
    } else {
      return ip_error_last();
    } 
  }
  client->_socket = fd;
  client->flags = IP_VALID | IP_CLIENT;
  ip_socket_set_blocking(client, s->flags & IP_BLOCKING);
  
  return IP_ERROR_NONE;
}

IP_DEF Ip_Error ip_server_open(Ip_Server *s, u16 port, u64 number_of_clients) {

  Ip_Error result = IP_ERROR_NONE;
  s->ret = -1;
  s->sockets = NULL;
  s->epfd = -1;

  s->sockets_count = number_of_clients + 1;
  s->sockets = IP_ALLOC(s->sockets_count * sizeof(Ip_Socket));
  if(!s->sockets) ip_return_defer(IP_ERROR_ALLOC_FAILED);
  for(u64 i=0;i<s->sockets_count;i++) {
    s->sockets[i] = ip_socket_invalid();
  }

  Ip_Socket *server = &s->sockets[s->sockets_count - 1];
  Ip_Error error = ip_socket_sopen(server, port, 0);
  if(error != IP_ERROR_NONE) {
    ip_return_defer(error);
  }
  
  s->epfd = epoll_create(1);
  if(s->epfd < 0) {
    ip_return_defer(ip_error_last());
  }
  
  struct epoll_event event;
  event.events = EPOLLIN | EPOLLET;
  event.data.ptr = (void *) (s->sockets_count - 1);
  if(epoll_ctl(s->epfd, EPOLL_CTL_ADD, server->_socket, &event) != 0) {
    ip_return_defer(ip_error_last());
  }

 defer:
  if(result != IP_ERROR_NONE) {
    if(s->sockets) IP_FREE(s->sockets);
    if(server->flags & IP_VALID) ip_socket_close(server);  
  }
  return result; 
}

IP_DEF Ip_Error ip_server_next(Ip_Server *s,
			       u64 *index,
			       Ip_Mode *mode) {
  if(s->ret == -2) {
    return ip_error_last();
  } 
  
  if(s->ret == -1) {
    s->ret = epoll_wait(s->epfd, s->ep_events, IP_SERVER_EP_EVENTS, 10);
    if(s->ret < 0) {
      s->ret = -2;
      return ip_error_last();
    }
    s->ep_events_off = 0;
  }

 repeat:
 
  if(s->ret == 0) {
    s->ret = -1;
    return IP_ERROR_REPEAT;
  }

  struct epoll_event *ep_event = &s->ep_events[s->ep_events_off];
  *index = (u64) ep_event->data.ptr;

  if((ep_event->events & EPOLLERR)) {
    ep_event->events &= ~EPOLLERR;
  }

  if((ep_event->events & EPOLLRDHUP) ||
     (ep_event->events & EPOLLHUP) ||
     (ep_event->events & EPOLLERR)) {
    *mode = IP_MODE_DISCONNECT;
    ep_event->events &= ~EPOLLRDHUP;
    ep_event->events &= ~EPOLLHUP;    
    return IP_ERROR_NONE;
  }

  if(ep_event->events & EPOLLIN) {
    *mode = IP_MODE_READ;
    ep_event->events &= ~EPOLLIN;
    return IP_ERROR_NONE;
  }

  if(ep_event->events & EPOLLOUT) {
    ep_event->events &= ~EPOLLOUT;

    if(s->sockets[*index].flags & IP_WRITING) {
      *mode = IP_MODE_WRITE;
      return IP_ERROR_NONE;
    }
    
  }

  s->ep_events_off++;
  s->ret--;

  goto repeat;
  
}

IP_DEF int ip_server_client(Ip_Server *s, u64 *index) {

  for(u64 i=0;i<s->sockets_count - 1;i++) {
    if(s->sockets[i].flags & IP_VALID) {
      continue;
    }

    *index = i;
    return 1;
  }

  return 0;

}

IP_DEF Ip_Error ip_server_accept(Ip_Server *s, u64 *index, Ip_Address *a) {
  if(!ip_server_client(s, index)) {
    return IP_ERROR_SOCKET_OVERFLOW;
  }

  Ip_Socket *server = &s->sockets[s->sockets_count - 1];
  Ip_Socket *client = &s->sockets[*index];
  
  Ip_Error error = ip_socket_accept(server, client, a);
  if(error != IP_ERROR_NONE) {
    return error;
  }

  struct epoll_event ep_event;
  ep_event.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP | EPOLLOUT;
  ep_event.data.ptr = (void *) *index;
  if(epoll_ctl(s->epfd, EPOLL_CTL_ADD, client->_socket, &ep_event) != 0) {
    return ip_error_last();
  }

  return IP_ERROR_NONE;
}

IP_DEF Ip_Error ip_server_discard(Ip_Server *s, u64 index) {

  Ip_Socket *client = &s->sockets[index];
  if(epoll_ctl(s->epfd, EPOLL_CTL_DEL, client->_socket, NULL) != 0) {
    return ip_error_last();
  }
  ip_socket_close(client);
 

  return IP_ERROR_NONE;
}

IP_DEF void ip_server_close(Ip_Server *s) {
  IP_FREE(s->sockets);
  ip_socket_close(&s->sockets[s->sockets_count -1]);
  close(s->epfd);
}

#endif // _WIN32

/* IP_DEF void ip_server_register_for_writing(Ip_Server *s, u64 index) { */
/*   Ip_Socket *client = &s->sockets[index]; */
/*   client->flags |= IP_WRITING; */
/* } */

/* IP_DEF void ip_server_unregister_for_writing(Ip_Server *s, u64 index) { */
/*   Ip_Socket *client = &s->sockets[index]; */
/*   client->flags &= ~IP_WRITING; */
/* } */

#endif // IP_IMPLEMENTATION

#undef u8
#undef u16
#undef s32
#undef u64

#endif // IP_H
