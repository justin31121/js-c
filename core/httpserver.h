#ifndef HTTPSERVER_H
#define HTTPSERVER_H

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

#ifndef HTTPSERVER_DEF
#  define HTTPSERVER_DEF static inline
#endif // HTTPSERVER_DEF

#ifndef HTTPSERVER_ALLOC
#  include <stdlib.h>
#  define HTTPSERVER_ALLOC malloc
#endif // HTTPSERVER_ALLOC

#ifndef HTTPSERVER_FREE
#  include <stdlib.h>
#  define HTTPSERVER_FREE free
#endif // HTTPSERVER_FREE

#ifdef HTTPSERVER_IMPLEMENTATION
#  define STR_IMPLEMENTATION
#  define IP_IMPLEMENTATION
#  define FS_IMPLEMENTATION
#  define HTTP_IMPLEMENTATION
#  define B64_IMPLEMENTATION
#  define VA_IMPLEMENTATION
#endif // HTTPSERVER_IMPLEMENTATION

#include <core/str.h>
#include <core/ip.h>
#include <core/fs.h>
#include <core/http.h>
#include <core/b64.h>
#include <core/va.h>
#include <core/types.h>

#define HTTPSERVER_SOCKETS_PER_CLIENT 1

typedef enum {
	HTTPSERVER_WRITE_KIND_FIXED,
	HTTPSERVER_WRITE_KIND_FILE,
	HTTPSERVER_WRITE_KIND_FILE_CHUNKED,
} Http_Server_Write_Kind;

typedef struct {
	str message;
	u64 off;
} Http_Server_Write_Fixed;

#define HTTPSERVER_WRITE_FILE_CHUNKED_LEN (2 << 13)
#define HTTPSERVER_WRITE_FILE_QUEUE_DATA_CAP 32
#define HTTPSERVER_SB_BUFFER_SIZE 1024

typedef struct {
	Fs_File file;
	u64 to_write;
	u8 queue_off;
	u8 queue_len;
	u8 queue_data[HTTPSERVER_WRITE_FILE_QUEUE_DATA_CAP];
} Http_Server_Write_File_Chunked;

typedef struct {
	Http_Server_Write_Kind kind;
	union {
		Http_Server_Write_Fixed fixed;
		Fs_File file;
		Http_Server_Write_File_Chunked chunked;
	} as;
} Http_Server_Write;

#define HTTPSERVER_WRITE_CAP 8

typedef struct {
	// State of http-request
	Http http;

	// Memory for headers/path/body/response-headers
	str_builder sb;
	u64 path_len, _header, _value, _body;

	// Enqueued writes
	Http_Server_Write queue[HTTPSERVER_WRITE_CAP];
	u64 queue_pos;
	u64 queue_len;

	// // buffer
	// u8 buf[1024];
	// u64 len;
	u64 off;
	u64 len;
	int started_to_write;

	u64 inactive_cycles;
} Http_Server_Session;

#define httpserver_session_enqueue(s, w)				\
	do {									\
		if((s)->queue_len >= HTTPSERVER_WRITE_CAP) {			\
			TODO();								\
		}									\
		(s)->queue[((s)->queue_pos + (s)->queue_len++) % HTTPSERVER_WRITE_CAP] = (w); \
	}while(0)

#define httpserver_enqueue_fixed(s, m) httpserver_session_enqueue((s), ((Http_Server_Write) { \
			.kind = HTTPSERVER_WRITE_KIND_FIXED,				\
			.as.fixed = (Http_Server_Write_Fixed) { .message = (m), .off = 0, } }))

#define httpserver_enqueue_file(s, f) httpserver_session_enqueue((s), ((Http_Server_Write) { \
			.kind = HTTPSERVER_WRITE_KIND_FILE,				\
			.as.file = (f) }))

