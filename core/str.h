#ifndef _STR_H
#define _STR_H

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

#include <stdbool.h>
#include <stdarg.h>

typedef unsigned char str_u8;
typedef unsigned int str_u32;
typedef int str_s32;
typedef long long str_s64;
typedef unsigned long long str_u64;
typedef double str_f64;

#define u8 str_u8
#define u32 str_u32
#define s32 str_s32
#define s64 str_s64
#define u64 str_u64
#define f64 str_f64

#ifndef STR_DEF
#  define STR_DEF static inline
#endif // STR_DEF

#ifndef STR_ASSERT
#  include <assert.h>
#  define STR_ASSERT assert
#endif // STR_ASSERT

#ifndef STR_ALLOC
#  include <stdlib.h>
#  define STR_ALLOC malloc
#endif // STR_ALLOC

#ifndef STR_FREE
#  include <stdlib.h>
#  define STR_FREE free
#endif // STR_FREE

STR_DEF u64 str_cstrlen(u8 *cstr);
STR_DEF s32 str_memcmp(const void *a, const void *b, u64 len);
STR_DEF void *str_memcpy(void *dst, const void *src, u64 len);

typedef struct{
  u8 *data;
  u64 len;
}str;

#define str_fmt "%.*s"
#define str_arg(s) (int) (s).len, (s).data

#define str_from(d, l) (str) { .data = (d), .len = (l) }
#define str_fromc(cstr) str_from((str_u8 *) (cstr), str_cstrlen((str_u8 *) (cstr)))
#define str_fromd(cstr) str_from((str_u8 *) (cstr), (sizeof((cstr)) - 1))

STR_DEF bool str_eq(str a, str b);
#define str_eqc(s, cstr) str_eq((s), str_fromc(cstr))
STR_DEF bool str_eq_ignorecase(str a, str b);
#define str_eq_ignorecasec(s, cstr) str_eq_ignorecase((s), str_fromc(cstr))
STR_DEF bool str_parse_s64(str s, s64 *n);
STR_DEF bool str_parse_f64(str s, f64 *n);

STR_DEF s32 str_index_of(str s, str needle);
STR_DEF s32 str_index_of_off(str s, u64 off, str needle);
#define str_index_ofc(s, cstr) str_index_of((s), str_fromc(cstr))
#define str_index_of_offc(s, off, cstr) str_index_of_off((s), (off), str_fromc(cstr))

STR_DEF bool str_chop_by(str *s, char *delim, str *d);

#ifndef STR_BUILDER_DEFAULT_CAP
#  define STR_BUILDER_DEFAULT_CAP 1024
#endif // STR_BUILDER_DEFAULT_CAP

typedef struct{
  u8 *data;
  u64 len;
  u64 cap;
}str_builder;

STR_DEF void str_builder_reserve(str_builder *sb, u64 cap);

STR_DEF void str_builder_append(str_builder *sb, const u8 *data, u64 len);
#define str_builder_appendc(sb, cstr) \
  str_builder_append((sb), (str_u8 *) (cstr), str_cstrlen((str_u8 *) (cstr)))
#define str_builder_appends(sb, s) str_builder_append((sb), (s).data, (s).len)
STR_DEF void str_builder_appends64(str_builder *sb, s64 n);
STR_DEF void str_builder_appendf(str_builder *sb, char *fmt, ...);

typedef u32 Rune;

STR_DEF Rune rune_decode(const u8 **data, u64 *data_len);
STR_DEF void rune_encode(Rune rune, u8 buf[4], u64 *buf_len);
STR_DEF Rune rune_unescape(u8 **data, u64 *data_len);
STR_DEF Rune rune_decode(const u8 **data, u64 *data_len);

#ifdef STR_IMPLEMENTATION

STR_DEF u64 str_cstrlen(u8 *cstr) {
  u64 len = 0;
  while(*cstr) {
    len++;
    cstr++;
  }
  return len;
}

STR_DEF s32 str_memcmp(const void *a, const void *b, u64 len) {
  const u8 *pa = a;
  const u8 *pb = b;

  s32 d = 0;
  while(!d && len) {
    d = *pa++ - *pb++;
    len--;
  }

  return d;
}

