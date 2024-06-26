#ifndef HTTP_H
#define HTTP_H

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

#include <string.h>
#include <stdlib.h>

typedef unsigned char Http_u8;
typedef int Http_s32;
typedef unsigned int Http_u32;
typedef long long Http_s64;
typedef unsigned long long Http_u64;
#define u8 Http_u8
#define s32 Http_s32
#define u32 Http_u32
#define s64 Http_s64
#define u64 Http_u64

#ifndef HTTP_DEF
#  define HTTP_DEF static inline
#endif // HTTP_DEF

#define HTTP_PORT 80
#define HTTPS_PORT 443

HTTP_DEF int http_parse_s64(u8 *data, u64 len, s64 *n);
HTTP_DEF int http_equals_ignorecase(u8 *data, u64 len, char *cstr);

#define HTTP_REQUEST_PAIR_IDLE 0
#define HTTP_REQUEST_PAIR_KEY 1
#define HTTP_REQUEST_PAIR_ALMOST_VALUE 2
#define HTTP_REQUEST_PAIR_VALUE 3

#define HTTP_REQUEST_STATE_IDLE 0
#define HTTP_REQUEST_STATE_R    1
#define HTTP_REQUEST_STATE_RN   2
#define HTTP_REQUEST_STATE_RNR  3
#define HTTP_REQUEST_STATE_RNRN 4

#define HTTP_REQUEST_BODY_NONE 0
#define HTTP_REQUEST_BODY_CONTENT_LEN 1
#define HTTP_REQUEST_BODY_CHUNKED 2

#define HTTP_METHODS_X				\
  HTTP_METHOD_X(GET)				\
       HTTP_METHOD_X(POST)			\
       HTTP_METHOD_X(DELETE)			\
       HTTP_METHOD_X(HEAD)			\
       HTTP_METHOD_X(PUT)			\
       HTTP_METHOD_X(PATCH)			\
       HTTP_METHOD_X(OPTIONS)			\
    

typedef enum {
  HTTP_METHOD_NONE = 0,
#define HTTP_METHOD_X(m) HTTP_METHOD_##m,
  HTTP_METHODS_X
#undef HTTP_METHOD_X
} Http_Method;

#define HTTP_DONE 0x1
#define HTTP_SET_BODY_CONTENT_LEN 0x2
#define HTTP_SET_BODY_CHUNKED 0x4
#define HTTP_FINISH_CHUNKED_BODY 0x8
#define HTTP_CHUNKED_SKIP_RN 0x10
#define HTTP_PROCESS_NOW 0x20

#define HTTP_HEX_CAP 16

typedef struct {
  s32 state;
  u32 pair;
  u32 body;
  s64 __content_length;

  s64 content_length;
  s32 response_code;
  Http_Method method;
  s32 flags;

  u8 *body_data;
  u64 body_len;

  u8 hex[HTTP_HEX_CAP];
  u64 hex_len;
} Http;

#define http_default() (Http) {			\
    .state = HTTP_REQUEST_STATE_IDLE,		\
      .pair = HTTP_REQUEST_PAIR_KEY,		\
      .body = HTTP_REQUEST_BODY_NONE,		\
      .content_length = 0,			\
      .response_code = 0,			\
      .method = HTTP_METHOD_NONE,		\
      .hex_len = 0,				\
      .flags = 0,				\
      .body_data = NULL,			\
      .body_len = 0,				\
      }

typedef enum {
  HTTP_EVENT_NOTHING = 0,  
  
  HTTP_EVENT_KEY   = HTTP_REQUEST_PAIR_KEY, // 1
  HTTP_EVENT_VALUE = HTTP_REQUEST_PAIR_VALUE, // 2

  HTTP_EVENT_ERROR,
  HTTP_EVENT_PROCESS,
  HTTP_EVENT_PATH,
  HTTP_EVENT_BODY,

} Http_Event;

HTTP_DEF Http_Event http_process(Http *h, u8 **data, u64 *len);
/* HTTP_DEF Http_Event http_process_prefix(Http *h); */
/* HTTP_DEF Http_Event http_process_header(Http *h); */

