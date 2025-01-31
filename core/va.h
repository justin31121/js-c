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
} Va_Type;

#define VA_FLAG_HEX 0x1

typedef struct {
    Va_Type type;
    u32 flags;
    union {
	str s;
	s64 n;
    } as;
} Va;

#define va_c(cstr) ((Va) { .type = VA_TYPE_STR, .flags = 0, .as.s = str_fromc(cstr) })
#define va_s(n) ((Va) { .type = VA_TYPE_STR, .flags = 0, .as.s = (n) })
#define va_n(s) ((Va) { .type = VA_TYPE_S64, .flags = 0, .as.n = ((s64) (s)) })
#define va_n_(s, fs) ((Va) { .type = VA_TYPE_S64, .flags = (fs), .as.n = ((s64) (s)) })

typedef struct {
    str fmt;
    
    Va *vas;
    u64 vas_len;

    u8 buf_len;
    u8 buf[32];
    u8 buf_off;
} Va_Format;

#define Va_Format_from(fmt, ...) (Va_Format) { str_fromc(fmt), va_catch(__VA_ARGS__), 0 }

typedef enum {
    VA_FORMAT_RESULT_OK = 0,
    VA_FORMAT_RESULT_DONE,
    
    VA_FORMAT_RESULT_VA_UNDERFLOW,
    VA_FORMAT_RESULT_VA_OVERRFLOW,
} Va_Format_Result;

VA_DEF Va_Format_Result va_format_next(Va_Format *f, u8 *buf, u64 buf_len, u64 *out);

#define va_catch(...) ((Va[]){__VA_ARGS__}), (sizeof((Va[]){__VA_ARGS__})/sizeof(Va)) 
#define va_appendf(sb, fmt, ...) va_appendf_impl((sb), str_fromc(fmt), va_catch(__VA_ARGS__))
VA_DEF Va_Format_Result va_appendf_impl(str_builder *sb, str fmt, Va *vas, u64 vas_len);

#ifdef VA_IMPLEMENTATION

VA_DEF Va_Format_Result va_format_next(Va_Format *f, u8 *buf, u64 buf_len, u64 *out) {
    u64 len = 0;
    
    for(u64 i=0;i<buf_len && 0 < f->fmt.len;i++) {
	u8 c = *f->fmt.data;
	if(c != '%') {
	    buf[len++] = c;
	    f->fmt.len--;
	    f->fmt.data++;
	    continue;
	}

	if(1 < f->fmt.len && f->fmt.data[1] == '%') {
	    buf[len++] = '%';
	    f->fmt.data += 2;
	    f->fmt.len -= 2;
	    continue;
	}

	if(0 < f->buf_len) {
	    buf[len++] = f->buf[f->buf_off];
	    f->buf_off++;
	    f->buf_len--;

	    if(f->buf_len == 0) {
		f->fmt.data++;
		f->fmt.len--;
	    }
	} else {
	    if(f->vas_len == 0) {
		return VA_FORMAT_RESULT_VA_UNDERFLOW;
	    }
	    Va *va = &f->vas[0];	

	    switch(va->type) {
	    case VA_TYPE_STR:
		str *s = &va->as.s;
		if(s->len == 0) {
		    f->vas++;
		    f->vas_len--;
		    
		    f->fmt.data++;
		    f->fmt.len--;
		    i -= 1;
		} else {
		    buf[len++] = *s->data;
		    s->data++;
		    s->len--;
		}
		break;
		
	    case VA_TYPE_S64:
		s64 n = va->as.n;
		if(n == 0) {
		    buf[len++] = '0';
		    
		    f->vas++;
		    f->vas_len--;
		    
		    f->fmt.data++;
		    f->fmt.len--;
		} else {
		    f->vas++;
		    f->vas_len--;

		    int is_negative = n < 0;
		    if(is_negative) n *= -1;

		    int shall_produce_hex = va->flags & VA_FLAG_HEX;

		    f->buf_off = sizeof(f->buf);
		    f->buf_len = 0;
		    while(n > 0) {

			switch(shall_produce_hex) {
			case 0:
			    f->buf[f->buf_off-- - 1] = (n % 10) + '0';
			    f->buf_len++;
			    n /= 10;
			    break;
			default:
			    u8 m = n % 16;
			    if(9 < m) {
				m += 'W';
			    } else {
				m += '0';
			    }
			    
			    f->buf[f->buf_off-- - 1] = m;
			    f->buf_len++;
			    n >>= 4; // n /= 16;
			    break;
			}
			
		    }
		    if(is_negative) {
			f->buf[f->buf_off-- - 1] = '-';
			f->buf_len++;
		    }
		    i -= 1;
		}
		break;
		
	    default:
		TODO();
	    }
	}
	
    }

    *out = len;
    if(0 < len || 0 < f->fmt.len) {
	return VA_FORMAT_RESULT_OK;
    } else {
	if(f->vas_len > 0) {
	    return VA_FORMAT_RESULT_VA_OVERRFLOW;
	} else {
	    return VA_FORMAT_RESULT_DONE;
	}	
    }

}

VA_DEF Va_Format_Result va_appendf_impl(str_builder *sb, str fmt, Va *vas, u64 vas_len) {

    Va_Format format = (Va_Format) { fmt, vas, vas_len, 0 };

    while(1) {
	if(sb->len == sb->cap) str_builder_reserve(sb, sb->len << 1);
	
	u64 len;
	Va_Format_Result result = va_format_next(&format, sb->data + sb->len, sb->cap - sb->len, &len);
	if(result == VA_FORMAT_RESULT_OK) {
	    sb->len += len;
	} else {
	    return result;
	}
    }
    
    
}

#endif // VA_IMPLEMENTATION

#endif // VA_H
