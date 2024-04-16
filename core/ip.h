#ifndef IP_H
#define IP_H

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
} Ip_Error;

IP_DEF Ip_Error ip_error_last();

IP_DEF u64 ip_strlen(u8 *cstr);

typedef struct {
  u_int fd_count;
#ifdef _WIN32
  SOCKET *fd_array;
#else 
  int *fd_array;  
#endif // _WIN32

} Ip_Fd_Set;

#define IP_VALID    0x1
#define IP_CLIENT   0x2
#define IP_SERVER   0x4
#define IP_BLOCKING 0x8

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
  int addr_len;
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

#define IP_SERVER_EP_EVENTS 12

typedef struct {
  Ip_Socket *sockets; // [client1, client2, ..., server]  

  u64 active_clients;
  u64 sockets_count;
  s32 ret;

#ifdef _WIN32
  Ip_Fd_Set set;
#else
  struct epoll_event ep_events[IP_SERVER_EP_EVENTS];
  s32 ep_events_off;
  int epfd;
#endif // _WIN32

} Ip_Server;

// Ip_Server is by default nonblocking
IP_DEF Ip_Error ip_server_open(Ip_Server *s, u16 port, u64 number_of_clients);

IP_DEF Ip_Error ip_server_next(Ip_Server *s, u64 *index);
IP_DEF int ip_server_client(Ip_Server *s, u64 *index);
IP_DEF Ip_Error ip_server_accept(Ip_Server *s, u64 *index, Ip_Address *a);
IP_DEF Ip_Error ip_server_discard(Ip_Server *s, u64 index);

IP_DEF void ip_server_close(Ip_Server *s);

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
  s32 ret = recv(s->_socket, (char *) buf, buf_len, 0);
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
  s32 ret = send(s->_socket, (char *) buf, buf_len, 0);
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

IP_DEF Ip_Error ip_server_open(Ip_Server *s,
			       u16 port,
			       u64 number_of_clients) {

  Ip_Error result = IP_ERROR_NONE;
  s->ret = -1;
  s->active_clients = 0;
  s->sockets = NULL;
  s->set.fd_array = NULL;

  u64 n = number_of_clients + 1;
  // s->sockets_count = n + (16 - (n % 16));
  s->sockets_count = n;
  s->sockets = IP_ALLOC(s->sockets_count * sizeof(*s->sockets));
  if(!s->sockets) ip_return_defer(IP_ERROR_ALLOC_FAILED);
  for(u64 i=0;i<s->sockets_count;i++) {
    s->sockets[i] = ip_socket_invalid();
  }

  s->set.fd_array = IP_ALLOC(s->sockets_count * sizeof(SOCKET));
  if(!s->set.fd_array) ip_return_defer(IP_ERROR_ALLOC_FAILED);

  Ip_Error error = ip_socket_sopen(&s->sockets[s->sockets_count - 1], port, 0);
  if(error != IP_ERROR_NONE) {
    ip_return_defer(error);
  }  
  
 defer:
  if(result != IP_ERROR_NONE) {
    if(s->sockets) IP_FREE(s->sockets);
    if(s->set.fd_array) IP_FREE(s->set.fd_array);
  }
  return result;
  
}