#ifdef HTTP_IMPLEMENTATION

static char *HTTP_METHOD_NAME[]= {
  [HTTP_METHOD_NONE] = "none",
#define HTTP_METHOD_X(n) [ HTTP_METHOD_##n ] = #n,
  HTTP_METHODS_X
#undef HTTP_METHOD_X
};

HTTP_DEF int http_parse_s64(u8 *data, u64 len, s64 *n) {
  if(len == 0) {
    return 0;
  }

  u64 i=0;
  s64 sum = 0;
  s32 negative = 0;

  if(len && data[0]=='-') {
    negative = 1;
    i++;
  }  
  while(i<len &&
	'0' <= data[i] && data[i] <= '9') {
    sum *= 10;
    s32 digit = data[i] - '0';
    sum += digit;
    i++;
  }

  if(negative) sum*=-1;
  *n = sum;
  
  return i==len;
}

HTTP_DEF int http_parse_hex_u64(u8 *buffer, u64 buffer_len, u64 *out) {

  u64 i = 0;
  u64 res = 0;

  while(i < buffer_len) {
    u8 c = buffer[i];

    res *= 16;
    if('0' <= c && c <= '9') {
      res += c - '0';
    } else if('a' <= c && c <= 'z') {
      res += c - 'W';
    } else if('A' <= c && c <= 'Z') {
      res += c - '7';
    } else {
      break;
    }
    i++;
  }

  *out = res;
  
  return i > 0 && i == buffer_len;
}

HTTP_DEF int http_equals_ignorecase(u8 *data, u64 len, char *cstr) {

  u64 i = 0;
  while(cstr[i] && i < len) {
    char a = (char) data[i];
    if('A' <= a && a <= 'Z') a += ' ';
    char b = cstr[i];
    if('A' <= b && b <= 'Z') b += ' ';
    if(a != b) return 0;    
    i++;
  }
  return !cstr[i] && i == len;

}


HTTP_DEF Http_Event __http_process_prefix(Http *h, u8 *data, u64 len) {
  
  u64 key_off = 0;
  for(u64 m=1;m<sizeof(HTTP_METHOD_NAME)/sizeof(HTTP_METHOD_NAME[0]);m++) {
    char *method_name = HTTP_METHOD_NAME[m];
    u64 method_name_len = strlen(method_name);
    if(len < method_name_len) continue;
    if(memcmp(data, method_name, method_name_len) != 0) continue;
    
    h->method = (Http_Method) m;
    key_off = method_name_len;
    break;
  }

  static char http1prefix[] = "HTTP/1.";
  static u64 http1prefix_len = sizeof(http1prefix) - 1;
  if(h->method != HTTP_METHOD_NONE) {

    if(key_off + 1 >= len ||
       data[key_off] != ' ') {
      return HTTP_EVENT_ERROR;
    }

    int found = 0;
    u64 j=key_off+1;
    for(;!found && j<len && http1prefix_len<=len-j;j++) {
      found = found || memcmp(data + j, http1prefix, http1prefix_len) == 0;
    }

    if(found) {
      h->body_data = data + key_off + 1;
      h->body_len = j - key_off - 3;

      return HTTP_EVENT_PATH;
    } else {
      return HTTP_EVENT_ERROR;
    }
	  
  } else {	  

    if(len < http1prefix_len ||
       memcmp(data, http1prefix, http1prefix_len) != 0) {
      return HTTP_EVENT_ERROR;
    }
	  
    s64 n;
    if(len - http1prefix_len <= 5 ||
       !http_parse_s64(data + http1prefix_len + 2, 3, &n)) {
      return HTTP_EVENT_ERROR;
    }
    h->response_code = (s32) n;

  }

  return HTTP_EVENT_NOTHING;
}


