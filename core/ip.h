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
#  include <iphlpapi.h>
#else 
#  include <unistd.h>
#  include <netinet/in.h>
#  include <netdb.h>
#  include <arpa/inet.h>
#  include <errno.h>
#  include <fcntl.h>
#  include <sys/epoll.h>
#  include <string.h>
#  include <ifaddrs.h>
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
  IP_ERROR_EMPTY,
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

typedef u8 Ip[64];

IP_DEF Ip_Error ip_get_address(Ip ip);

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

#define IP_SOCKETS_EP_EVENTS 32

typedef struct {
  Ip_Socket *sockets;
  u64 sockets_count;

  s32 ret;
  u64 off;
#ifdef _WIN32
  fd_set *set_reading;
  fd_set *set_writing;
#else
  struct epoll_event ep_events[IP_SOCKETS_EP_EVENTS];
  s32 epfd;
#endif // _WIN32

} Ip_Sockets;

IP_DEF Ip_Error ip_sockets_open(Ip_Sockets *s, u64 n);
IP_DEF Ip_Error ip_sockets_next(Ip_Sockets *s, u64 *index, Ip_Mode *m);
IP_DEF void ip_sockets_close(Ip_Sockets *s);

IP_DEF Ip_Error ip_sockets_register(Ip_Sockets *s, u64 index);
IP_DEF Ip_Error ip_sockets_unregister(Ip_Sockets *s, u64 index);

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