#define httpserver_enqueue_file_chunked(s, f) httpserver_session_enqueue((s), ((Http_Server_Write) { \
			.kind = HTTPSERVER_WRITE_KIND_FILE_CHUNKED,			\
			.as.chunked = (Http_Server_Write_File_Chunked) {		\
			.file = (f),							\
			.queue_off = HTTPSERVER_WRITE_FILE_QUEUE_DATA_CAP,		\
			}}))

typedef str Http_Server_Headers;

#define HTTPSERVER_HEADERS_PAIR_DELIM "|"
#define HTTPSERVER_HEADERS_KEY_VALUE_DELIM ":"

typedef struct {
	Http_Method method;
	str path;
	str params;
	str body;
	Http_Server_Headers headers;
} Http_Server_Request;

HTTPSERVER_DEF int httpserver_headers_find(Http_Server_Headers headers, u8 *name, u64 name_len, str *value);
#define httpserver_headers_findc(hs, cstr, v) httpserver_headers_find((hs), (cstr), strlen(cstr), (v))
#define httpserver_headers_finds(hs, s, v) httpserver_headers_find((hs), (s).data, (s).len, (v))

typedef struct {
	Http_Server_Session *sessions;
	u64 number_of_clients;

	u8 ip_buf[1024];
} Http_Server;

HTTPSERVER_DEF int httpserver_open(Http_Server *h, u64 number_of_clients);
HTTPSERVER_DEF int httpserver_next(Http_Server *h,
		Ip_Sockets *s,
		u64 off,
		u64 len,
		Ip_Error error,
		u64 index,
		Ip_Mode mode,
		Http_Server_Request *r);
HTTPSERVER_DEF void httpserver_close(Http_Server *h);

///////////////////////////////////////////////////////////////////////////////////////////

HTTPSERVER_DEF int httpserver_is_authenticated(Http_Server_Session *s,
		Http_Server_Request *r,
		str username,
		str password,
		str_builder *sb);

HTTPSERVER_DEF void httpserver_serve_files(Http_Server_Session *s,
		str dir,
		Http_Server_Request *r,
		str_builder *sb);
HTTPSERVER_DEF void httpserver_serve_files_get(Http_Server_Session *s,
		str dir,
		Http_Server_Request *r,
		str_builder *sb);
HTTPSERVER_DEF void httpserver_serve_files_head(Http_Server_Session *s,
		str dir,
		Http_Server_Request *r,
		str_builder *sb);
HTTPSERVER_DEF int httpserver_open_file(Http_Server_Session *s,
		str path,
		Fs_File *file);
HTTPSERVER_DEF int httpserver_translate_path(Http_Server_Session *s,
		str dir,
		str raw_path,
		str_builder *sb,
		str *out_path);
HTTPSERVER_DEF char *httpserver_guess_content_type(str path);

// HTTPSERVER_DEF str httpserver_snprintf(Http_Server_Session *s, char *fmt, ...);

#define httpserver_snprintf2(s, fmt, ...) httpserver_snprintf2_impl((s), (fmt), va_catch(__VA_ARGS__))
HTTPSERVER_DEF str httpserver_snprintf2_impl(Http_Server_Session *s, char *fmt, Va *vas, u64 vas_len);

#ifdef HTTPSERVER_IMPLEMENTATION

HTTPSERVER_DEF int httpserver_headers_find(Http_Server_Headers headers, u8 *name, u64 name_len, str *value) {
	str key = str_from(name, name_len);

	str key_value;
	while(str_chop_by(&headers, HTTPSERVER_HEADERS_PAIR_DELIM, &key_value)) {

		str potential_key;
		while(str_chop_by(&key_value, HTTPSERVER_HEADERS_KEY_VALUE_DELIM, &potential_key)) {

			if(str_eq_ignorecase(key, potential_key)) {
				*value = key_value;
				return 1;
			}

		}

	}

	return 0;
}

HTTPSERVER_DEF int httpserver_open(Http_Server *h, u64 number_of_clients) {

	h->sessions = HTTPSERVER_ALLOC(sizeof(*h->sessions) * number_of_clients);
	if(!h->sessions) {
		return 0;
	}
	for(u64 i=0;i<number_of_clients;i++) {
		h->sessions[i].sb = (str_builder) {0};
	}
	h->number_of_clients = number_of_clients;

	return 1;
}

