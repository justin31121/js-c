#ifndef JV_H
#define JV_H

// MIT License
// 
// Copyright (c) 2024 Justin Su8tner
// 
// Permission is hereby granted, free of u8ge, to any person obtaining a copy
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

typedef unsigned char Json_View_u8;
typedef unsigned long long int Json_View_u64;

#define u8 Json_View_u8
#define u64 Json_View_u64

#ifndef JV_ASSERT
#  include <assert.h>
#  define JV_ASSERT assert
#endif // JV_ASSERT

#ifndef JV_DEF
#  define JV_DEF static inline
#endif // JV_DEF

JV_DEF int jv_isdigit(u8 c);
JV_DEF int jv_isspace(u8 c);
JV_DEF int jv_memcmp(void *a, void *b, u64 len);
JV_DEF u64 jv_strlen(u8 *cstr);

typedef enum{
  JV_TYPE_NULL,
  JV_TYPE_FALSE,
  JV_TYPE_TRUE,
  JV_TYPE_NUMBER,
  JV_TYPE_STRING,
  JV_TYPE_ARRAY,
  JV_TYPE_OBJECT,
}Json_View_Type;

typedef struct{
  u8 *data;
  u64 len;
  Json_View_Type type;
}Json_View;

#define jv_fmt "%.*s"
#define jv_arg(j) (int) (j).len, (j).data
#define jv_from(d, l, t) (Json_View) { (d), (l), (t) }

JV_DEF int jv_parse(Json_View *view, u8 *data, u64 len);
JV_DEF int jv_parse_number(Json_View *view, u8 *data, u64 len);
JV_DEF int jv_parse_string(Json_View *view, u8 *data, u64 len);
JV_DEF int jv_parse_array(Json_View *view, u8 *data, u64 len);
JV_DEF int jv_parse_object(Json_View *view, u8 *data, u64 len);
JV_DEF int jv_parse_impl(Json_View *view, u8 *data, u64 len, u8 *target, u64 target_len, Json_View_Type type);

JV_DEF int jv_array_next(Json_View array, Json_View *sub_view, u64 *off);
JV_DEF int jv_array_get(Json_View array, u64 index, Json_View *value); 

JV_DEF int jv_object_next(Json_View object, Json_View *key_view, Json_View *value_view, u64 *off);
JV_DEF int jv_object_get(Json_View object, char *key, Json_View *value);

#ifdef JV_IMPLEMENTATION

JV_DEF int jv_isdigit(u8 c) {
  return '0' <= c && c <= '9';
}