IP_DEF Ip_Error ip_get_address(Ip ip) {

  IP_ADAPTER_INFO *adapter_info = IP_ALLOC(sizeof(IP_ADAPTER_INFO));
  if(!adapter_info) {
    return IP_ERROR_ALLOC_FAILED;
  }
  
  ULONG size = sizeof(IP_ADAPTER_INFO);  
  if(GetAdaptersInfo(adapter_info, &size) == ERROR_BUFFER_OVERFLOW) {
    IP_FREE(adapter_info);
    adapter_info = IP_ALLOC(size);
    if(!adapter_info) {
      return IP_ERROR_ALLOC_FAILED;
    }
  }
  
  if(GetAdaptersInfo(adapter_info, &size) != NO_ERROR) {
    return ip_error_last();
  }
  IP_ADAPTER_INFO *curr = adapter_info;

  int found = 0;
  while(!found && curr) {
    if (!curr->DhcpEnabled) {
      continue;
    }
    found = 1;

    u64 len = ip_strlen(curr->IpAddressList.IpAddress.String);
    memcpy(ip, curr->IpAddressList.IpAddress.String, len + 1);
    
    curr = curr->Next;
  }

  IP_FREE(adapter_info);

  if(found) {
    return IP_ERROR_NONE;
  } else {
    return IP_ERROR_EMPTY;
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

  s32 buffer_size = 256 * 1024;
  if(setsockopt(client->_socket,
		SOL_SOCKET,
		SO_SNDBUF,
		(char *) &buffer_size,
		sizeof(s32)) != 0) {
    exit(69);
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

IP_DEF Ip_Error ip_sockets_register(Ip_Sockets *s, u64 index) {
  (void) s;
  (void) index;
  return IP_ERROR_NONE;
}

IP_DEF Ip_Error ip_sockets_unregister(Ip_Sockets *s, u64 index) {
  (void) s;
  (void) index;
  return IP_ERROR_NONE;
}

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

IP_DEF Ip_Error ip_get_address(Ip ip) {

  struct ifaddrs *ifaddrs;
  if(getifaddrs(&ifaddrs) == -1) {
    return ip_error_last();
  }

  u8 localhost[] = "127.0.0.1";

  struct ifaddrs *ifa;
  int found = 0;
  for(ifa = ifaddrs;!found && ifa!=NULL;ifa=ifa->ifa_next) {
    if(ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET) {
      continue;
    }

    int ret = getnameinfo(ifa->ifa_addr,
			  sizeof(*ifa->ifa_addr),
			  ip,
			  sizeof(ip),
			  NULL,
			  0,
			  NI_NUMERICHOST);
    if(ret != 0) {
      continue;
    }
    if(strcmp(ip, localhost) == 0) {
      continue;
    }

    found = 1;
  }

  freeifaddrs(ifaddrs);
  
  return IP_ERROR_NONE;
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

IP_DEF Ip_Error ip_sockets_open(Ip_Sockets *s, u64 n) {
  s->sockets = IP_ALLOC(n * sizeof(*s->sockets));
  if(!s->sockets) {
    return IP_ERROR_ALLOC_FAILED;
  }
  s->sockets_count = n;
  for(u64 i=0;i<s->sockets_count;i++) {
    s->sockets[i] = ip_socket_invalid();
  }

  s->epfd = epoll_create(1);
  if(s->epfd < 0) {
    IP_FREE(s->sockets);
    return ip_error_last();
  }

  s->ret = -1;
  s->off = 0;

  return IP_ERROR_NONE;
}

IP_DEF Ip_Error ip_sockets_next(Ip_Sockets *s, u64 *index, Ip_Mode *m) {

 repeat:

  if(s->ret == -1) {
    s->ret = epoll_wait(s->epfd, s->ep_events, IP_SOCKETS_EP_EVENTS, 10);
    if(s->ret < 0) {
      s->ret = -2;
      return ip_error_last();
    }
    s->off = 0;
  }

  if(s->ret == 0) {
    s->ret = -1;
    goto repeat;
  }

  struct epoll_event *ep_event = &s->ep_events[s->off];
  *index = (u64) ep_event->data.ptr;
  if((ep_event->events & EPOLLRDHUP) || 
     (ep_event->events & EPOLLHUP) || 
     (ep_event->events & EPOLLERR)) {
    s->sockets[*index] = ip_socket_invalid();

    ep_event->events &= ~EPOLLRDHUP;
    ep_event->events &= ~EPOLLHUP;
    ep_event->events &= ~EPOLLERR;
    s->off++;
    goto repeat;
  }

  if(ep_event->events & EPOLLIN) {
    *m = IP_MODE_READ;
    ep_event->events &= ~EPOLLIN;
    return IP_ERROR_NONE;
  }

  if(ep_event->events & EPOLLOUT) {
    ep_event->events &= ~EPOLLOUT;

    if(s->sockets[*index].flags & IP_WRITING) {
      *m = IP_MODE_WRITE;
      return IP_ERROR_NONE;
    }
  }

  s->off++;
  s->ret--;

  goto repeat;
}

IP_DEF void ip_sockets_close(Ip_Sockets *s) {

  for(u64 i=0;i<s->sockets_count;i++) {
    Ip_Socket *socket = &s->sockets[i];
    if(!(socket->flags & IP_VALID)) continue;
    ip_socket_close(socket);
  }
  IP_FREE(s->sockets);
  close(s->epfd);
}

IP_DEF Ip_Error ip_sockets_register(Ip_Sockets *s, u64 index) {
  Ip_Socket *socket = &s->sockets[index];
  if(socket->flags & IP_VALID) {

    struct epoll_event ep_event;
    if(socket->flags & IP_SERVER) {
      ep_event.events = EPOLLIN | EPOLLET;
    } else if(socket->flags & IP_CLIENT) {
      ep_event.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP | EPOLLOUT;
    } else {
      TODO();
    }
    ep_event.data.ptr = (void *) index;
    if(epoll_ctl(s->epfd, 
		 EPOLL_CTL_ADD, 
		 s->sockets[index]._socket, 
		 &ep_event) != 0) {
      return ip_error_last();
    } else {
      return IP_ERROR_NONE;
    }

  } else {
    TODO();
  }

}

IP_DEF Ip_Error ip_sockets_unregister(Ip_Sockets *s, u64 index) {
  if(epoll_ctl(s->epfd, 
	       EPOLL_CTL_DEL, 
	       s->sockets[index]._socket, 
	       NULL) != 0) {
    return ip_error_last();
  }

  return IP_ERROR_NONE;
}


#endif // _WIN32
       //
#endif // IP_IMPLEMENTATION

#undef u8
#undef u16
#undef s32
#undef u64

#endif // IP_H
