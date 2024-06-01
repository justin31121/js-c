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

/* main.c: */
#if 0
#include <stdio.h>

#define CONF_IMPLEMENTATION
#include "conf.h"

int main() {

  const char content_cstr[] =
    "[minimal]config=file";

  conf_str content;
  content.data = (unsigned char *) content_cstr;
  content.len = sizeof(content_cstr) - 1;
  
  conf_str object;
  while(conf_object_next(&content, &object)) {
    conf_str name = conf_object_name(object);    
    printf("%.*s\n", (int) name.len, name.data);

    conf_str key, value;
    while(conf_attribute_next(&object, &key, &value)) {
      printf("\t'%.*s' = '%.*s'\n",
	     (int) key.len, key.data,
	     (int) value.len, value.data);
    }
  }

}
#endif

/*  example.conf:  (https://docs.fileformat.com/programming/config/) */
/*
    [default]
    region = us-west-2
    output = json
    
    [profile dev-user]
    region = us-east-1
    output = text
    
    [profile developers]
    role_arn = arn:aws:iam::123456789012:role/developers
    source_profile = dev-user
    region = us-west-2
    output = json
 */

#ifndef CONF_H
#define CONF_H

typedef unsigned char conf_u8;
typedef unsigned long long conf_u64;

#define u8 conf_u8
#define u64 conf_u64

typedef struct {
  u8 *data;
  u64 len;
} conf_str;

#define str conf_str

#ifndef CONF_DEF
#  define CONF_DEF static inline
#endif // CONF_DEF

CONF_DEF int conf_isspace(u8 c);
CONF_DEF void conf_str_trim(str *s);
CONF_DEF int conf_comment_chop(str *content);

CONF_DEF int conf_object_next(str *content, str *object);
CONF_DEF str conf_object_name(str object);
CONF_DEF int conf_attribute_next(str *object, str *key, str *value);

#ifdef CONF_IMPLEMENTATION

CONF_DEF int conf_isspace(u8 c) {
  return
    c == ' '  ||
    c == '\t' ||
    c == '\r' ||
    c == '\n';
}

CONF_DEF void conf_str_trim(str *s) {
  
  while(s->len && conf_isspace(*s->data)) {
    s->data++;
    s->len--;
  }

  while(s->len && conf_isspace(*(s->data + s->len - 1))) {
    s->len--;
  }
  
}

CONF_DEF int conf_comment_chop(str *content) {
  
  
   while(content->len &&
	conf_isspace(*content->data)) {
    content->data++;
    content->len--;
  }

   if(content->len && *content->data == '#') {
     while(content->len && *content->data != '\n') {
       content->data++;
       content->len--;
     }

     if(content->len && *content->data == '\n') {       
       content->data++;
       content->len--;
     }
     
     return 1;
   } else {
     
     return 0;
   }
   
}

CONF_DEF int conf_object_next(str *content, str *object) {

  while(conf_comment_chop(content)) ;
  
  while(content->len && conf_isspace(*content->data)) {
    content->data++;
    content->len--;
  }

  u8 *start = content->data;
  
  if(!content->len || *content->data != '[') return 0;
  content->data++;
  content->len--;

  while(content->len && *content->data != ']') {
    content->data++;
    content->len--;    
  }

  if(!content->len || *content->data != ']') return 0;
  content->data++;
  content->len--;
   
  while(content->len && *content->data != '[') {
    content->data++;
    content->len--;    
  }

  u8 *end = content->data;

  object->data = start;
  object->len = end - start;
  
  return object->len > 0;
}

CONF_DEF str conf_object_name(str object) {

  str name;
  name.data = object.data + 1;

  while(object.len && *object.data != ']') {
    object.data++;
    object.len--;    
  }

  name.len = object.data - name.data;

  return name;
}

CONF_DEF int conf_attribute_next(str *object, str *key, str *value) {

  // maybe skip name
  if(object->len && *object->data == '[') {

    while(object->len && *object->data != ']') {
      object->data++;
      object->len--;    
    }
    
    if(!object->len || *object->data != ']') return 0;
    object->data++;
    object->len--;
  }

  while(conf_comment_chop(object)) ;
  
  // key
  u8 *key_start = object->data;
  while(object->len && *object->data != '=') {
    object->data++;
    object->len--;
  }

  u8 *key_end = object->data;
  key->data = key_start;
  key->len = key_end - key_start;
  conf_str_trim(key);
  
  if(!object->len || *object->data != '=') return 0;
  object->data++;
  object->len--;

  // value
  u8 *value_start = object->data;
  while(object->len && *object->data != '\n') {
    object->data++;
    object->len--;
  }

  u8 *value_end = object->data;
  value->data = value_start;
  value->len = value_end - value_start;
  conf_str_trim(value);

  return key->len > 0 && value->len > 0;
}

#endif // CONF_IMPLEMENTATION

#undef u8
#undef u64
#undef str

#endif  // CONF_H
