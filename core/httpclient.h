#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

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

#ifdef HTTPCLIENT_IMPLEMENTATION
#  define STR_IMPLEMENTATION
#  define IP_IMPLEMENTATION
#  define HTTP_IMPLEMENTATION
#endif // HTTPCLIENT_IMPLEMENTATION

#define HTTPCLIENT_HEADERS_PAIR_DELIM "|"
#define HTTPCLIENT_HEADERS_KEY_VALUE_DELIM ":"

#include <core/str.h>
#include <core/ip.h>
#include <core/http.h>
#include <core/types.h>

#ifndef HTTPCLIENT_DEF
#  define HTTPCLIENT_DEF static inline
#endif // HTTPCLIENT_DEF

HTTPCLIENT_DEF int httpclient_parse_url(str url,
					str *hostname,
					str *route,
					u16 *port,
					int *encrypted);

typedef str Http_Client_Headers;

typedef struct {
  s32 code;
  Http_Client_Headers headers;
  str body;
} Http_Client_Response;

HTTPCLIENT_DEF int httpclient_request(str url,
				      Http_Method method,
				      str_builder *sb,
				      Http_Client_Response *r);

#ifdef HTTPCLIENT_IMPLEMENTATION

// http(s)://%hostname%:%port%%route%
HTTPCLIENT_DEF int httpclient_parse_url(str url,
					str *hostname,
					str *route,
					u16 *port,
					int *encrypted) {
  if(url.len == 0) return 0;
  
  str http = str_fromd("http://");
  str https = str_fromd("https://");
  if(str_index_of(url, http) == 0) {
    url = str_from(url.data + http.len, url.len - http.len);
    *encrypted = 0;
  } else if(str_index_of(url, https) == 0) {
    url = str_from(url.data + https.len, url.len - https.len);
    *encrypted = 1;
  } else {
    return 0;
  }
  if(url.len == 0) return 0;
  
  s32 port_delim = str_index_ofc(url, ":");
  if(port_delim < 0) {
    if(*encrypted) {
      *port = HTTPS_PORT;
    } else {
      *port = HTTP_PORT;
    }
    
    s32 route_indicator = str_index_ofc(url, "/");
    if(route_indicator < 0) {
      *hostname = url;
      *route = str_fromd("/");
      
    } else {
      u64 _route_indicator = (u64) route_indicator;
      *hostname = str_from(url.data, _route_indicator);
      url = str_from(url.data + _route_indicator + 1, url.len - _route_indicator - 1);

      *route = url;
    }
    
    
  } else {
    u64 _port_delim = (u64) port_delim;
    *hostname = str_from(url.data, _port_delim);
    url = str_from(url.data + _port_delim + 1, url.len - _port_delim - 1);

    s32 route_indicator = str_index_ofc(url, "/");
    if(route_indicator < 0) {
      str port_string = url;

      s64 n;
      if(!str_parse_s64(port_string, &n) || n < 0) return 0;
      *port = (u16) n;
      
      *route = str_fromd("/");
      
    } else {      
      u64 _route_indicator = (u64) route_indicator;
      str port_string = str_from(url.data, route_indicator);
      url = str_from(url.data + _route_indicator, url.len - _route_indicator);
      
      s64 n;
      if(!str_parse_s64(port_string, &n) || n < 0) return 0;
      *port = (u16) n;
	
      *route = url;
      
    }
    
  }

  if(hostname->len == 0 || route->len == 0) return 0;
  
  return 1;
}

HTTPCLIENT_DEF int httpclient_request(str url,
				      Http_Method method,
				      str_builder *sb,
				      Http_Client_Response *r) {
  u16 port;
  str hostname, route;
  int encrypted;
  if(!httpclient_parse_url(url, &hostname, &route, &port, &encrypted)) {
    return 0;
  }
  if(encrypted) {
    TODO();
  }
  
  u64 sb_len = sb->len;  
  str_builder_appends(sb, hostname);
  str_builder_append(sb, "\0", 1);
  char *_hostname = sb->data + sb_len;
  sb->len = sb_len;
  
  Ip_Socket socket;
  if(ip_socket_copen(&socket, _hostname, port) != IP_ERROR_NONE) {
    return 0;
  }

  str_builder_appendf(sb, "%s / HTTP/1.1\r\n"
		      "Host: "str_fmt"\r\n"
		      "\r\n",
		      HTTP_METHOD_NAME[method],
		      str_arg(hostname));
  str request_string = str_from(sb->data + sb_len, sb->len - sb_len);
  u64 written;
  if(ip_socket_writes(&socket, request_string, &written) != IP_ERROR_NONE ||
     written != request_string.len) {
    TODO();
  }
  sb->len = sb_len;

  u64 _header, _value, _body;
  _header = sb_len;
  _value = 0;
  _body = 0;
  
  u8 buffer[1024];
  Http http = http_default();
  while(!(http.flags & HTTP_DONE)) {

    u64 read;
    if(ip_socket_read(&socket, buffer, sizeof(buffer), &read) != IP_ERROR_NONE) {
      TODO();
    }
    u8 *buf = buffer;
    u64 len = read;

    while(!(http.flags & HTTP_DONE) && len > 0) {
      switch(http_process(&http, &buf, &len)) {
      case HTTP_EVENT_ERROR:
	TODO();
      case HTTP_EVENT_KEY:
        str_builder_append(sb, HTTPCLIENT_HEADERS_PAIR_DELIM, (u64) (sb->len == _header));
	str_builder_append(sb, http.body_data, http.body_len);
	_value = sb->len;
	break;
      case HTTP_EVENT_VALUE:
	str_builder_append(sb, HTTPCLIENT_HEADERS_KEY_VALUE_DELIM, (u64) (sb->len == _value));
	str_builder_append(sb, http.body_data, http.body_len);
	break;
      case HTTP_EVENT_BODY:
	str_builder_append(sb, http.body_data, http.body_len);
	break;
      case HTTP_EVENT_PROCESS:
	str key = str_from(sb->data + _header + 1, _value - _header - 1);
	str value = str_from(sb->data + _value + 1, sb->len - _value);
	value.len -= (value.len > 0);

	/* printf("key: '"str_fmt"'\n", str_arg(key)); */
	/* printf("value: '"str_fmt"'\n", str_arg(value)); */
	/* printf("'"str_fmt"'\n", str_arg(str_from(sb->data + sb_len, sb->len - sb_len))); fflush(stdout); */

	switch(__http_process_header(&http,
				     key.data, key.len,
				     value.data, value.len)) {
	case HTTP_EVENT_ERROR:
	  TODO();
	case HTTP_EVENT_PATH:
	  UNREACHABLE();
	case HTTP_EVENT_NOTHING:
	  break;
	default:
	  UNREACHABLE();
	  break;
	}

	_header = sb->len;
	_body = sb->len;
	
	break;
      default:
	break;
      }
    }    
    
  }

  r->headers = str_from(sb->data + sb_len, _body - sb_len);
  r->body = str_from(sb->data + _body, sb->len - _body + sb_len);

  r->code = http.response_code;
  ip_socket_close(&socket);
  
  return 1;
}

#endif // HTTPCLIENT_IMPLEMENTATION

#endif // HTTPCLIENT_H