STR_DEF void *str_memcpy(void *_dst, const void *_src, u64 len) {
  u8 *dst = _dst;
  const u8 *src = _src;
  for(u64 i=0;i<len;i++) {
    *dst++ = *src++;
  }
  return dst;
}

//////////////////////////////////////////////////////////////////////////////////////////

STR_DEF bool str_eq(str a, str b) {
  if(a.len != b.len) {
    return false;
  }
  return str_memcmp(a.data, b.data, a.len) == 0;
}

STR_DEF bool str_eq_ignorecase(str a, str b) {
  if(a.len != b.len) {
    return false;
  }
  for(u64 i=0;i<a.len;i++) {
    u8 p = a.data[i];
    u8 q = b.data[i];
    if('A' <= p && p <= 'Z') p += ' ';
    if('A' <= q && q <= 'Z') q += ' ';
    if(p != q) return false;
  }
  return true;
}

STR_DEF bool str_parse_s64(str s, s64 *n) {
  u64 i=0;
  s64 sum = 0;
  s32 negative = 0;

  const char *data = (char *) s.data;
  
  if(s.len && data[0]=='-') {
    negative = 1;
    i++;
  }  
  while(i<s.len &&
	'0' <= data[i] && data[i] <= '9') {
    sum *= 10;
    s32 digit = data[i] - '0';
    sum += digit;
    i++;
  }

  if(negative) sum*=-1;
  *n = sum;
  
  return i==s.len;
}

STR_DEF bool str_parse_f64(str s, f64 *n) {
  if (s.len == 0) {
    return false;
  }

  f64 parsedResult = 0.0;
  s32 sign = 1;
  s32 decimalFound = 0;
  s32 decimalPlaces = 0;
  f64 exponentFactor = 1.0;

  u8 *data = (u8 *)s.data;

  u64 i = 0;

  if (i < s.len && (data[i] == '+' || data[i] == '-')) {
    if (data[i] == '-') {
      sign = -1;
    }
    i++;
  }

  while (i < s.len && ('0' <= data[i] || data[i] <= '9')) {
    parsedResult = parsedResult * 10.0 + (data[i] - '0');
    i++;
  }

  if (i < s.len && data[i] == '.') {
    i++;
    while (i < s.len && ('0' <= data[i] || data[i] <= '9')) {
      parsedResult = parsedResult * 10.0 + (data[i] - '0');
      decimalPlaces++;
      i++;
    }
    decimalFound = 1;
  }

  exponentFactor = 1.0;
  for (int j = 0; j < decimalPlaces; j++) {
    exponentFactor *= 10.0;
  }
  
  parsedResult *= sign;
  if (decimalFound) {
    parsedResult /= exponentFactor;
  }

  *n = parsedResult;

  return true;

}

static s32 str_index_of_impl(const char *haystack, u64 haystack_size, const char* needle, u64 needle_size) {
  if(needle_size > haystack_size) {
    return -1;
  }
  haystack_size -= needle_size;
  u64 i, j;
  for(i=0;i<=haystack_size;i++) {
    for(j=0;j<needle_size;j++) {
      if(haystack[i+j] != needle[j]) {
	break;
      }
    }
    if(j == needle_size) {
      return (int) i;
    }
  }
  return -1;

}

STR_DEF s32 str_index_of(str s, str needle) {
  return str_index_of_impl((char *) s.data, s.len, (char *) needle.data, needle.len);
}

STR_DEF s32 str_index_of_off(str s, u64 off, str needle) {
  if(off > s.len) {
    return - 1;
  }

  s32 pos = str_index_of_impl((char *) s.data + off, s.len - off, (char *) needle.data, needle.len);
  if(pos < 0) {
    return -1;
  }

  return pos + (s32) off;
}

STR_DEF bool str_chop_by(str *s, char *delim, str *d) {
  if(!s->len) return false;
  
  s32 pos = str_index_ofc(*s, (u8 *) delim);
  if(pos < 0) pos = (int) s->len;
      
  if(d) {
    *d = str_from(s->data, pos);
  }

  if(pos == (int) s->len) {
    *d = *s;
    s->len = 0;
    return true;
  } else {
    *s = str_from(s->data + pos + 1, s->len - pos - 1);
    return true;
  }

}

