#ifndef FTPSERVER_H
#define FTPSERVER_H

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

#ifndef FTPSERVER_DEF
#  define FTPSERVER_DEF static inline
#endif // FTPSERVER_DEF

#ifndef FTPSERVER_ALLOC
#  include <stdlib.h>
#  define FTPSERVER_ALLOC malloc
#endif // FTPSERVER_ALLOC

#ifndef FTPSERVER_FREE
#  include <stdlib.h>
#  define FTPSERVER_FREE free
#endif // FTPSERVER_FREE

#ifdef FTPSERVER_IMPLEMENTATION
#  define IP_IMPLEMENTATION
#  define STR_IMPLEMENTATION
#  define FS_IMPLEMENTATION
#endif // FTPSERVER_IMPLEMENTATION

#include <core/ip.h>
#include <core/str.h>
#include <core/fs.h>
#include <core/types.h>

#define FTPSERVER_SOCKETS_PER_CLIENT 3 // text + data + data_acceptor

typedef enum {
  FTPSERVER_ACTION_KIND_NONE,
  FTPSERVER_ACTION_KIND_MESSAGE,
  FTPSERVER_ACTION_KIND_WRITE_FILE,
  FTPSERVER_ACTION_KIND_READ_FILE,
} Ftp_Server_Action_Kind;

#define FTPSERVER_SESSION_WINDOW_SIZE 1024

typedef struct {
  
  // state
  u8 dir[FS_MAX_PATH];
  u64 dir_len;
  int logged_in;
  
  // allocations
  str_builder sb;  
  
  // request
  u8 request[64];
  u64 request_len;

  // response
  Ftp_Server_Action_Kind response_kind;
  Fs_File file;
  str message;
  
  Ftp_Server_Action_Kind data_kind;
  int look_for_data_connection;
  
} Ftp_Server_Session;

FTPSERVER_DEF int ftpserver_session_fill_for_data(Ftp_Server_Session *s);

typedef struct {
  Ftp_Server_Session *sessions;
  u64 number_of_clients;
  str dir_base;
  str username;
  str password;

  // current directory
  u8 path[FS_MAX_PATH];
  u64 path_len;

  Ip ip;
} Ftp_Server;

FTPSERVER_DEF int ftpserver_open(Ftp_Server *f, u64 number_of_clients,
				 str dir,
				 str username,
				 str password);
FTPSERVER_DEF void ftpserver_next(Ftp_Server *f,
				  Ip_Sockets *s,
				  u64 off,
				  u64 len,
				  Ip_Error error,
				  u64 index,
				  Ip_Mode mode);
FTPSERVER_DEF void ftpserver_close(Ftp_Server *f);

#ifdef FTPSERVER_IMPLEMENTATION

FTPSERVER_DEF int ftpserver_open(Ftp_Server *f,
				 u64 number_of_clients,
				 str dir,
				 str username,
				 str password) {

  f->number_of_clients = number_of_clients;
  f->sessions = FTPSERVER_ALLOC(sizeof(*f->sessions) * number_of_clients);
  if(!f->sessions) {
    return 0;
  }
  for(u64 i=0;i<number_of_clients;i++) {
    f->sessions[i].sb = (str_builder) {0};
    f->sessions[i].dir[0] = '.';
    f->sessions[i].dir[1] = FS_DELIM;
    f->sessions[i].dir_len = 2;
  }
  f->dir_base = dir;
  f->username = username;
  f->password = password;

  Ip ip;
  if(ip_get_address(ip) != IP_ERROR_NONE) {
    return 0;
  }

  u64 i = 0;
  while(ip[i]) {
    u8 c = ip[i];
    if(c == '.') {
      c = ',';
    }
    f->ip[i] = c;
    i++;
  }

  return 1;
}

