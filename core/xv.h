#ifndef XV_H
#define XV_H

typedef unsigned char Xv_u8;
typedef int Xv_s32;
typedef unsigned long long int Xv_u64;
#define u8 Xv_u8
#define s32 Xv_s32
#define u64 Xv_u64

#ifndef XV_DEF
#  define XV_DEF static inline
#endif // XV_DEF

#define XV_SINGLETON_TAGS_XS()			\
  XV_SINGLETON_TAGS_X(!doctype)			\
  XV_SINGLETON_TAGS_X(link)

XV_DEF int xv_isspace(u8 c);
XV_DEF s32 xv_memcmp(void *a, void *b, u64 len);
XV_DEF int xv_equal(u8 *data, u64 len, char *cstr);

typedef enum {
  XV_TAG_TYPE_OPENING,   // <body>
  XV_TAG_TYPE_CLOSING,   // </body>
  XV_TAG_TYPE_IMMEDIATE, // <meta/>
} Xv_Tag_Type;

typedef enum {
  XV_TYPE_NAME,
  XV_TYPE_TAG,
  XV_TYPE_STRING,
} Xv_Type;

typedef struct {
  u8 *data;
  u64 len;
  Xv_Type type;
} Xv;

#define xv_from(d, l, t) (Xv) { (d), (l), (t) }

XV_DEF void xv_trim(Xv *x);

XV_DEF int xv_parse_tag(Xv *name,
			Xv *attributes,
			Xv_Tag_Type *t,
			u64 *consumed,
			u8 *data,
			u64 len);
XV_DEF int xv_parse(Xv *x, u8 *data, u64 len);

XV_DEF int xv_next(Xv x, Xv *sub, Xv *attributes, u64 *off);

#ifdef XV_IMPLEMENTATION