HTTPSERVER_DEF int httpserver_next(Http_Server *h,
		Ip_Sockets *_s,
		u64 off,
		u64 len,

		Ip_Error error,
		u64 index,
		Ip_Mode mode,

		Http_Server_Request *r) {

	switch(error) {
		case IP_ERROR_REPEAT:
		case IP_ERROR_NONE:
			// pass;
			break;
		default:
			printf("error: %d\n", error);
			TODO();
	}

	for(u64 i=0;i<len - 1;i++) {
		Ip_Socket *socket = &_s->sockets[off + i];
		if(!(socket->flags & IP_VALID)) {
			continue;
		}

		Http_Server_Session *session = &h->sessions[i];
		if(session->inactive_cycles >= 512) {
			ip_sockets_unregister(_s, i);
			ip_socket_close(socket);
			*socket = ip_socket_invalid();
			_s->ret = -1;

		} else {
			session->inactive_cycles += 1;
		}

	}

	if(error == IP_ERROR_REPEAT) {
		return 0;
	}

	Ip_Socket *socket = &_s->sockets[index];
	if(socket->flags & IP_SERVER) {

		int found = 0;
		u64 client_index = 0;
		while(!found && client_index<len) {
			Ip_Socket *socket = &_s->sockets[off + client_index];
			if(socket->flags & IP_VALID) {
				client_index++;
			} else {
				found = 1;
			}
		}
		if(!found) {
			TODO();
		}

		Ip_Address address;
		if(ip_socket_accept(socket, 
					&_s->sockets[off + client_index],
					&address) != IP_ERROR_NONE) {
			TODO();	
		}
		ip_sockets_register(_s, off + client_index);

		Http_Server_Session *s = &h->sessions[client_index];
		s->http = http_default();
		s->sb.len = 0;
		s->path_len = 0;
		s->_header = 0;
		s->_value = 0;
		s->_body = 0;

		s->started_to_write = 0;
		s->inactive_cycles = 0;
		return 0;

	} else { // socket->flags & IP_CLIENT

		Http_Server_Session *s = &h->sessions[index - off];
		s->inactive_cycles = 0;

		switch(mode) {
			case IP_MODE_READ: {
						   int keep_reading = s->queue_len == 0; // TODO: this may not work on linux with 'epfd'
						   while(keep_reading) {

							   u64 read;
							   switch(ip_socket_read(socket, h->ip_buf, sizeof(h->ip_buf), &read)) {
								   case IP_ERROR_NONE:
									   break;
								   case IP_ERROR_REPEAT:
									   keep_reading = 0;
									   break;
								   case IP_ERROR_CONNECTION_CLOSED:
								   case IP_ERROR_CONNECTION_ABORTED:
									   *socket = ip_socket_invalid();
									   _s->ret = 1;
									   keep_reading = 0;
									   return 0;
								   default:
									   TODO();
									   break;
							   }

							   if(!keep_reading) {
								   break;
							   }
							   int bad_request = 0;

							   u8 *buf = h->ip_buf;
							   u64 buf_len = read;

							   while(!bad_request && buf_len > 0) {
								   switch(http_process(&s->http, &buf, &buf_len)) {
									   case HTTP_EVENT_ERROR: {
													  bad_request = 1;
												  } break;
									   case HTTP_EVENT_KEY: {
													str_builder_append(&s->sb, HTTPSERVER_HEADERS_PAIR_DELIM, (u64) (s->sb.len == s->_header));
													str_builder_append(&s->sb, s->http.body_data, s->http.body_len);
													s->_value = s->sb.len;
												} break;
									   case HTTP_EVENT_VALUE: {
													  str_builder_append(&s->sb, HTTPSERVER_HEADERS_KEY_VALUE_DELIM, (u64) (s->sb.len == s->_value));
													  str_builder_append(&s->sb, s->http.body_data, s->http.body_len);
												  } break;
									   case HTTP_EVENT_BODY: {
													 str_builder_append(&s->sb, s->http.body_data, s->http.body_len);
												 } break;
									   case HTTP_EVENT_PROCESS: {
													    str key = str_from(s->sb.data + s->_header + 1, s->_value - s->_header - 1);
													    str value = str_from(s->sb.data + s->_value + 1, s->sb.len - s->_value);
													    value.len -= (value.len > 0);

													    switch(__http_process_header(&s->http,
																    key.data, key.len,
																    value.data, value.len)) {
														    case HTTP_EVENT_ERROR:
															    bad_request = 1;
															    break;
														    case HTTP_EVENT_PATH:
															    s->sb.len = 0;
															    str_builder_append(&s->sb, s->http.body_data, s->http.body_len);
															    s->path_len = s->sb.len;
															    break;
														    case HTTP_EVENT_NOTHING:
															    break;
														    default:
															    UNREACHABLE();
															    break;
													    }

													    s->_header = s->sb.len;
													    s->_body = s->sb.len;
												    } break;
									   case HTTP_EVENT_NOTHING: {
													    // repeat
												    } break;
									   default: {
											    UNREACHABLE();
										    } break;

								   }
							   }

							   if(!((s->http.flags & HTTP_DONE) || bad_request)) {
								   continue;
							   }
							   keep_reading = 0;

							   // sb.data: '%path%%body%'
							   //                 ^
							   //                 start
							   r->method = s->http.method;
							   r->params = str_from(s->sb.data, s->path_len);
							   str_chop_by(&r->params, "?", &r->path);
							   r->body = str_from(s->sb.data + s->_body, s->sb.len - s->_body);
							   r->headers = str_from(s->sb.data, s->_body);

							   printf("HTTP [%llu/%llu] '"str_fmt"'\n", (index -  off), h->number_of_clients, str_arg(r->path));

							   s->len = s->sb.cap;
							   return 1;
						   }

					   } break;

			case IP_MODE_WRITE: {

						    if(s->queue_len == 0) {
							    UNREACHABLE();
						    }

						    int keep_writing = 1;
						    int disconnected = 0;
						    while(keep_writing && s->queue_len > 0) {
							    Http_Server_Write *w = &s->queue[s->queue_pos];

							    switch(w->kind) {
								    case HTTPSERVER_WRITE_KIND_FIXED: {
													      Http_Server_Write_Fixed *fixed = &w->as.fixed;

													      if(fixed->off < fixed->message.len) {

														      u64 written;
														      switch(ip_socket_write(socket,
																	      fixed->message.data + fixed->off,
																	      fixed->message.len - fixed->off,
																	      &written)) {
															      case IP_ERROR_NONE:
																      fixed->off += written;
																      break;
															      case IP_ERROR_REPEAT:
																      keep_writing = 0;
																      break;
															      default:
																      TODO();
																      break;
														      }
													      }

													      if(fixed->off == fixed->message.len) {
														      s->queue_pos = (s->queue_pos + 1) % HTTPSERVER_WRITE_CAP;
														      s->queue_len--;
													      }

												      } break;
								    case HTTPSERVER_WRITE_KIND_FILE: {

													     if(!s->started_to_write) {
														     s->off = s->sb.len;
														     s->len = 0;
														     str_builder_reserve(&s->sb, s->sb.len + HTTPSERVER_SB_BUFFER_SIZE);
														     s->started_to_write = 1;
													     }

													     Fs_File *file = &w->as.file;

													     u64 can_read = file->size - file->pos;
													     // if there are bytes to read and
													     // there is space inside 's->buf' =>
													     // read from 'file' to 's->buf'
													     if(can_read > 0 &&
															     s->len < (s->sb.cap - s->off)) {

														     u64 read;
														     switch(fs_file_read(file,
																	     s->sb.data + s->off + s->len,
																	     s->sb.cap - s->off - s->len,
																	     &read)) {
															     case FS_ERROR_NONE:
																     break;
															     case FS_ERROR_EOF:
																     break;
															     default:
																     TODO();
																     /* disconnected = 1; */
																     /* keep_writing = 0; */
																     /* s->queue_len = 0; */
																     /* fs_file_close(file); */
																     break;
														     }
														     s->len += read;
													     }

													     if(!keep_writing) {
														     break;
													     }

													     // if 's->buf' is empty, 'file' is fully transmitted
													     // otherwise write 's->buf'
													     if(s->len > 0) {

														     u64 written;
														     Ip_Error _error = ip_socket_write(socket,
																     s->sb.data + s->off,
																     s->len,
																     &written);
														     switch(_error) {
															     case IP_ERROR_NONE:
																     s->len -= written;
																     memmove(s->sb.data + s->off, s->sb.data + s->off + written, s->len);
																     break;
															     case IP_ERROR_REPEAT:
																     keep_writing = 0;
																     break;
															     case IP_ERROR_CONNECTION_ABORTED:
																     keep_writing = 0;
																     *socket = ip_socket_invalid();
																     s->queue_len = 0;
																     fs_file_close(file);
																     _s->ret = -1;
																     break;
															     default:
																     // *socket = ip_socket_invalid();
																     // keep_writing = 0;
																     // s->queue_len = 0;
																     // fs_file_close(file);
																     // break;
																     printf("error: %d\n", _error);
																     TODO();
														     }

													     } else { // s->len == 0
														     s->queue_pos = (s->queue_pos + 1) % HTTPSERVER_WRITE_CAP;
														     s->queue_len--;
														     fs_file_close(file);

													     }
												     } break;

								    case HTTPSERVER_WRITE_KIND_FILE_CHUNKED: {

														     if(!s->started_to_write) {
															     s->off = s->sb.len;
															     s->len = 0;
															     str_builder_reserve(&s->sb, s->sb.len + HTTPSERVER_SB_BUFFER_SIZE);
															     s->started_to_write = 1;
														     }


														     Http_Server_Write_File_Chunked *chunked = &w->as.chunked;
														     Fs_File *file = &chunked->file;

														     if(chunked->queue_off == HTTPSERVER_WRITE_FILE_QUEUE_DATA_CAP) {
															     // Constructor

															     u64 remaining = file->size - file->pos;
															     if(remaining < HTTPSERVER_WRITE_FILE_CHUNKED_LEN) {
																     chunked->to_write = remaining;
															     } else {
																     chunked->to_write = HTTPSERVER_WRITE_FILE_CHUNKED_LEN;
															     }

															     chunked->queue_len =
																     (u8) snprintf((char *) chunked->queue_data,
																		     HTTPSERVER_WRITE_FILE_QUEUE_DATA_CAP,
																		     "%llx\r\n",
																		     chunked->to_write);
															     chunked->queue_off = 0;
														     }

														     while((chunked->queue_len > 0 || (file->pos < file->size)) &&
																     (s->len < (s->sb.cap - s->off))) {

															     while((chunked->queue_len > 0) && (s->len < (s->sb.cap - s->off))) {
																     s->sb.data[s->off + s->len++] = chunked->queue_data[chunked->queue_off];
																     chunked->queue_off++;
																     chunked->queue_len--;
															     }

															     if((s->len == (s->sb.cap - s->off)) ||
																	     (file->pos == file->size)) {
																     break;
															     }

															     u64 can_write = (s->sb.cap - s->off) - s->len;
															     u64 to_read;
															     if(chunked->to_write > can_write) {
																     to_read = can_write;
															     } else {
																     to_read = chunked->to_write;
															     }

															     u64 read;
															     switch(fs_file_read(file,
																		     s->sb.data + s->off + s->len,
																		     to_read,
																		     &read)) {
																     case FS_ERROR_NONE:
																     case FS_ERROR_EOF:
																	     break;
																     default:
																	     TODO();
															     }
															     s->len += read;
															     chunked->to_write -= read;

															     if(chunked->to_write == 0) {

																     u64 remaining = file->size - file->pos;
																     if(remaining < HTTPSERVER_WRITE_FILE_CHUNKED_LEN) {
																	     chunked->to_write = remaining;
																     } else {
																	     chunked->to_write = HTTPSERVER_WRITE_FILE_CHUNKED_LEN;
																     }

																     char *fmt;
																     if(chunked->to_write == 0) {
																	     fmt = "\r\n%llx\r\n\r\n";
																     } else {
																	     fmt = "\r\n%llx\r\n";
																     }

																     chunked->queue_len =
																	     (u8) snprintf((char *) chunked->queue_data,
																			     HTTPSERVER_WRITE_FILE_QUEUE_DATA_CAP,
																			     fmt,
																			     chunked->to_write);
																     chunked->queue_off = 0;
															     }

														     }

														     if(s->len > 0) {
															     u64 written;
															     switch(ip_socket_write(socket,
																		     s->sb.data + s->off,
																		     s->len,
																		     &written)) {
																     case IP_ERROR_NONE:
																	     memmove(s->sb.data + s->off, s->sb.data + s->off + written, s->len - written);
																	     s->len -= written;
																	     break;
																     case IP_ERROR_REPEAT:
																	     keep_writing = 0;
																	     break;
																     case IP_ERROR_CONNECTION_ABORTED:
																	     keep_writing = 0;
																	     *socket = ip_socket_invalid();
																	     s->queue_len = 0;
																	     fs_file_close(file);
																	     _s->ret = -1;
																	     break;
																     default:
																	     TODO();
																	     /* keep_writing = 0; */
																	     /* s->queue_len = 0; */
																	     /* fs_file_close(file); */
																	     break;
															     }


														     } else { // s->len == 0
															     s->queue_pos = (s->queue_pos + 1) % HTTPSERVER_WRITE_CAP;
															     s->queue_len--;
															     fs_file_close(file);

														     }

													     } break;

								    default: {
										     TODO();
									     } break;

							    }
						    }

						    if(s->queue_len == 0) {
							    s->http = http_default();
							    s->sb.len = 0;
							    s->path_len = 0;
							    s->_header = 0;
							    s->_value = 0;
							    s->_body = 0;

							    s->started_to_write = 0;
							    socket->flags &= ~IP_WRITING;
						    }

						    if(disconnected) {
							    return 0;
						    }

					    } break;

			default:
					    TODO();
		}

	}


	return 0;
}

