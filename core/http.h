#ifndef HTTP_H
#define HTTP_H

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

HTTP_DEF int http_parse_s64(u8 *data, u64 len, s64 *n);
HTTP_DEF int http_equals_ignorecase(u8 *data, u64 len, char *cstr);

#define HTTP_REQUEST_PAIR_INVALID 0
#define HTTP_REQUEST_PAIR_KEY 1
#define HTTP_REQUEST_PAIR_ALMOST_VALUE 2
#define HTTP_REQUEST_PAIR_VALUE 3
#define HTTP_REQUEST_PAIR_VALUE_RESET 4

#define HTTP_REQUEST_STATE_DONE (-1)
#define HTTP_REQUEST_STATE_IDLE 0
#define HTTP_REQUEST_STATE_R    1
#define HTTP_REQUEST_STATE_RN   2
#define HTTP_REQUEST_STATE_RNR  3
#define HTTP_REQUEST_STATE_RNRN 4

#define HTTP_REQUEST_BODY_NONE 0
#define HTTP_REQUEST_BODY_CONTENT_LEN 1
#define HTTP_REQUEST_BODY_CHUNKED 2
#define HTTP_REQUEST_BODY_INFO 3

#define HTTP_BUF_CAP 64

typedef enum {
  HTTP_METHOD_NONE   = 0,
  HTTP_METHOD_GET    = 1,
  HTTP_METHOD_POST   = 2,
  HTTP_METHOD_DELETE = 3,
} Http_Method;

#define HTTP_DONE 0x1
#define HTTP_SET_BODY_CONTENT_LEN 0x2
#define HTTP_SET_BODY_CHUNKED 0x4
#define HTTP_FINISH_CHUNKED_BODY 0x8
#define HTTP_CHUNKED_SKIP_RN 0x10

typedef struct {
  s32 state, state2;
  u32 pair;
  u32 body;
  s64 __content_length;

  s64 content_length;
  s32 response_code;
  Http_Method method;
  s32 flags;

  u8 *body_data;
  u64 body_len;

  u8 key[HTTP_BUF_CAP];
  u64 key_len;
  u8 value[HTTP_BUF_CAP];
  u64 value_len;
} Http;

#define http_default() (Http) {			\
    .state = HTTP_REQUEST_STATE_IDLE,		\
      .state2 = HTTP_REQUEST_STATE_IDLE,	\
      .pair = HTTP_REQUEST_PAIR_KEY,		\
      .body = HTTP_REQUEST_BODY_NONE,		\
      .content_length = 0,			\
      .response_code = 0,			\
      .method = HTTP_METHOD_NONE,		\
      .key_len = 0,				\
      .value_len = 0,				\
      .flags = 0,				\
      .body_data = NULL,			\
      .body_len = 0,				\
      }

typedef enum {
  HTTP_EVENT_ERROR = 0,
  HTTP_EVENT_NOTHING,
  HTTP_EVENT_PATH,
  HTTP_EVENT_HEADER,
  HTTP_EVENT_BODY,
} Http_Event;

HTTP_DEF Http_Event http_process(Http *h, u8 **data, u64 *len);
HTTP_DEF Http_Event http_process_prefix(Http *h);
HTTP_DEF Http_Event http_process_header(Http *h);

#ifdef HTTP_IMPLEMENTATION