JV_DEF int jv_isspace(u8 c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

JV_DEF int jv_memcmp(void *a, void *b, u64 len) {
  u8 *pa = a;
  u8 *pb = b;

  int d = 0;
  while(!d && len) {
    d = *pa++ - *pb++;
    len--;
  }

  return d;
}

JV_DEF u64 jv_strlen(u8 *cstr) {
  u64 len = 0;
  while(*cstr++) len++;
  return len;
}

static u8 jv_data_null[] = "null";
#define jv_parse_null(v, d, l) jv_parse_impl((v), (d), (l), jv_data_null, sizeof(jv_data_null) - 1, JV_TYPE_NULL)

static u8 jv_data_false[] = "false";
#define jv_parse_false(v, d, l) jv_parse_impl((v), (d), (l), jv_data_false, sizeof(jv_data_false) - 1, JV_TYPE_FALSE)

static u8 jv_data_true[] = "true";
#define jv_parse_true(v, d, l) jv_parse_impl((v), (d), (l), jv_data_true, sizeof(jv_data_true) - 1, JV_TYPE_TRUE)

JV_DEF int jv_parse(Json_View *view, u8 *data, u64 len) {

  if(len == 0) return 0;

  if(*data == 'n') return jv_parse_null(view, data, len);
  else if(*data == 'f') return jv_parse_false(view, data, len);
  else if(*data == 't') return jv_parse_true(view, data, len);
  else if(*data == '\"') return jv_parse_string(view, data, len);
  else if(*data == '[') return jv_parse_array(view, data, len);
  else if(*data == '{') return jv_parse_object(view, data, len);
  else if(*data == '-' || jv_isdigit(*data)) return jv_parse_number(view, data, len);
  else return 0;

}

JV_DEF int jv_parse_impl(Json_View *view, u8 *data, u64 len,
			  u8 *target, u64 target_len, Json_View_Type type) {
  
  if(len < target_len) return 0;
  if(jv_memcmp(data, target, target_len) != 0) return 0;
  *view = jv_from(data, target_len, type);
  
  return 1;
}

JV_DEF int jv_parse_number(Json_View *view, u8 *data, u64 len) {

  int encountered_dot = 0;

  u64 i = 1;
  while(1) {
    if(i >= len) return 0;
    if(data[i] == '.') {
      if(encountered_dot) return 0;
      else encountered_dot = 1;
    } else if(!jv_isdigit(data[i])) {
      break;
    }
    i++;
  }
  *view = jv_from(data, i, JV_TYPE_NUMBER);
  
  return 1;
}

JV_DEF int jv_parse_string(Json_View *view, u8 *data, u64 len) {
  
  u64 i = 1;
  while(1) {
    if(i >= len) return 0;
    if(data[i] == '\"') break;
    if(data[i] == '\\') i++;    
    i++;
  }
  *view = jv_from(data, i + 1, JV_TYPE_STRING);
  
  return 1;
}

JV_DEF int jv_parse_array(Json_View *view, u8 *data, u64 len) {

  u64 i = 1;
  while(1) {

    // whitespace
    while(1) {
      if(i >= len) return 0;
      if(!jv_isspace(data[i])) break;
      i++;
    }

    if(i >= len) return 0;
    if(data[i] == ']') break;

    Json_View sub_view;
    if(!jv_parse(&sub_view, data + i, len - i)) {
      return 0;
    }
    i += sub_view.len;

    // whitespace
    while(1) {
      if(i >= len) return 0;
      if(!jv_isspace(data[i])) break;
      i++;
    }

    if(i >= len) return 0;
    if(data[i] == ']') break;
    if(data[i] != ',') return 0;
    i++;
  }
  *view = jv_from(data, i + 1, JV_TYPE_ARRAY);
  
  return 1;
}

JV_DEF int jv_parse_object(Json_View *view, u8 *data, u64 len) {
  
  u64 i = 1;
  while(1) {

    // whitespace
    while(1) {
      if(i >= len) return 0;
      if(!jv_isspace(data[i])) break;
      i++;
    }

    if(i >= len) return 0;
    if(data[i] == '}') break;

    Json_View sub_key_view;
    if(!jv_parse(&sub_key_view, data + i, len - i)) {
      return 0;
    }
    i += sub_key_view.len;
    if(sub_key_view.type != JV_TYPE_STRING) return 0;

    // whitespace
    while(1) {
      if(i >= len) return 0;
      if(!jv_isspace(data[i])) break;
      i++;
    }

    if(i >= len) return 0;
    if(data[i] != ':') return 0;
    i++;

    // whitespace
    while(1) {
      if(i >= len) return 0;
      if(!jv_isspace(data[i])) break;
      i++;
    }

    Json_View sub_value_view;
    if(!jv_parse(&sub_value_view, data + i, len - i)) {
      return 0;
    }
    i += sub_value_view.len;

    // whitespace
    while(1) {
      if(i >= len) return 0;
      if(!jv_isspace(data[i])) break;
      i++;
    }

    if(i >= len) return 0;
    if(data[i] == '}') break;
    if(data[i] != ',') return 0;
    i++;
  }
  *view = jv_from(data, i + 1, JV_TYPE_OBJECT);

  return 1;
}

JV_DEF int jv_array_next(Json_View array, Json_View *sub_view, u64 *off) {
  JV_ASSERT(array.type == JV_TYPE_ARRAY);

  array.data += *off;
  array.len  -= *off;

  u64 i = 0;
  if((*off) == 0) i++;
  
  while(1) {
    if(i >= array.len) return 0;
    if(array.data[i] == ']') return 0;    

    // whitespace
    while(1) {
      if(i >= array.len) return 0;
      if(!jv_isspace(array.data[i])) break;
      i++;
    }

    if(!jv_parse(sub_view, array.data + i, array.len - i)) {
      return 0;
    }
    i += sub_view->len;

    // whitespace
    while(1) {
      if(i >= array.len) return 0;
      if(!jv_isspace(array.data[i])) break;
      i++;
    }

    if(i >= array.len) return 0;
    if(array.data[i] == ']' || array.data[i] == ',') {
      *off = (*off) + i + 1;
      return 1;
    } else {
      return 0;
    }
    
  }

  return 0;
}

JV_DEF int jv_object_next(Json_View object, Json_View *key_view, Json_View *value_view, u64 *off) {
  JV_ASSERT(object.type == JV_TYPE_OBJECT);

  object.data += *off;
  object.len  -= *off;

  u64 i = 0;
  if((*off) == 0) i++;
  
  while(1) {
    if(i >= object.len) return 0;
    if(object.data[i] == '}') return 0;    

    // whitespace
    while(1) {
      if(i >= object.len) return 0;
      if(!jv_isspace(object.data[i])) break;
      i++;
    }

    if(!jv_parse(key_view, object.data + i, object.len - i)) {
      return 0;
    }
    if(key_view->type != JV_TYPE_STRING) return 0;
    i += key_view->len;

    // whitespace
    while(1) {
      if(i >= object.len) return 0;
      if(!jv_isspace(object.data[i])) break;
      i++;
    }

    if(i >= object.len) return 0;
    if(object.data[i] != ':') return 0;
    i++;

    // whitespace
    while(1) {
      if(i >= object.len) return 0;
      if(!jv_isspace(object.data[i])) break;
      i++;
    }

    if(!jv_parse(value_view, object.data + i, object.len - i)) {
      return 0;
    }
    i += value_view->len;

    // whitespace
    while(1) {
      if(i >= object.len) return 0;
      if(!jv_isspace(object.data[i])) break;
      i++;
    }

    if(i >= object.len) return 0;
    if(object.data[i] == '}' || object.data[i] == ',') {
      *off = (*off) + i + 1;
      return 1;
    } else {
      return 0;
    }
  }

  return 0;
}

JV_DEF int jv_array_get(Json_View array, u64 index, Json_View *value) {

  index++;
  u64 off = 0;
  for(;index;index--) {
    if(!jv_array_next(array, value, &off)) {
      return 0;
    }

  }

  return 1;
}

JV_DEF int jv_object_get(Json_View object, char *key, Json_View *value) {

  u64 key_len = jv_strlen((u8 *) key);

  Json_View key_view;
  u64 off = 0;
  while(jv_object_next(object, &key_view, value, &off)) {
    JV_ASSERT(key_view.type == JV_TYPE_STRING && key_view.len >= 2);
    if(key_view.len - 2 != key_len) {
      continue;
    }

    if(jv_memcmp(key_view.data + 1, key, key_len) != 0) {
      continue;
    }

    return 1;
  }
  
  return 0;
}
  
#endif // JV_IMPLEMENTATION

#undef u8
#undef u64

#endif // JV_H