HTTPSERVER_DEF void httpserver_close(Http_Server *h) {
	for(u64 i=0;i<h->number_of_clients;i++) {
		STR_FREE(h->sessions[i].sb.data);
	}
	HTTPSERVER_FREE(h->sessions);
}

HTTPSERVER_DEF int httpserver_is_authenticated(Http_Server_Session *s,
		Http_Server_Request *r,
		str username,
		str password,
		str_builder *sb) {
	int authenticated;
	str authorization;
	if(httpserver_headers_findc(r->headers, "Authorization", &authorization)) {

		if(!str_chop_by(&authorization, " ", NULL)) {
			authenticated = 0;
		} else {
			u64 len = base64_decode(sb->data,
					sb->cap,
					authorization.data, authorization.len);
			if(sb->cap < len) {
				str_builder_reserve(sb, len);
				base64_decode(sb->data + sb->len,
						sb->cap - sb->len,
						authorization.data, authorization.len);
			}

			str password = str_from(sb->data, len);
			str username;
			if(str_chop_by(&password, ":", &username)) {
				authenticated =
					str_eqc(username, "admin") &&
					str_eqc(password, "nimda");
			} else {
				authenticated = 0;
			}

		}

	} else {
		authenticated = 0;
	}

	if(!authenticated) {
		if(r->method == HTTP_METHOD_HEAD) {
			httpserver_enqueue_fixed(s, str_fromd("HTTP/1.1 401 Unauthorized\r\n"
						"WWW-Authenticate: Basic realm=\"User Visible Realm\"\r\n"
						"\r\n"));
		} else {
			httpserver_enqueue_fixed(s, str_fromd("HTTP/1.1 401 Unauthorized\r\n"
						"WWW-Authenticate: Basic realm=\"User Visible Realm\"\r\n"
						"Content-Type: text/plain\r\n"
						"Content-Length: 12\r\n"
						"\r\n"
						"Unauthorized"));
		}
		return 0 ;
	}

	return 1;
}