HTTP_DEF Http_Event __http_process_header(Http *h,
					  u8 *key, u64 key_len,
					  u8 *value, u64 value_len) {

  if(key_len == 0) {
    return HTTP_EVENT_ERROR;
  }

  if(value_len == 0) {
    return __http_process_prefix(h, key, key_len);
  }
  
  if(http_equals_ignorecase(key, key_len, "content-length")) {
    if(!http_parse_s64(value,
		       value_len,
		       &h->__content_length)) {
      return HTTP_EVENT_ERROR;
    }
    if(h->__content_length == 0) h->flags |= HTTP_DONE;
    h->flags |= HTTP_SET_BODY_CONTENT_LEN;
  }
  
  if(http_equals_ignorecase(key, key_len, "transfer-encoding") &&
     http_equals_ignorecase(value, value_len, "chunked")) {
    h->__content_length = -1;
    h->flags |= HTTP_SET_BODY_CHUNKED;
  }

  return HTTP_EVENT_NOTHING;
}

HTTP_DEF Http_Event http_process(Http *h, u8 **_data, u64 *_len) {

  if(h->flags & HTTP_PROCESS_NOW) {
    h->flags &= ~HTTP_PROCESS_NOW;
    h->hex_len = 0;
    return HTTP_EVENT_PROCESS;
  }

  u8 *data = *_data;
  u64 len = *_len;

  if(h->body == HTTP_REQUEST_BODY_CONTENT_LEN) {
    h->body_data = data;
    h->body_len  = len;
    if(h->body_len + h->content_length > (u64) h->__content_length) {
      h->body_len = (u64) h->__content_length - h->content_length;
    }
    h->content_length += h->body_len;
    if(h->content_length == h->__content_length) h->flags |= HTTP_DONE;
    
    *_data = *_data + h->body_len;
    *_len  = *_len  - h->body_len;
    return HTTP_EVENT_BODY;
  }
  
  if(h->body == HTTP_REQUEST_BODY_CHUNKED) {    
    if(h->__content_length > 0) {
      h->body_data = data;
      h->body_len  = len;
      if(h->body_len > (u64) h->__content_length) h->body_len = (u64) h->__content_length;
      h->content_length += h->body_len;
      h->__content_length -= h->body_len;
      
      h->state = HTTP_REQUEST_STATE_IDLE;
      *_data = *_data + h->body_len;
      *_len  = *_len  - h->body_len;
      return HTTP_EVENT_BODY;
    }
  }

  u64 header_start = len;
  u64 header_len = 1;
  u32 header = h->pair;
  if(h->pair == HTTP_REQUEST_PAIR_ALMOST_VALUE) {
    header = HTTP_REQUEST_PAIR_VALUE;
  }

  for(u64 i=0;i<len;i++) {
    u8 c = data[i];    

    switch(c) {
      
    case '\r': {
      switch(h->state) {
      case HTTP_REQUEST_STATE_IDLE:
	h->state = HTTP_REQUEST_STATE_R;
	break;
      case HTTP_REQUEST_STATE_R:
	h->state = HTTP_REQUEST_STATE_IDLE;
	break;
      case HTTP_REQUEST_STATE_RN:
	h->state = HTTP_REQUEST_STATE_RNR;
	break;	  
      case HTTP_REQUEST_STATE_RNR:
        h->state = HTTP_REQUEST_STATE_IDLE;
	break;
      case HTTP_REQUEST_STATE_RNRN: // Not sure
	h->state = HTTP_REQUEST_STATE_R;
	break;	
      default:
        return HTTP_EVENT_ERROR;
      }     
    } break;
      
    case '\n': {
      switch(h->state) {
      case HTTP_REQUEST_STATE_IDLE:
	h->state = HTTP_REQUEST_STATE_IDLE;
	break;
      case HTTP_REQUEST_STATE_R:
	h->state = HTTP_REQUEST_STATE_RN;
	break;
      case HTTP_REQUEST_STATE_RN:
	h->state = HTTP_REQUEST_STATE_IDLE;
	break;	  
      case HTTP_REQUEST_STATE_RNR:
        h->state = HTTP_REQUEST_STATE_RNRN;
	break;
      case HTTP_REQUEST_STATE_RNRN:
	h->state = HTTP_REQUEST_STATE_IDLE;
	break;
      default:
        return HTTP_EVENT_ERROR;
      }      
    } break;

    case ':': {
      if(h->pair == HTTP_REQUEST_PAIR_KEY) {
	h->pair = HTTP_REQUEST_PAIR_ALMOST_VALUE;
      }
    } break;
    case ' ': {
      if(h->pair == HTTP_REQUEST_PAIR_ALMOST_VALUE) {
	h->pair = HTTP_REQUEST_PAIR_VALUE;
      }
    } break;
      
    default: {
      h->state = HTTP_REQUEST_STATE_IDLE;
    } break;
    }

    switch(h->body) {
    case HTTP_REQUEST_BODY_NONE: {

      switch(h->state) {

      case HTTP_REQUEST_STATE_IDLE: {
	// HTTP_REQUEST_BODY_NONE
	// HTTP_REQUEST_STATE_IDLE
	//     collect Headers

	switch(h->pair) {
	case HTTP_REQUEST_PAIR_KEY: {

	  if(header_start == len) {
	    header_start = i;
	    header = HTTP_REQUEST_PAIR_KEY;
	  } else {
	    if(header == HTTP_REQUEST_PAIR_VALUE) {
	      u64 dv = (h->hex_len == 0) * 1;
	      
	      h->body_data = data + header_start + dv;
	      h->body_len  = header_len - dv;
	      *_data = *_data + i;
	      *_len = *_len - i;

	      h->hex_len += h->body_len;
	      
	      return (Http_Event) (h->body_len > 0) * HTTP_EVENT_VALUE;
	    } else {
	      header_len++;
	      
	    }
	    
	  }
	} break;

	case HTTP_REQUEST_PAIR_VALUE: {
	  if(header_start == len) {
	    header_start = i;
	    header = HTTP_REQUEST_PAIR_VALUE;
	  } else {
	    if(header == HTTP_REQUEST_PAIR_KEY) {
	      h->body_data = data + header_start;
	      h->body_len  = header_len;
	      *_data = *_data + i;
	      *_len = *_len - i;
	      return HTTP_EVENT_KEY;
	    } else {
	      header_len++;
	      
	    }
	    
	  }

	} break;
	}
	
      } break;

      case HTTP_REQUEST_STATE_RN: {
	// HTTP_REQUEST_BODY_NONE
	// HTTP_REQUEST_STATE_RN
	//     perform events/ parse Headers

	h->pair = HTTP_REQUEST_PAIR_KEY;

	if(header_start < len) {
	  h->flags |= HTTP_PROCESS_NOW;

	  u64 is_value = (header == HTTP_REQUEST_PAIR_VALUE);
	  u64 dv = is_value * (h->hex_len == 0) * 1;
	  
	  h->body_data = data + header_start + dv;
	  h->body_len  = header_len - dv;
	  *_data = *_data + (i + 1);
	  *_len = *_len - (i + 1);

	  h->hex_len += is_value * h->body_len;	  
	  return (Http_Event) ((h->body_len > 0) * header);
	} else {
	  *_data = *_data + (i + 1);
	  *_len = *_len - (i + 1);
	  return HTTP_EVENT_PROCESS;
	}
	
      } break;

      case HTTP_REQUEST_STATE_RNRN: {
	// HTTP_REQUEST_BODY_NONE
	// HTTP_REQUEST_STATE_RNRN
	//     move to body or finalize
	
	if(h->flags & HTTP_SET_BODY_CONTENT_LEN) {
	  h->flags &= ~HTTP_SET_BODY_CONTENT_LEN;
	  h->body = HTTP_REQUEST_BODY_CONTENT_LEN;
	} else if(h->flags & HTTP_SET_BODY_CHUNKED) {
	  h->flags &= ~HTTP_SET_BODY_CHUNKED;
	  h->body = HTTP_REQUEST_BODY_CHUNKED;
	} else {
	  h->flags |= HTTP_DONE;
	}
	
	*_data = *_data + i + 1;
	*_len = *_len - (i + 1);
	h->hex_len = 0;
	return HTTP_EVENT_NOTHING;
      } break;
	
      }
      
    } break;

    case HTTP_REQUEST_BODY_CONTENT_LEN: {

      // HTTP_REQUEST_BODY_CONTENT_LEN
      //     process data, no matter what
      h->body_data = data;
      h->body_len  = 1;
      h->content_length += h->body_len;
      if(h->content_length == h->__content_length) h->flags |= HTTP_DONE;
      
      *_data = *_data + h->body_len;
      *_len  = *_len  - h->body_len;
      return HTTP_EVENT_BODY;
      
    } break;

    case HTTP_REQUEST_BODY_CHUNKED: {

      if(h->__content_length > 0) {
	// HTTP_REQUEST_BODY_CHUNKED
	// h->__content_length > 0
	// !(h->flags & HTTP_CHUNKED_SKIP_RN)
	//     process data, no matter what

	h->body_data = data;
	h->body_len  = 1;
	
	h->content_length += h->body_len;
	h->__content_length -= h->body_len;
	
	*_data = *_data + h->body_len;
	*_len  = *_len  - h->body_len;
	return HTTP_EVENT_BODY;
	
      } else if(h->__content_length < 0) {

	switch(h->state) {

	case HTTP_REQUEST_STATE_IDLE: {
	  // HTTP_REQUEST_BODY_CHUNKED
	  // h->__content_length < 0
	  // HTTP_REQUEST_STATE_IDLE
	  //     collect hex data, called 'length of a chunk'

	  if(h->hex_len < HTTP_HEX_CAP) h->hex[h->hex_len++] = c;
	  
	  
	} break;

	case HTTP_REQUEST_STATE_RN: {
	  // HTTP_REQUEST_BODY_CHUNKED
	  // h->__content_length < 0
	  // HTTP_REQUEST_STATE_RN
	  //     parse 'length of a chunk' chunk

	  u64 n;
	  if(!http_parse_hex_u64(h->hex, h->hex_len, &n)) {
	    return HTTP_EVENT_ERROR;
	  }
	  h->hex_len = 0;
	  if(n == 0) {
	    h->flags |= HTTP_FINISH_CHUNKED_BODY;
	  }
	  h->__content_length = (s64) n;

	  *_data = *_data + i + 1;
	  *_len = *_len - (i + 1);
	  return HTTP_EVENT_NOTHING;
	} break;
	  
	}
	
      } else { // h->__content_length = 0

        if(h->flags & HTTP_FINISH_CHUNKED_BODY) {
	  
	  if(h->state == HTTP_REQUEST_STATE_RNRN) {
	    // HTTP_REQUEST_BODY_CHUNKED
	    // h->__content_length == 0
	    // h->flags & HTTP_FINISH_CHUNKED_BODY
	    // h->state == HTTP_REQUEST_STATE_RNRN
	    //     finalize
	  
	    h->flags |= HTTP_DONE;
	  }
	  
	} else {

	  if(h->state == HTTP_REQUEST_STATE_RN) {
	    // HTTP_REQUEST_BODY_CHUNKED
	    // h->__content_length == 0
	    // !(h->flags & HTTP_FINISH_CHUNKED_BODY)
	    // h->state == HTTP_REQUEST_STATE_RN
	    //     allow parsing of the next 'length of a chunk'
	  
	    h->__content_length = -1;
	  }
	  	  	  
	}
	
      }
      
    } break;
      
    }
  }

  *_len = 0;
  if(header_start < len) {
    u64 is_value = (header == HTTP_REQUEST_PAIR_VALUE);
    u64 dv = is_value * (h->hex_len == 0) * 1;

    h->body_data = data + header_start + dv;
    h->body_len  = header_len - dv;

    h->hex_len += is_value * h->body_len;	  
    return (Http_Event) ((h->body_len > 0) * header);
  } else {
    return HTTP_EVENT_NOTHING;
  }
 
}


#endif // HTTP_IMPLEMENTATION

#undef u8
#undef s32
#undef u32
#undef s64
#undef u64

#endif // HTTP_H