XV_DEF int xv_isspace(u8 c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

XV_DEF s32 xv_memcmp(void *a, void *b, u64 len) {
  char *pa = a;
  char *pb = b;

  s32 d = 0;
  while(!d && len) {
    d = *pa++ - *pb++;
    len--;
  }

  return d;
}

XV_DEF int xv_equal(u8 *data, u64 len, char *cstr) {

  u64 i = 0;
  while(1) {
    if(i == len) return *cstr == 0;
    if(*cstr == 0) return i == len;
    if(data[i] != *cstr) return 0;

    i++;
    cstr++;
  }
  
  return 0;
}

XV_DEF void xv_trim(Xv *x) {

  if(x->len == 0) return;
  
  // trim left
  u64 i = 0;
  while(i < x->len && xv_isspace(x->data[i])) i++;
  x->data += i;
  x->len  -= i;

  if(x->len == 0) return;
  
  // trim right
  u64 j = x->len - 1;
  while(j > 0 && xv_isspace(x->data[j])) j--;
  x->len = j + 1;  
  
}

XV_DEF int xv_parse_tag(Xv *name,
			Xv *attributes,
			Xv_Tag_Type *t,
			u64 *consumed,
			u8 *data,
			u64 len) {
  
  u64 i = 0;
  while(i < len && xv_isspace(data[i])) i++;
  if(i == len) return 0;

  if(data[i++] != '<') return 0;
  if(i == len) return 0;

  while(i < len && xv_isspace(data[i])) i++;
  if(i == len) return 0;

  if(data[i] == '/') {
    i++;
    if(i == len) return 0;
    
    while(i < len && xv_isspace(data[i])) i++;
    if(i == len) return 0;

    *t = XV_TAG_TYPE_CLOSING;
  } else {
    *t = XV_TAG_TYPE_OPENING;
  }  
  u64 begin = i;

  while(i < len && data[i] != '/' && data[i] != '>' && !xv_isspace(data[i])) i++;
  if(i == len) return 0;
  if(i == begin) return 0;
  u64 end = i;

  int in_quotations = 0;
  while(i < len) {
    u8 c = data[i];
    if(in_quotations) {
      if(c == '\"') {
	in_quotations = 0;
      } else {
	// pass
      }
    } else {
      if(c == '\"') {
	in_quotations = 1;
      } else if(c == '/') {
	break;
      } else if(c == '>') {
	break;
      } else {
	// pass
      }
    }
    i++;
  }  
  if(i == len) return 0;
  
  while(i < len && data[i] != '/' && data[i] != '>') i++;
  if(i == len) return 0;

  if(attributes) {
    if(i != end) {
      *attributes = xv_from(data + end, i - end, XV_TYPE_STRING);
      xv_trim(attributes);
    } else {
      *attributes = xv_from(NULL, 0, XV_TYPE_STRING);
    }
  }
  
  if(data[i] == '/') {    
    while(i < len && data[i] != '>') i++;
    if(i == len) return 0;

    *t = XV_TAG_TYPE_IMMEDIATE;
  } else { // data[i] == '>'

    // *t = *t;
  }

  *name = xv_from(data + begin, end - begin, XV_TYPE_STRING);
  *consumed = i + 1;

  return 1;
}

XV_DEF int xv_parse(Xv *x, u8 *data, u64 len) {

  Xv name;
  Xv_Tag_Type tag_type;
  u64 consumed;
  if(!xv_parse_tag(&name, NULL, &tag_type, &consumed, data, len)) {    
    return 0;
  }
  switch(tag_type) {
  case XV_TAG_TYPE_OPENING:
    // pass
    break;
  case XV_TAG_TYPE_CLOSING:    
    return 0;
  case XV_TAG_TYPE_IMMEDIATE:
    *x = xv_from(data, consumed, XV_TYPE_TAG);
    return 1;
  }
  u64 i = consumed;
  u64 i_copy = i;

  while(1) {

    while(i < len && data[i] != '<') i++;
    if(i == len) {
#define XV_SINGLETON_TAGS_X(n) \
      do{		       \
	if(xv_equal(name.data, name.len, #n )) {	\
	  *x = xv_from(data, len, XV_TYPE_TAG);		\
	  return 1;					\
	}						\
      }while(0);
      XV_SINGLETON_TAGS_XS()
#undef XV_SINGLETON_TAGS_X	
      return 0;
    }

    Xv next_name;
    if(!xv_parse_tag(&next_name, NULL, &tag_type, &consumed, data + i, len - i)) {      
      return 0;
    }

    switch(tag_type) {
    case XV_TAG_TYPE_OPENING: {
      Xv sub;
      if(!xv_parse(&sub, data + i, len - i)) {
	return 0;    	
      }
      i += sub.len;
      
    } break;
    case XV_TAG_TYPE_CLOSING: {
      if(name.len == next_name.len &&
	 xv_memcmp(name.data, next_name.data, name.len) == 0) {
	i += consumed;
	*x = xv_from(data, i, XV_TYPE_TAG);
	return 1;
      
      } else {
#define XV_SINGLETON_TAGS_X(n)					\
	do{							\
	  if(xv_equal(name.data, name.len, #n )) {		\
	    *x = xv_from(data, i_copy, XV_TYPE_TAG);		\
	    return 1;						\
	  }							\
	}while(0);
	XV_SINGLETON_TAGS_XS()
#undef XV_SINGLETON_TAGS_X

	return 0;    
      
      }
      
    } break;
    case XV_TAG_TYPE_IMMEDIATE: {
      i += consumed;
      
    } break;
    }

  }

  return 0;
}

XV_DEF int xv_next(Xv x, Xv *sub, Xv *attributes, u64 *off) {

  u8 *data = x.data + *off;
  u64 len  = x.len  - *off;

  u64 i = 0;
  if(*off == 0) {
    
    Xv name;
    Xv_Tag_Type tag_type;
    u64 consumed;
    if(!xv_parse_tag(&name, attributes, &tag_type, &consumed, data, len)) {
      return 0;
    }
    if(tag_type == XV_TAG_TYPE_CLOSING) {
      return 0;
    }

    *sub = xv_from(name.data, name.len, XV_TYPE_NAME);
    *off = *off + consumed;
    return 1;
  }
  
  u64 begin = i;

  while(i < len && data[i] != '<') i++;
  if(i == len) return 0;
    
  if(i == begin) {
    if(!xv_parse(sub, data + i, len - i)) {
      return 0;
    }
    *off = *off + i + sub->len;
    
    return 1;
    
  } else {
    u64 end = i;
    *sub = xv_from(data + begin, end - begin, XV_TYPE_STRING);
    xv_trim(sub);
    *off = *off + end;

    if(sub->len == 0) {
      return xv_next(x, sub, attributes, off);
      
    } else {
      return 1;
      
    }
    
  }
    
}

#endif // XV_IMPLEMENTATION

#undef u8
#undef s32
#undef u64

#endif // XV_H