HTTPSERVER_DEF void httpserver_serve_files(Http_Server_Session *s,
		str dir,
		Http_Server_Request *r,
		str_builder *sb) {

	switch(r->method) {
		case HTTP_METHOD_GET:
			httpserver_serve_files_get(s, dir, r, sb);
			break;
		case HTTP_METHOD_HEAD:
			httpserver_serve_files_head(s, dir, r, sb);
			break;
		default:
			httpserver_enqueue_fixed(s, str_fromd("HTTP/1.1 501 Not Implemented\r\n"
						"Content-Type: text/plain\r\n"
						"Content-Length: 15\r\n"
						"\r\n"
						"Not Implemented"));
			break;
	}

}

HTTPSERVER_DEF void httpserver_serve_files_get(Http_Server_Session *s,
		str dir,
		Http_Server_Request *r,
		str_builder *sb) {

	str path;
	if(!httpserver_translate_path(s,
				dir,
				r->path,
				sb,
				&path)) {
		return;
	}

	Fs_File file;
	if(!httpserver_open_file(s,
				path,
				&file)) {
		return;
	}
	char *content_type = httpserver_guess_content_type(path);

	Va vas[2];
	vas[0] = va_n(file.size);
	vas[1] = va_c(content_type);
	httpserver_enqueue_fixed(s, httpserver_snprintf2_impl(s,
				"HTTP/1.1 200 OK\r\n"
				"Content-Length: %\r\n"
				"Content-Type: %\r\n"
				"\r\n",
				vas, 2));
	httpserver_enqueue_file(s, file);  

	/* Va va = va_c(content_type); */
	/* httpserver_enqueue_fixed(s, httpserver_snprintf2_impl(s, "HTTP/1.1 200 OK\r\n" */
	/* 							"Transfer-Encoding: chunked\r\n" */
	/* 							"Content-Type: %\r\n" */
	/* 							"\r\n", */
	/* 							&va, 1)); */
	/* httpserver_enqueue_file_chunked(s, file); */


}