STR_DEF void str_builder_reserve(str_builder *sb, u64 needed_cap) {
  u64 cap;
  if(sb->cap == 0) {
    cap = STR_BUILDER_DEFAULT_CAP;
  } else {
    cap = sb->cap;
  }
    
  while(cap < needed_cap) {
    cap *= 2;
  }

  if(cap == sb->cap) {
    // nothing to do here
    
  } else {
    u8 *new_data = STR_ALLOC(cap);
    str_memcpy(new_data, sb->data, sb->len);
    if(sb->cap != 0) {
      STR_FREE(sb->data);
    }
    sb->data = new_data;
    sb->cap = cap;

  }
  
  
}

STR_DEF void str_builder_append(str_builder *sb, const u8 *data, u64 len) {
  str_builder_reserve(sb, sb->len + len);
  str_memcpy(sb->data + sb->len, data, len);
  sb->len += len;
}

STR_DEF void str_builder_appends64(str_builder *sb, s64 n) {
  char buf[32];
  u64 index = 31;  

  if(n == 0) {
    buf[index--] = '0';
  } else {

    int append_minus;
    if(n < 0) {
      append_minus = 1;
      n *= -1;
    } else {
      append_minus = 0;
    }
    
    while(n > 0) {
      buf[index--] = (n % 10) + '0';
      n = n / 10;
    }

    if(append_minus) {
      buf[index--] = '-';
    }
  }

  u64 len = sizeof(buf) - index - 1;
  str_builder_append(sb, (u8 *) &buf[index + 1], len);
}

STR_DEF void str_builder_appendf(str_builder *sb, char *fmt, ...) {
  va_list list;
  va_start(list, fmt);
  u64 len = (u64) vsnprintf(sb->data + sb->len, sb->cap - sb->len, fmt, list);
  va_end(list);

  if(sb->len + len <= sb->cap) {
    
  } else {
    str_builder_reserve(sb, sb->len + len);
    va_start(list, fmt);
    u64 len = (u64) vsnprintf(sb->data + sb->len, sb->cap - sb->len, fmt, list);
    va_end(list);
  }  

  sb->len += len;
}

STR_DEF Rune rune_decode(const u8 **data, u64 *data_len) {
  u8 c = (*data)[0];

  Rune rune;
  if((0xf0 & c) == 0xf0) {    
    rune = ((Rune) (c & 0x07)) << 18;
    rune |= ((Rune) ( (*data)[1] & 0x3f )) << 12;
    rune |= ((Rune) ( (*data)[2] & 0x3f )) <<  6;
    rune |= (Rune) ( (*data)[3] & 0x3f );
      
    *data = *data + 4;
    *data_len = *data_len - 4;
  } else if((0xe0 & c) == 0xe0) {

    rune = ((Rune) (c & 0x0f)) << 12;
    rune |= ((Rune) ( (*data)[1] & 0x3f )) << 6;
    rune |= (Rune) ( (*data)[2] & 0x3f );

    *data = *data + 3;
    *data_len = *data_len - 3;
  } else if((0xc0 & c) == 0xc0) {
    
    rune = ((Rune) (c & 0x1f)) << 6;
    rune |= (Rune) ( (*data)[1] & 0x3f );

    *data = *data + 2;
    *data_len = *data_len - 2;
  } else {
    STR_ASSERT((0x80 & c) != 0x80);
    rune = (Rune) c;

    *data = *data + 1;
    *data_len = *data_len - 1;
  }

  return rune;
}