IP_DEF Ip_Error ip_server_next(Ip_Server *s,
			       u64 *index) {
  if(s->ret == -2) {
    return IP_ERROR_REPEAT;
  }
  
  if(s->ret == -1) {

    FD_ZERO((fd_set *) &s->set);

    s->active_clients = 0;
    
    for(u64 i=0;i<s->sockets_count;i++) {
      Ip_Socket *socket = &s->sockets[i];
      
      if(socket->flags & IP_VALID) {
	FD_SET(socket->_socket, (fd_set *) &s->set);
	s->active_clients++;
      }
    }
    // Exclude server-socket
    s->active_clients--;
    
    struct timeval timeout;
    timeout.tv_sec  = 0;
    timeout.tv_usec = 1;
    
    s->ret = select(s->sockets_count,
		    (fd_set *) &s->set,
		    NULL,
		    NULL,
		    &timeout);

    if(s->ret == SOCKET_ERROR) {
      s->ret = -2;
      return ip_error_last();
    } else if(GetLastError() == WSAEWOULDBLOCK) {
      s->ret = -1;
      return IP_ERROR_REPEAT;
    }

  }
  
  if(s->ret == 0) {
    s->ret = -1;
    return IP_ERROR_REPEAT;
  }

  for(u64 i=0;i<s->sockets_count;i++) {
    if(!(s->sockets[i].flags & IP_VALID)) {
      continue;
    }

    if(!FD_ISSET(s->sockets[i]._socket, (fd_set *) &s->set)) {
      continue;
    }

    *index = i;
    s->ret -= 1;
    return IP_ERROR_NONE;
  }

  s->ret = -2;
  return ip_error_last();
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

  s->active_clients++;

  return IP_ERROR_NONE;
}

IP_DEF void ip_server_close(Ip_Server *s) {
  ip_socket_close(&s->sockets[s->sockets_count - 1]);
  IP_FREE(s->sockets);
  IP_FREE(s->set.fd_array);
}

IP_DEF Ip_Error ip_server_discard(Ip_Server *s, u64 index) {
  s->sockets[index] = ip_socket_invalid();  
  s->active_clients--;
  return IP_ERROR_NONE;
}

#else // _WIN32 

IP_DEF Ip_Error ip_error_last() {
  switch(errno) {
  case 9:
    return IP_ERROR_CONNECTION_ABORTED;
  case 111:
    return IP_ERROR_CONNECTION_REFUSED;
  case 104:
    return IP_ERROR_CONNECTION_CLOSED;
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
  s32 addr_len = sizeof(a->addr);
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
  a->addr_len = (int) sizeof(a->addr);
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
  s->active_clients = 0;
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
  event.events = EPOLLIN;
  event.data.ptr = (void *) s->sockets_count - 1;
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

IP_DEF Ip_Error ip_server_next(Ip_Server *s, u64 *index) {
  if(s->ret == -2) {
    return ip_error_last();
  } 
  
  if(s->ret == -1) {
    s->ret = epoll_wait(s->epfd, s->ep_events, IP_SERVER_EP_EVENTS, 1);
    if(s->ret < 0) {
      s->ret = -2;
      return ip_error_last();
    }
    s->ep_events_off = 0;
  }
 
  if(s->ret == 0) {
    s->ret = -1;
    return IP_ERROR_REPEAT;
  }

  struct epoll_event *ep_event = &s->ep_events[s->ep_events_off++];
  s->ret--;
  *index = (u64) ep_event->data.ptr;
  return IP_ERROR_NONE;
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
  // ep_event.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
  ep_event.events = EPOLLIN;
  ep_event.data.ptr = (void *) *index;
  if(epoll_ctl(s->epfd, EPOLL_CTL_ADD, client->_socket, &ep_event) != 0) {
    return ip_error_last();
  }

  s->active_clients++;

  return IP_ERROR_NONE;
}

IP_DEF Ip_Error ip_server_discard(Ip_Server *s, u64 index) {

  Ip_Socket *client = &s->sockets[index];
  if(epoll_ctl(s->epfd, EPOLL_CTL_DEL, client->_socket, NULL) != 0) {
    return ip_error_last();
  }

  *client = ip_socket_invalid();  
  s->active_clients--;

  return IP_ERROR_NONE;
}

IP_DEF void ip_server_close(Ip_Server *s) {
  IP_FREE(s->sockets);
  ip_socket_close(&s->sockets[s->sockets_count -1]);
  close(s->epfd);
}

#endif // _WIN32

#endif // IP_IMPLEMENTATION

#undef u8
#undef u16
#undef s32
#undef u64

#endif // IP_H