HTTPSERVER_DEF void httpserver_serve_files_head(Http_Server_Session *s,
		str dir,
		Http_Server_Request *r,
		str_builder *sb) {

	str path;
	if(!httpserver_translate_path(s,
				dir,
				r->path,
				sb,
				&path)) {
		return;
	}

	Fs_File file;
	if(!httpserver_open_file(s,
				path,
				&file)) {
		return;
	}


	char *content_type = httpserver_guess_content_type(path);

	Va vas[2];
	vas[0] = va_n(file.size);
	vas[1] = va_c(content_type);

	httpserver_enqueue_fixed(s, httpserver_snprintf2_impl(s, "HTTP/1.1 200 OK\r\n"
				"Content-Length: %\r\n"
				"Content-Type: %\r\n"
				"\r\n",
				vas, 2));
	fs_file_close(&file);

}

// - create file handle inside 'file' specified by 'path'
// - on error write to 'Http_Server'
HTTPSERVER_DEF int httpserver_open_file(Http_Server_Session *s,
		str path,
		Fs_File *file) {

	switch(fs_file_ropens(file, path)) {
		case FS_ERROR_NONE:
			// caller of 'open_file' now owns the file
			return 1;
		case FS_ERROR_FILE_NOT_FOUND:
		case FS_ERROR_ACCESS_DENIED:
			httpserver_enqueue_fixed(s, str_fromd("HTTP/1.1 404 Not Found\r\n"
						"Content-Length: 9\r\n"
						"Content-Type: text/plain\r\n"
						"\r\n"
						"Not Found"));
			return 0;
		default:
			httpserver_enqueue_fixed(s, str_fromd("HTTP/1.1 500 Internal Server Error\r\n"
						"Content-Length: 21\r\n"
						"Content-Type: text/plain\r\n"
						"\r\n"
						"Internal Server Error"));
			return 0;
	}

}