STR_DEF void rune_encode(Rune rune, u8 buf[4], u64 *buf_len) {

  *buf_len = 0;
  
  if(rune <= 128) {
    buf[(*buf_len)++] = (u8) rune;
  } else if(rune <= 2048) {
    // **** ****  **** ****  **** *123  4567 89AB
    //                     |
    //                     v
    // ***1 2345 **67 89AB
    buf[(*buf_len)++] = 0xc0 | ((rune >> 6) & 0x1f);
    buf[(*buf_len)++] = 0x80 | (rune & 0x3f);
  } else if(rune <= 65536) {
    // **** ****  **** ****  1234 5678  9ABC DEFG
    //                     |
    //                     v
    // **** 1234  **56 789A  **BC DEFG
    buf[(*buf_len)++] = 0xe0 | ((rune >> 12) & 0x0f);
    buf[(*buf_len)++] = 0x80 | ((rune >> 6) & 0x3f);
    buf[(*buf_len)++] = 0x80 | (rune & 0x3f);
  } else {
    // **** ****  ***1 2345  6789 ABCD  EFGH IJKL
    //                     |
    //                     v
    // **** *123  **45 6789  **AB CDEF  **GH IJKL
    buf[(*buf_len)++] = 0xf0 | ((rune >> 18) & 0x07);
    buf[(*buf_len)++] = 0x80 | ((rune >> 12) & 0x3f);
    buf[(*buf_len)++] = 0x80 | ((rune >> 6) & 0x3f);
    buf[(*buf_len)++] = 0x80 | (rune & 0x3f);
  }
}

STR_DEF void rune_escape(Rune rune, u8 buf[6], u64 *buf_len) {
  *buf_len = 0;

  if(rune <= 128) {
    u8 c = (u8) rune;
    
    switch(c) {
    case '\"': {
      buf[(*buf_len)++] = '\\';
      buf[(*buf_len)++] = '\"';
    } break;
    case '\\': {
      buf[(*buf_len)++] = '\\';
      buf[(*buf_len)++] = '\\';	
    } break;
    case '/': {
      buf[(*buf_len)++] = '\\';
      buf[(*buf_len)++] = '/';      
    } break;
    case '\b': {
      buf[(*buf_len)++] = '\\';
      buf[(*buf_len)++] = 'f';
    } break;
    case '\f': {
      buf[(*buf_len)++] = '\\';
      buf[(*buf_len)++] = 'f';      
    } break;
    case '\n': {
      buf[(*buf_len)++] = '\\';
      buf[(*buf_len)++] = 'n';      
    } break;
    case '\r': {
      buf[(*buf_len)++] = '\\';
      buf[(*buf_len)++] = 'r';
    } break;
    case '\t': {
      buf[(*buf_len)++] = '\\';
      buf[(*buf_len)++] = 't';
    } break;
    default: {
      buf[(*buf_len)++] = (u8) rune;
    } break;
    } 
        
  } else {
    buf[0] = '\\';
    buf[1] = 'u';

    u64 pos = 5;

    while(rune > 0) {
      u8 c = (u8) (rune % 16);
      if(c < 10) {
	c += '0';
      } else {
	c += 'W';
      }
      buf[pos--] = c;
      rune /= 16;
    }

    while(pos >= 2) buf[pos--] = '0';
    *buf_len = 6;
  }
  
}


STR_DEF Rune rune_unescape(u8 **data, u64 *data_len) {
  u8 c = (*data)[0];

  Rune rune = 0;
  if(c != '\\') {  // unescaped u8 'a'
    rune = (Rune) c;
    *data = *data + 1;
    *data_len = *data_len - 1;
  } else {
    c = (*data)[1];

    if(c != 'u') { // escaped u8 '\n'

      switch(c) {
      case '\"': rune = (Rune) '\"'; break;
      case '\\': rune = (Rune) '\\'; break;
      case '/': rune = (Rune) '/'; break;
      case 'b': rune = (Rune) '\b'; break;
      case 'f': rune = (Rune) '\f'; break;
      case 'n': rune = (Rune) '\n'; break;
      case 'r': rune = (Rune) '\r'; break;
      case 't': rune = (Rune) '\t'; break;
      }

      *data = *data + 2;
      *data_len = *data_len - 2;
    } else { // unicode u8acter '\u00d6'
      
      Rune n;
      for(u8 i=0;i<4;i++) {
    
	c = (*data)[2 + i];
	if('0' <= c && c <= '9') {
	  n = c - '0';
	} else if('a' <= c && c <= 'f') {
	  n = c - 'W';
	} else if('A' <= c && c <= 'F') {
	  n = c - '7';
	} else {
	  break;
	}
	rune *= 16;
	rune += n;
      }

      *data = *data + 6;
      *data_len = *data_len - 6;
    }
     
  }

  return rune;
}


#endif // STR_IMPLEMENTATION

#undef u8
#undef u32
#undef s32
#undef s64
#undef u64
#undef f64

#endif //  _STR_H
