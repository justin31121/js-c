#ifndef VA_H
#define VA_H

#ifndef VA_DEF
#  define VA_DEF static inline
#endif // VA_DEF

#ifdef VA_IMPLEMENTATION
#  define STR_IMPLEMENTATION
#endif // VA_IMPLEMENTATION

#include <core/str.h>
#include <core/types.h>

typedef enum {
  VA_TYPE_STR,
  VA_TYPE_S64,
  VA_TYPE_S64_HEX,
} Va_Type;

typedef struct {
  Va_Type type;
  union {
    str str;
    s64 s64;
  } as;
} Va;

#define va_c(cstr) ((Va) { .type = VA_TYPE_STR, .as.str = str_fromc(cstr) })
#define va_s(s) ((Va) { .type = VA_TYPE_STR, .as.str = (s) })
#define va_n(n) ((Va) { .type = VA_TYPE_S64, .as.s64= ((s64) (n)) })
#define va_n_hex(n) ((Va) { .type = VA_TYPE_S64_HEX, .as.s64= ((s64) (n)) })

#define va_catch(...) ((Va[]){__VA_ARGS__}), (sizeof((Va[]){__VA_ARGS__})/sizeof(Va)) 
#define va_appendf(sb, fmt, ...) va_appendf_impl((sb), str_fromc(fmt), va_catch(__VA_ARGS__))
VA_DEF int va_appendf_impl(str_builder *sb, str fmt, Va *vas, u64 vas_len);

#ifdef VA_IMPLEMENTATION

VA_DEF int va_appendf_impl(str_builder *sb, str fmt, Va *vas, u64 vas_len) {

  u64 vas_index = 0;
  u64 last = 0;
  for(u64 i=0;i<fmt.len;i++) {
    u8 c = fmt.data[i];
    if(c == '%') {
      if(last < i) {
	str_builder_append(sb, fmt.data + last, i - last);
      }
				
      if(vas_len <= vas_index) {
	return 0;
      }

      Va *va = &vas[vas_index++];
      switch(va->type) {
      case VA_TYPE_STR:
	str_builder_appends(sb, va->as.str);
	break;
      case VA_TYPE_S64:
	str_builder_appends64(sb, va->as.s64);
	break;
      default:
	TODO();
      }
      last = i + 1;
    }
  }
  if(vas_index != vas_len) {
    return 0;
  }
  if(last < fmt.len) {
    str_builder_append(sb, fmt.data + last, fmt.len - last);
  }

  return 1;
}

#endif // VA_IMPLEMENTATION

#endif // VA_H