// - translate 'raw_path' into actual 'path'
// - map '/' to 'index.html' and disallow '/..'
// - maybe short-circuit and write error to 'Http_Server'
// - allocate 'path' inside 'sb_temp'
HTTPSERVER_DEF int httpserver_translate_path(Http_Server_Session *s,
		str dir,
		str raw_path,
		str_builder *sb,
		str *out_path) {
	if(str_index_ofc(raw_path, "/..") >= 0) {
		httpserver_enqueue_fixed(s, str_fromd("HTTP/1.1 405 Not Allowed\r\n"
					"Content-Length: 11\r\n"
					"Content-Type: text/plain\r\n"
					"\r\n"
					"Not Allowed"));
		return 0;
	}

	if(str_eqc(raw_path, "/")) {
		raw_path = str_fromd("/index.html");
	}

	str_builder_reserve(sb, sb->len + dir.len + raw_path.len);
	u64 sb_len = sb->len;

	memcpy(sb->data + sb->len, dir.data, dir.len);
	sb->len += dir.len;

	for(u64 i=0;i<raw_path.len;i++) {
		u8 c = raw_path.data[i];

		if(c == '/') {
			c = FS_DELIM;
		} else {
			// c = c
		}

		sb->data[sb->len++] = c;
	}

	*out_path = str_from(sb->data + sb_len, sb->len - sb_len);

	return 1;
}