static char *HTTP_METHOD_NAME[]= {
  [HTTP_METHOD_NONE] = "none",
  [HTTP_METHOD_GET] = "GET",
  [HTTP_METHOD_POST] = "POST",
  [HTTP_METHOD_DELETE] = "DELETE",
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

HTTP_DEF Http_Event http_process_prefix(Http *h) {
  
  u64 key_off = 0;
  for(u64 m=1;m<sizeof(HTTP_METHOD_NAME)/sizeof(HTTP_METHOD_NAME[0]);m++) {
    char *method_name = HTTP_METHOD_NAME[m];
    u64 method_name_len = strlen(method_name);
    if(h->key_len < method_name_len) continue;
    if(memcmp(h->key, method_name, method_name_len) != 0) continue;
    
    h->method = (Http_Method) m;
    key_off = method_name_len;
    break;
  }

  static char http1prefix[] = "HTTP/1.";
  static u64 http1prefix_len = sizeof(http1prefix) - 1;
  if(h->method != HTTP_METHOD_NONE) {

    if(key_off + 1 >= h->key_len ||
       h->key[key_off] != ' ') {
      return HTTP_EVENT_ERROR;
    }

    int found = 0;
    u64 j=key_off+1;
    for(;!found && j<h->key_len && http1prefix_len<=h->key_len-j;j++) {
      found = found || memcmp(h->key + j, http1prefix, http1prefix_len) == 0;
    }

    if(found) {
      h->value_len = j - key_off - 3;
      memcpy(h->value, h->key + key_off + 1, h->value_len);

      return HTTP_EVENT_PATH;
    } else {
      return HTTP_EVENT_ERROR;
    }
	  
  } else {	  

    if(h->key_len < http1prefix_len ||
       memcmp(h->key, http1prefix, http1prefix_len) != 0) {	    
      return HTTP_EVENT_ERROR;
    }
	  
    s64 n;
    if(h->key_len - http1prefix_len <= 5 ||
       !http_parse_s64(h->key + http1prefix_len + 2, 3, &n)) {
      return HTTP_EVENT_ERROR;
    }
    h->response_code = (s32) n;

  }

  return HTTP_EVENT_NOTHING;
}

HTTP_DEF Http_Event http_process_header(Http *h) {
  
  if(http_equals_ignorecase(h->key, h->key_len, "content-length")) {
    if(!http_parse_s64(h->value,
		       h->value_len,
		       &h->__content_length)) {
      return HTTP_EVENT_ERROR;
    }
    if(h->__content_length == 0) h->flags |= HTTP_DONE;
    h->flags |= HTTP_SET_BODY_CONTENT_LEN;
  }
  
  if(http_equals_ignorecase(h->key, h->key_len, "transfer-encoding") &&
     http_equals_ignorecase(h->value, h->value_len, "chunked")) {
    h->__content_length = -1;
    h->flags |= HTTP_SET_BODY_CHUNKED;
  }

  return HTTP_EVENT_HEADER;
}

HTTP_DEF Http_Event http_process(Http *h, u8 **_data, u64 *_len) {

  if(h->pair == HTTP_REQUEST_PAIR_VALUE_RESET) {
    h->key_len = 0;
    h->value_len = 0;
    h->pair = HTTP_REQUEST_PAIR_VALUE;
  }

  u8 *data = *_data;
  u64 len = *_len;

  if(h->body == HTTP_REQUEST_BODY_CONTENT_LEN) {
    h->body_data = data;
    h->body_len  = len;
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

  for(u64 i=0;i<len;i++) {
    u32 state_before = h->state;
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
      if(h->pair == HTTP_REQUEST_PAIR_KEY) h->pair = HTTP_REQUEST_PAIR_ALMOST_VALUE;
    } break;
    case ' ': {
      if(h->pair == HTTP_REQUEST_PAIR_ALMOST_VALUE) h->pair = HTTP_REQUEST_PAIR_VALUE;
    } break;
      
    default: {
      h->state = HTTP_REQUEST_STATE_IDLE;
    } break;
    }

    if(h->state == HTTP_REQUEST_STATE_IDLE &&
       state_before == HTTP_REQUEST_STATE_RN) {
      h->pair = HTTP_REQUEST_PAIR_KEY;
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
	  if(h->key_len < HTTP_BUF_CAP) h->key[h->key_len++] = c;
	} break;
	
	case HTTP_REQUEST_PAIR_VALUE: {
	  if(h->value_len < HTTP_BUF_CAP) {
	    if(h->value_len == 0) h->value_len = 1;
	    else {
	      if(c == ' ' && h->value_len == 1) ; //ignore
	      else h->value[h->value_len++ - 1] = c;
	    }
	  }
	} break;
	}
	
      } break;

      case HTTP_REQUEST_STATE_RN: {
	// HTTP_REQUEST_BODY_NONE
	// HTTP_REQUEST_STATE_RN
	//     perform events/ parse Headers

	switch(h->pair) {
	case HTTP_REQUEST_PAIR_KEY: {
	  Http_Event event = http_process_prefix(h);
	  h->key_len = 0;
	
	  if(event != HTTP_EVENT_NOTHING) {
	    *_data = *_data + i + 1;
	    *_len = *_len - (i + 1);
	    return event;
	  }
	} break;
	
	case HTTP_REQUEST_PAIR_VALUE: {
	  if(h->value_len > 0) h->value_len -= 1;

	  Http_Event event = http_process_header(h);
	  *_data = *_data + i + 1;
	  *_len = *_len - (i + 1);

	  // Clear h->key_len and h->value_len, after event is processed
	  h->pair = HTTP_REQUEST_PAIR_VALUE_RESET;
	  
	  return event;
	
	} break;
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
	h->key_len = 0;
	h->value_len = 0;
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

	  if(h->key_len < HTTP_BUF_CAP) h->key[h->key_len++] = c;
	} break;

	case HTTP_REQUEST_STATE_RN: {
	  // HTTP_REQUEST_BODY_CHUNKED
	  // h->__content_length < 0
	  // HTTP_REQUEST_STATE_RN
	  //     parse 'length of a chunk' chunk

	  u64 n;
	  if(!http_parse_hex_u64(h->key, h->key_len, &n)) {
	    return HTTP_EVENT_ERROR;
	  }
	  h->key_len = 0;
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
  return HTTP_EVENT_NOTHING;
}


#endif // HTTP_IMPLEMENTATION

#undef u8
#undef s32
#undef u32
#undef s64
#undef u64

#endif // HTTP_H