FTPSERVER_DEF void ftpserver_next(Ftp_Server *f,
				  Ip_Sockets *_s,
				  u64 off,
				  u64 len,
				  Ip_Error error,
				  u64 index,
				  Ip_Mode mode) {

  switch(error) {
  case IP_ERROR_REPEAT:
    return;
  case IP_ERROR_NONE:
    // pass;
    break;
  default:
    // return FTPSERVER_EVENT_NONE;
    TODO();
  }

  Ip_Socket *socket = &_s->sockets[index];
  if(socket->flags & IP_SERVER) {

    if((index - off) == (len - 1)) {

      int found = 0;
      u64 client_index = 0;
      for(;client_index<len - 1;client_index++) {
	if(!(_s->sockets[off + client_index].flags & IP_VALID)) {
	  found = 1;
	  break;
	}
      }
      if(!found) {
	TODO();
      }

      Ip_Socket *client_socket = &_s->sockets[off + client_index];
      Ip_Address address;
      if(ip_socket_accept(socket, client_socket, &address) != IP_ERROR_NONE) {
	TODO();
      }
      if(ip_sockets_register(_s, off + client_index) != IP_ERROR_NONE) {
	      TODO();
      }
    
      Ftp_Server_Session *s = &f->sessions[client_index];
      s->response_kind = FTPSERVER_ACTION_KIND_MESSAGE;
      s->message = str_fromd("220 FTP Server ready.\r\n");
      s->sb.len = 0;
      s->request_len = 0;
      s->look_for_data_connection = 0;
      s->logged_in = 0;
      client_socket->flags |= IP_WRITING;
      return;
      
    } else {

      // session_index := relative index into 'f->sessions'
      u64 session_index = index - off - f->number_of_clients;
      // data_index := absolute index into 's->sockets'
      u64 data_index = index + f->number_of_clients;
                  
      Ip_Socket *client = &_s->sockets[data_index];
      Ip_Address address;
      if(ip_socket_accept(socket, client, &address) != IP_ERROR_NONE) {
	TODO();
      }
	if(ip_sockets_register(_s, data_index) != IP_ERROR_NONE) {
	      TODO();
      }


      Ftp_Server_Session *s = &f->sessions[session_index];
      if(s->look_for_data_connection && s->message.len == 0) {
	if(ftpserver_session_fill_for_data(s)) {
	  client->flags |= IP_WRITING;
	}
      }      
      
    }
    
  } else { // socket->flags & IP_CLIENT;

    int is_data_index = f->number_of_clients*2 <= (index - off);

    // session_index := relative index into 'f->sessions'
    // data_index := absolute index into 's->sockets'
    // text_index := absolute index into 's->sockets'
    u64 session_index, data_index, text_index;
    if(is_data_index) {
      session_index = (index - off) - 2*f->number_of_clients;
      data_index = index;
      text_index = index - 2*f->number_of_clients;
    } else {
      session_index = (index - off);
      data_index = index + 2*f->number_of_clients;
      text_index = index;
    }

    Ftp_Server_Session *s = &f->sessions[session_index];
    switch(mode) {
    case IP_MODE_READ: {
      
      int keep_reading = 1;
      while(keep_reading) {

	if(is_data_index) {
	  u64 read;
	  switch(ip_socket_read(socket,
				s->sb.data,
			        FTPSERVER_SESSION_WINDOW_SIZE,
				&read)) {
	  case IP_ERROR_REPEAT:
	    keep_reading = 0;
	    break;
	  case IP_ERROR_EOF:
	    if(ip_sockets_unregister(_s, index) != IP_ERROR_NONE) TODO();
	    ip_socket_close(socket);
	    _s->ret = -1;

	    fs_file_close(&s->file);
	    keep_reading = 0;
	    *socket = ip_socket_invalid();

	    s->response_kind = FTPSERVER_ACTION_KIND_MESSAGE;
	    s->message = str_fromd("226 Transfer complete\r\n");
	    s->sb.len = 0;
	    s->request_len = 0;
	    _s->sockets[text_index].flags |= IP_WRITING;	    
	    break;
	  case IP_ERROR_NONE: {
	    switch(s->response_kind) {
	    case FTPSERVER_ACTION_KIND_MESSAGE:
	    case FTPSERVER_ACTION_KIND_WRITE_FILE: {
	      // fs_file_close(file);
	      keep_reading = 0;
	      if(ip_sockets_unregister(_s, index) != IP_ERROR_NONE) TODO();
	      ip_socket_close(socket);
	      *socket = ip_socket_invalid();
	    } break;
	    case FTPSERVER_ACTION_KIND_READ_FILE: {

	      u64 written_total = 0;
	      while(written_total < read) {
		u64 written;
		switch(fs_file_write(&s->file,
				     s->sb.data + written_total,
				     read - written_total,
				     &written)) {
		case FS_ERROR_NONE:
		  written_total += written;
		  break;
		default:
		  TODO();
		}
		
	      }
	      
	      
	    } break;
	    default:
	      TODO();
	    }

	  } break;
	  default:
	    TODO();
	  }
	  
	} else{
	  u64 read;
	  switch(ip_socket_read(socket,
			      s->request + s->request_len,
			      sizeof(s->request) - s->request_len,
			      &read)) {
	  case IP_ERROR_REPEAT:
	    keep_reading = 0;
	    break;
	  case IP_ERROR_NONE:
	    if(sizeof(s->request) < s->request_len + read) {
	      TODO();
	    }
	    s->request_len += read;
	    break;
	  default:
	    *socket = ip_socket_invalid();
	    return;
	  }
	  
	}

	if(!keep_reading) {
	  break;
	}

	if(2 <= s->request_len &&	   
	   s->request[s->request_len - 2] == '\r' && 
	   s->request[s->request_len - 1] == '\n') {
	  u64 request_len = s->request_len - 2;
	  s->request_len = 0;
	  s->look_for_data_connection = 0;

	  str request = str_from(s->request, request_len);
	  printf("FTP [%llu/%llu] '"str_fmt"'\n", (index -  off), f->number_of_clients, str_arg(request)); fflush(stdout);

	  if(s->logged_in) {

	    if(str_eqc(request, "AUTH TLS") ||
	       str_eqc(request, "AUTH SSL")) {
	      s->message = str_fromd("530 Not logged in\r\n");	   
	    
	    } else if(str_eqc(request, "SYST") ||
		      str_eqc(request, "syst")) {
	      s->message = str_fromd("215 Windows_NT\r\n");
	    } else if(str_eqc(request, "FEAT")) {
	      s->message = str_fromd("211-Extensions supported\r\n211 End\r\n");
	    } else if(str_eqc(request, "PWD")) {
	      s->sb.len = 0;
	      str_builder_appendc(&s->sb, "257 \"");
	      str_builder_reserve(&s->sb, s->sb.len + s->dir_len - 1);
	      for(u64 i=1;i<s->dir_len;i++) {
		u8 c = s->dir[i];
		if(c == FS_DELIM) {
		  c = '/';
		}

		s->sb.data[s->sb.len++] = c;
	      }
	      str_builder_appendc(&s->sb, "\" is current directory\r\n");

	      s->message = str_from(s->sb.data, s->sb.len);
	    
	    } else if(str_eqc(request, "TYPE I")) {
	      s->message = str_fromd("200 Type set\r\n");

	    } else if(str_eqc(request, "TYPE A")) {
	      s->message = str_fromd("500 Type not supported\r\n");

	    } else if(str_index_ofc(request, "EPRT") == 0) {
		    s->message = str_fromd("500 This not supported\r\n");

	    } else if(str_eqc(request, "EPSV")) {
	      u16 port_to_use = 60000 - f->number_of_clients + (index - off);

	      s->sb.len = 0;
	      str_builder_appendf(&s->sb,
				  "229 Entering Extended Passive Mode (|||%u|)\r\n",
				  port_to_use);
	      s->message = str_from(s->sb.data, s->sb.len);

	      // TODO: an argument should enable/disable this behaviour
	      Ip_Socket *server = &_s->sockets[index + f->number_of_clients];
	      if(!(server->flags & IP_VALID)) {
		if(ip_socket_sopen(server, port_to_use, 1) != IP_ERROR_NONE) {
		  TODO();
		}
		if(ip_sockets_register(_s, index + f->number_of_clients) != IP_ERROR_NONE) {
			TODO();
		}
	      }
	    

	    } else if(str_eqc(request, "PASV")) {
	      u16 port_to_use = 60000 - f->number_of_clients + (index - off);

	      s->sb.len = 0;
	      str_builder_appendf(&s->sb,
				  "227 Entering Passive Mode (%s,%u,%u)\r\n",
				  f->ip,
				  port_to_use / 256,
				  port_to_use % 256);
	      s->message = str_from(s->sb.data, s->sb.len);

	      // TODO: an argument should enable/disable this behaviour
	      Ip_Socket *server = &_s->sockets[index + f->number_of_clients];
	      if(!(server->flags & IP_VALID)) {
		if(ip_socket_sopen(server, port_to_use, 1) != IP_ERROR_NONE) {
		  TODO();
		}
		if(ip_sockets_register(_s, index + f->number_of_clients) != IP_ERROR_NONE) {
			TODO();
		}
	      }
	    
	    } else if(str_eqc(request, "LIST")) {
	      s->sb.len = 0;

	      memcpy(f->path, f->dir_base.data, f->dir_base.len);
	      memcpy(f->path + f->dir_base.len, s->dir, s->dir_len);
	      str dirpath = str_from(f->path, f->dir_base.len + s->dir_len);
	    
	      Fs_Dir dir;
	      if(fs_dir_opens(&dir, dirpath) == FS_ERROR_NONE) {
		Fs_Dir_Entry entry;
		while(fs_dir_next(&dir, &entry) == FS_ERROR_NONE) {
		  if(entry.flags & FS_DIR_ENTRY_FROM_SYSTEM) {
		    continue;
		  }

		  u8 c;
		  if(entry.flags & FS_DIR_ENTRY_IS_DIR) {
		    c = 'd';
		  } else {
		    c = '-';
		  }

#if 1
		  str_builder_appendf(&s->sb,
			     "%crw-rw-rw- jschartner %8llu %02d-%02d-%04d %02d:%02d %s\r\n",
			     c,
			     entry.size,
			     entry.time.month,
			     entry.time.day,
			     entry.time.year,
			     entry.time.hour,
			     entry.time.min,
			     entry.name);

#else
		  str_builder_appendf(&s->sb,
				      "%02d.%02d.%02d  %02d:%02d    ",
				      entry.time.day,
				      entry.time.month,
				      entry.time.year,
				  
				      entry.time.hour,
				      entry.time.min);

		  if(entry.flags & FS_DIR_ENTRY_IS_DIR) {
		    str_builder_appendf(&s->sb, "<DIR>         ");
		  } else {
		    if(entry.size > 999999) {
		      str_builder_appendf(&s->sb,
					  "   %3d%03d%03d",
					  entry.size / 1000 / 1000,
					  (entry.size / 1000) % 1000,
					  entry.size % 1000);
		    } else if(entry.size > 999) {
		      str_builder_appendf(&s->sb,
					  "       %3d%03d",
					  (entry.size / 1000) % 1000,
					  entry.size % 1000);
      
		    } else {
		      str_builder_appendf(&s->sb,
					  "          %3d",
					  entry.size % 1000);
      
		    }
		  }
    
    

		  str_builder_appendf(&s->sb, " %s\r\n", entry.name);
#endif
	
		}
		if(dir.error == FS_ERROR_EOF) {
		  fs_dir_close(&dir);
		  s->data_kind = FTPSERVER_ACTION_KIND_MESSAGE;
		  s->look_for_data_connection = 1;
		
		  s->message = str_fromd("150 Opening data connection\r\n");
		} else {
		
		  s->message = str_fromd("500 Cannot list directory\r\n");
		}
	      
	      } else {
	      
		s->message = str_fromd("500 Cannot list directory\r\n");
	      }

	    } else if(str_index_ofc(request, "SIZE ") == 0) {

	      memcpy(f->path, f->dir_base.data, f->dir_base.len);
	      memcpy(f->path + f->dir_base.len, s->dir, s->dir_len);
	      memcpy(f->path + f->dir_base.len + s->dir_len, request.data + 5, request.len - 5);
	      str filepath = str_from(f->path, f->dir_base.len + s->dir_len + request.len - 5);
	    
	      if(fs_file_ropens(&s->file, filepath) == FS_ERROR_NONE) {
		u64 size = s->file.size;
		fs_file_close(&s->file);

		s->sb.len = 0;
		str_builder_appendf(&s->sb, "213 %llu\r\n", size);
		s->message = str_from(s->sb.data, s->sb.len);
	      } else {
		s->message = str_fromd("500 Cannot retrieve filesize\r\n");
	      }
	    
	    } else if(str_index_ofc(request, "RETR ") == 0) {
	      memcpy(f->path, f->dir_base.data, f->dir_base.len);
	      memcpy(f->path + f->dir_base.len, s->dir, s->dir_len);
	      memcpy(f->path + f->dir_base.len + s->dir_len, request.data + 5, request.len - 5);
	      str filepath = str_from(f->path, f->dir_base.len + s->dir_len + request.len - 5);

	      if(fs_file_ropens(&s->file, filepath) == FS_ERROR_NONE) {
		s->sb.len = 0;
		str_builder_reserve(&s->sb, FTPSERVER_SESSION_WINDOW_SIZE);
		s->data_kind = FTPSERVER_ACTION_KIND_WRITE_FILE;
		s->look_for_data_connection = 1;

		s->message = str_fromd("150 Opening data connection\r\n");
	      } else {

		s->message = str_fromd("500 Cannot open file for reading\r\n");
	      }	    
	    
	    } else if(str_index_ofc(request, "CWD ") == 0) {
	      str dir = str_from(request.data + 4, request.len - 4);

	      s->sb.len = 0;
	    
	      str_builder_reserve(&s->sb, FS_MAX_PATH);
	      if(dir.len > 0 && dir.data[0] == '/') {
		s->sb.data[s->sb.len++] = '.';
		s->sb.data[s->sb.len++] = FS_DELIM;
	      
	      } else {
		str_builder_append(&s->sb, s->dir, s->dir_len);
	      }
	      for(u64 i=0;i<dir.len;i++) {
		u8 c = dir.data[i];

		if(c == '/') {
		  c = FS_DELIM;
		} else {
		  // c = c
		}

		if(c == FS_DELIM &&
		   s->sb.data[s->sb.len - 1] == FS_DELIM) {
		  continue;
		}
		s->sb.data[s->sb.len++] = c;
	      }
	      if(s->sb.data[s->sb.len - 1] != FS_DELIM) {
		s->sb.data[s->sb.len++] = FS_DELIM;
	      }

	      memcpy(f->path, f->dir_base.data, f->dir_base.len);
	      memcpy(f->path + f->dir_base.len, s->sb.data, s->sb.len);
	      str dirpath = str_from(f->path, f->dir_base.len + s->sb.len);

	      int is_file;
	      if(fs_existss(dirpath, &is_file) && !is_file) {
		memcpy(s->dir, s->sb.data, s->sb.len);
		s->dir_len = s->sb.len;
		s->message = str_fromd("250 CWD command successful\r\n");
	      } else {
		s->message = str_fromd("500 Does not exists\r\n");
	      }
	      s->sb.len = 0;
	    
	    } else if(str_index_ofc(request, "DELE ") == 0) {
	      memcpy(f->path, f->dir_base.data, f->dir_base.len);
	      memcpy(f->path + f->dir_base.len, s->dir, s->dir_len);
	      memcpy(f->path + f->dir_base.len + s->dir_len, request.data + 5, request.len - 5);
	      str filepath = str_from(f->path, f->dir_base.len + s->dir_len + request.len - 5);
	    
	      if(fs_deletes(filepath) == FS_ERROR_NONE) {
		s->message = str_fromd("250 command successful\r\n");
	      } else {
		s->message = str_fromd("500 command was not successful\r\n");
	      }

	    } else if(str_eqc(request, "CDUP")) {
	      s->dir[0] = '.';
	      s->dir[1] = FS_DELIM;
	      s->dir_len = 2;
	    
	      s->message = str_fromd("250 command successful\r\n");

	    } else if(str_index_ofc(request, "STOR ") == 0) {
	      memcpy(f->path, f->dir_base.data, f->dir_base.len);
	      memcpy(f->path + f->dir_base.len, s->dir, s->dir_len);
	      memcpy(f->path + f->dir_base.len + s->dir_len, request.data + 5, request.len - 5);
	      str filepath = str_from(f->path, f->dir_base.len + s->dir_len + request.len - 5);

	      if(fs_file_wopens(&s->file, filepath) == FS_ERROR_NONE) {
		s->sb.len = 0;
		str_builder_reserve(&s->sb, FTPSERVER_SESSION_WINDOW_SIZE);
		s->data_kind = FTPSERVER_ACTION_KIND_READ_FILE;
		s->look_for_data_connection = 1;

		s->message = str_fromd("150 Opening data connection\r\n");
	      } else {

		s->message = str_fromd("500 Cannot store file\r\n");
	      }

	    } else if(str_index_ofc(request, "RNFR ") == 0) {
	      s->sb.len = 0;
	      str_builder_appendc(&s->sb, "RNFR ");
	      str_builder_appends(&s->sb, f->dir_base);
	      str_builder_append(&s->sb, s->dir, s->dir_len);
	      str_builder_append(&s->sb, request.data + 5, request.len - 5);

	      s->message = str_fromd("350 continue\r\n");
	    
	    } else if(str_index_ofc(request, "RNTO ") == 0) {
	      str last_request = str_from(s->sb.data, s->sb.len);

	      if(str_index_ofc(last_request, "RNFR ") != 0) {
		s->message = str_fromd("421 You did not send RNFR before\r\n");
	      } else {
		str from = str_from(last_request.data + 5, last_request.len - 5);
	      
		memcpy(f->path, f->dir_base.data, f->dir_base.len);
		memcpy(f->path + f->dir_base.len, s->dir, s->dir_len);
		memcpy(f->path + f->dir_base.len + s->dir_len, request.data + 5, request.len - 5);
		str to = str_from(f->path, f->dir_base.len + s->dir_len + request.len - 5);
	      
		if(fs_moves(from, to) == FS_ERROR_NONE) {
		  s->message = str_fromd("250 command successful\r\n");
		} else {
		  s->message = str_fromd("500 command was not successful\r\n");
		}
	      
	      }
	    
	    } else if(str_eqc(request, "opts utf8 on") ||
		      str_eqc(request, "noop") ||
		      str_eqc(request, "site help") ||
		      str_index_ofc(request, "PORT ") == 0 ||
		      str_index_ofc(request, "REST ") == 0) {
	      s->message = str_fromd("500 What?\r\n");

	    } else if(str_index_ofc(request, "MKD ") == 0) {
	      memcpy(f->path, f->dir_base.data, f->dir_base.len);
	      memcpy(f->path + f->dir_base.len, s->dir, s->dir_len);
	      memcpy(f->path + f->dir_base.len + s->dir_len, request.data + 4, request.len - 4);
	      str dir = str_from(f->path, f->dir_base.len + s->dir_len + request.len - 4);

	      if(fs_mkdirs(dir) == FS_ERROR_NONE) {
		s->message = str_fromd("250 command successful\r\n");
	      } else {
		s->message = str_fromd("500 command was not successful\r\n");
	      }

	    } else if(str_index_ofc(request, "RMD ") == 0) {
	      memcpy(f->path, f->dir_base.data, f->dir_base.len);
	      memcpy(f->path + f->dir_base.len, s->dir, s->dir_len);
	      memcpy(f->path + f->dir_base.len + s->dir_len, request.data + 4, request.len - 4);
	      str dir = str_from(f->path, f->dir_base.len + s->dir_len + request.len - 4);

	      if(fs_rmdirs(dir) == FS_ERROR_NONE) {
		s->message = str_fromd("250 command successful\r\n");
	      } else {
		s->message = str_fromd("500 command was not successful\r\n");
	      }

	    } else {
	      printf("'"str_fmt"'\n", str_arg(request)); fflush(stdout);
	      TODO();
	    }
	    
	  } else {

	    if(str_index_ofc(request, "USER ") == 0) {
	      str username = str_from(request.data + 5, request.len - 5);

	      if(str_eqc(username, "anonymous")) {
		s->message = str_fromd("530 Not logged in\r\n");
		// s->message = str_fromd("230 User logged in\r\n");
		// s->logged_in = 1;
		
	      } else if(str_eq(username, f->username)) {
		s->message = str_fromd("331 Password required for login\r\n");
	      
	      }

	    } else if(str_index_ofc(request, "PASS ") == 0) {
	      str password = str_from(request.data + 5, request.len - 5);

	      if(str_eq(password, f->password)) {
		s->message = str_fromd("230 User logged in\r\n");
		s->logged_in = 1;
	      } else {
		s->message = str_fromd("530 Not logged in\r\n");
	      }
	    } else {
	      s->message = str_fromd("530 Not logged in\r\n");  
	    }
	    
	    
	  }

	  s->response_kind = FTPSERVER_ACTION_KIND_MESSAGE;
	  socket->flags |= IP_WRITING;

	  break;
	}
	
      }

    } break;
    case IP_MODE_WRITE: {

      int keep_writing = 1;
      while(keep_writing) {

	switch(s->response_kind) {
	case FTPSERVER_ACTION_KIND_MESSAGE: {

	  if(s->message.len == 0) {
	    keep_writing = 0;
	    socket->flags &= ~IP_WRITING;

	    if(is_data_index) {
	      ip_socket_close(socket);
	      *socket = ip_socket_invalid();
	      
	      s->response_kind = FTPSERVER_ACTION_KIND_MESSAGE;
	      s->message = str_fromd("226 Transfer complete\r\n");
	      s->request_len = 0;
	      s->sb.len = 0;
	      _s->sockets[text_index].flags |= IP_WRITING;
	  
	      /* u64 server_index = f->index - f->number_of_clients; */
	      /* ip_socket_close(&f->ip_server.sockets[server_index]); */
	      /* ip_server_discard(&f->ip_server, server_index); */
	  
	    } else {

	      Ip_Socket *data_socket = &_s->sockets[data_index];
	      int data_index_connected = (data_socket->flags & IP_VALID);
	      
	      if(s->look_for_data_connection &&
		 data_index_connected) {
		if(ftpserver_session_fill_for_data(s)) {
		  data_socket->flags |= IP_WRITING;
		}
	      }
	      
	    }
	
	  }

	  if(!keep_writing) {
	    break;
	  }
	  
	  u64 written;
	  switch(ip_socket_write(socket,
				 s->message.data,
				 s->message.len,
				 &written)) {
	  case IP_ERROR_NONE:
	    s->message = str_from(s->message.data + written, s->message.len - written);
	    break;
	  case IP_ERROR_REPEAT:
	    keep_writing = 0;
	    break;
	  default:
	    TODO();
	    break;
	  }

	} break;

	case FTPSERVER_ACTION_KIND_WRITE_FILE: {
	  Fs_File *file = &s->file;

	  if(s->sb.len < FTPSERVER_SESSION_WINDOW_SIZE &&
	     file->pos < file->size) {

	    u64 read;
	    switch(fs_file_read(file,
				s->sb.data + s->sb.len,
			        FTPSERVER_SESSION_WINDOW_SIZE - s->sb.len,
				&read)) {
	    case FS_ERROR_NONE:
	      s->sb.len += read;
	      break;
	    default:
	      TODO();
	    }
	    
	  }

	  if(s->sb.len == 0) {
	    fs_file_close(file);
	    keep_writing = 0;
	    if(ip_sockets_unregister(_s, index) != IP_ERROR_NONE) TODO();
	    ip_socket_close(socket);
	    *socket = ip_socket_invalid();

	    s->response_kind = FTPSERVER_ACTION_KIND_MESSAGE;
	    s->message = str_fromd("226 Transfer complete\r\n");
	    s->request_len = 0;
	    _s->sockets[text_index].flags |= IP_WRITING;

	  } else {

	    u64 written;
	    switch(ip_socket_write(socket,
				   s->sb.data,
				   s->sb.len,
				   &written)) {
	    case IP_ERROR_NONE:
	      s->sb.len -= written;
	      memmove(s->sb.data, s->sb.data + written, s->sb.len);
	      break;
	    case IP_ERROR_REPEAT:
	      keep_writing = 0;
	      break;
	    default:
	      TODO();
	      break;
	    }

	  }

	  
	} break;

	
	  
	default: {
	  TODO();
	} break;
	}
      }

    } break;

    case IP_MODE_DISCONNECT: {
      *socket = ip_socket_invalid();
    } break;

    default: {
      TODO();
    } break;
    }
    
  }
  
}

FTPSERVER_DEF int ftpserver_session_fill_for_data(Ftp_Server_Session *s) {

  int result;

  switch(s->data_kind) {
  case FTPSERVER_ACTION_KIND_MESSAGE: {
    s->response_kind = FTPSERVER_ACTION_KIND_MESSAGE;
    s->message = str_from(s->sb.data, s->sb.len);
    result = 1;
	
  } break;
  case FTPSERVER_ACTION_KIND_WRITE_FILE: {
    s->response_kind = FTPSERVER_ACTION_KIND_WRITE_FILE;
    result = 1;
	
  } break;
  case FTPSERVER_ACTION_KIND_READ_FILE: {
    s->response_kind = FTPSERVER_ACTION_KIND_READ_FILE;
    result = 0;
    
  } break;
  default: {
    TODO(); // Is this correct ?
  } break;
  }
  
  if(s->look_for_data_connection) s->look_for_data_connection = 0;

  return result;
}

FTPSERVER_DEF void ftpserver_close(Ftp_Server *f) {  
  FTPSERVER_FREE(f->sessions);
}

#endif // FTPSERVER_IMPLEMENTATION

#endif // FTPSERVER_H