// - guess content_type by potential file-extension
// - by default return 'application/octet-stream'
HTTPSERVER_DEF char *httpserver_guess_content_type(str path) {
	if(path.len == 0) return "application/octet-stream";

	u64 i = path.len - 1;
	while(i > 0 && path.data[i] != '.') i--;

	str maybe_extension = str_from(path.data + i, path.len - i);
	if(maybe_extension.len == 0) return "application/octet-stream";

	char *content_type;
	if(str_eqc(maybe_extension, ".html")) {
		content_type = "text/html";
	} else if(str_eqc(maybe_extension, ".txt")) {
		content_type = "text/plain";
	} else {
		content_type = "application/octet-stream";
	}

	return content_type;

}

// HTTPSERVER_DEF str httpserver_snprintf(Http_Server_Session *s, char *fmt, ...) {
//   str_builder *sb = &s->sb;
//
//   u64 sb_len = sb->len;
//   u64 available = sb->cap - sb->len;
//
//   va_list list;
//   va_start(list, fmt);
//   u64 len = vsnprintf((char *) (sb->data + sb->len), available, fmt, list);
//   va_end(list);
//
//   if(len + 1 > available) {
//     va_start(list, fmt);
//     str_builder_reserve(sb, sb->len + len + 1);
//     vsnprintf((char *) (sb->data + sb->len), available, fmt, list);
//     va_end(list);
//   }
//   sb->len += len;
//
//   return str_from(sb->data + sb_len, sb->len - sb_len);
// }

HTTPSERVER_DEF str httpserver_snprintf2_impl(Http_Server_Session *s, char *_fmt, Va *vas, u64 vas_len) {

	str fmt = str_fromc(_fmt);

	str_builder *sb = &s->sb;
	u64 sb_len = sb->len;

	if(!va_appendf_impl(sb, fmt, vas, vas_len)) {
		TODO();
	}

	return str_from(sb->data + sb_len, sb->len - sb_len);
}

#endif // HTTPSERVER_IMPLEMENTATION

#endif // HTTPSERVER_H
