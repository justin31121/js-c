#ifndef FLAG_H
#define FLAG_H

#ifdef FLAG_IMPLEMENTATION 
#  define STR_IMPLEMENTATION
#endif // FLAG_IMPLEMENTATION

#include <core/str.h>
#include <core/types.h>

typedef enum {
	FLAG_TYPE_S64,
	FLAG_TYPE_F64,
	FLAG_TYPE_STR,
	FLAG_TYPE_BOOL,
} Flag_Type;

#define FLAG_ATTRIBUTE_FOUND 0x1
#define FLAG_ATTRIBUTE_OPTIONAL 0x2
#define FLAG_ATTRIBUTE_DOUBLE_DASH 0x4
#define FLAG_ATTRIBUTE_SPECIAL 0x8
#define FLAG_ATTRIBUTE_MASTER 0x10

#define FLAG_ATTRIBUTE_IMPLICIT_BOOL (FLAG_ATTRIBUTE_OPTIONAL | FLAG_ATTRIBUTE_SPECIAL)

typedef struct {
	Flag_Type type;
	str name;
	char *description;
	union {
		s64 s64;
		f64 f64;
		str str;
		int b32;
	} as;
	u32 attributes;
} Flag;
#define flag_from(t, n, d, as) (Flag) { .type = (t), .name = str_fromc(n), .description = (d), .attributes = (as) }

int flag_find(Flag **fs, u64 fs_len, str name, Flag **_f);
void flag_fmts(Flag *f, str *dashes, str *value);

#define flag_parse(c, v, ...) flag_parse_impl((c), (v), ((Flag*[]) { __VA_ARGS__ }), (sizeof((Flag*[]) { __VA_ARGS__ })/sizeof(Flag*)) )
int flag_parse_impl(s32 argc, char **argv, Flag **fs, u64 fs_len);

#ifdef FLAG_IMPLEMENTATION

int flag_find(Flag **fs, u64 fs_len, str name, Flag **_f) {
	for(u64 i=0;i<fs_len;i++) {
		Flag *f = fs[i];
		if(str_eq(name, f->name)) {
			*_f = f;
			f->attributes |= FLAG_ATTRIBUTE_FOUND;
			return 1;
		}
	}

	return 0;
}

int flag_next_master(Flag **fs, u64 fs_len, u64 master_index, Flag **_f) {

	for(u64 i=0;i<fs_len;i++) {
		Flag *f = fs[i];
		if(!(f->attributes & FLAG_ATTRIBUTE_MASTER)) {
			continue;
		}

		if(0 < master_index) {
			master_index--;
			continue;
		}

		*_f = f;
		f->attributes |= FLAG_ATTRIBUTE_FOUND;
		return 1;
	}

	return 0;
}

void flag_fmts(Flag *f, str *dashes, str *value) {

	if(f->attributes & FLAG_ATTRIBUTE_MASTER) {
		*dashes = str_fromd("");
		*value = str_fromd("");

	} else {

		if(f->attributes & FLAG_ATTRIBUTE_DOUBLE_DASH) {
			*dashes = str_fromd("--");
		} else {
			*dashes = str_fromd("-");
		}

		if(f->attributes & FLAG_ATTRIBUTE_SPECIAL) {
			*value = str_fromd("");

		} else {
			switch(f->type) {

				case FLAG_TYPE_S64:
					*value = str_fromd("<s64>");
					break;
				case FLAG_TYPE_F64:
					*value = str_fromd("<f64>");
					break;
				case FLAG_TYPE_STR:
					*value = str_fromd("<string>");
					break;
				case FLAG_TYPE_BOOL:
					*value = str_fromd("<bool>");
					break;
			}

		}

	}
}

void flag_parse_impl_usage(Flag **fs, u64 fs_len, u64 master_len, char *program) {

	printf("USAGE: %s --help / [options...]", program);

	u64 master_index = 0;
	for(u64 i=0;i<fs_len && master_index<master_len;i++) {
		Flag *f = fs[i];
		if(!(f->attributes & FLAG_ATTRIBUTE_MASTER)) {
			continue;
		}

		printf(" <"str_fmt">", str_arg(f->name));
	}

	printf("\n\n");
}

#define flag_parse(c, v, ...) flag_parse_impl((c), (v), ((Flag*[]) { __VA_ARGS__ }), (sizeof((Flag*[]) { __VA_ARGS__ })/sizeof(Flag*)) )
int flag_parse_impl(s32 argc, char **argv, Flag **fs, u64 fs_len) {

	u64 master_len = 0;
	for(u64 i=0;i<fs_len;i++) {
		Flag *f = fs[i];
		if(f->attributes & FLAG_ATTRIBUTE_SPECIAL) {
			f->as.b32 = 0;
		}

		if(f->attributes & FLAG_ATTRIBUTE_MASTER) {
			master_len++;
		} else {
			if(str_index_ofc(f->name, "-") > 0) {
				f->attributes |= FLAG_ATTRIBUTE_DOUBLE_DASH;
			}	
		}
	}

	u64 master_index = 0;
	char *program = argv[0];
	for(s32 i=1;i<argc;i++) {
		str arg = str_fromc(argv[i]);

		if(str_eq_ignorecase(arg, str_fromd("--help"))) {

			flag_parse_impl_usage(fs, fs_len, master_len, program);

			//  --help 
			u64 max_width = 6;
			str dashes, value;

			for(u64 j=0;j<fs_len;j++) {
				Flag *f = fs[j];
				flag_fmts(f, &dashes, &value);

				u64 width = dashes.len + value.len + 1 + f->name.len;
				if(max_width < width) max_width = width;
			}

			for(u64 j=0;j<fs_len;j++) {
				Flag *f = fs[j];
				flag_fmts(f, &dashes, &value);

				u64 width = dashes.len + f->name.len + value.len;
				printf("  " str_fmt str_fmt " " str_fmt, str_arg(dashes), str_arg(f->name), str_arg(value) );
				for(u64 w=width;w<max_width;w++) printf(" ");
				printf("  %s\n", f->description);
			}

			printf("  --help");
			for(u64 w=6;w<max_width;w++) printf(" ");
			printf(" Print this help message\n");

			return 0;
		}

		Flag *f;
		int double_dash = 0;
		if(0 < arg.len && arg.data[0] == '-') {
			arg = str_from(arg.data + 1, arg.len - 1);
			if(arg.len > 0 && arg.data[0] == '-') {
				arg = str_from(arg.data + 1, arg.len - 1);
				double_dash = 1;
			}

			if(!flag_find(fs, fs_len, arg, &f)) {
				fprintf(stderr, "ERROR: unknown flag: '"str_fmt"'\n", str_arg(arg));
				flag_parse_impl_usage(fs, fs_len, master_len, program);
				return 0;
			}


		} else {
			if(master_len <= master_index) {
				fprintf(stderr, "ERROR: flags must start with a '-'. But got '"str_fmt"'\n", str_arg(arg));
				flag_parse_impl_usage(fs, fs_len, master_len, program);
				return 0;
			} else {

				if(!flag_next_master(fs, fs_len, master_index, &f)) {
					panic("unreachable");
				}
				master_index++;
			}
		}

		if(f->attributes & FLAG_ATTRIBUTE_DOUBLE_DASH) {
			if(double_dash) {
				// fine
			} else {
				fprintf(stderr, "ERROR: unknown flag: '-"str_fmt"'\n", str_arg(arg));
				flag_parse_impl_usage(fs, fs_len, master_len, program);
				return 0;
			}	
		}

		if(f->attributes & FLAG_ATTRIBUTE_SPECIAL) {
			f->as.b32 = 1;

		} else {
			str value;
			if(f->attributes & FLAG_ATTRIBUTE_MASTER) {
				value = arg;	
			} else {
				if(argc <= i + 1) {
					fprintf(stderr, "ERROR: Missing a value for the flag: '"str_fmt"'\n", str_arg(arg));
					flag_parse_impl_usage(fs, fs_len, master_len, program);
					return 0;
				}
				i++;
				value = str_fromc(argv[i]);
			}

			switch(f->type) {
				case FLAG_TYPE_STR: 
					f->as.str = value;
					break;
				case FLAG_TYPE_S64:
					if(!str_parse_s64(value, &f->as.s64)) {
						fprintf(stderr, "ERROR: Cannot parse '"str_fmt"'. Expected s64\n", str_arg(value));
						flag_parse_impl_usage(fs, fs_len, master_len, program);
						return 0;
					}
					break;
				case FLAG_TYPE_F64:
					if(!_str_parse_f64(value, &f->as.f64)) {
						fprintf(stderr, "ERROR: Cannot parse '"str_fmt"'. Expected f64\n", str_arg(value));
						flag_parse_impl_usage(fs, fs_len, master_len, program);
						return 0;
					}	
					break;
				case FLAG_TYPE_BOOL:
					if(str_eq_ignorecase(value, str_fromd("true")) || str_eq_ignorecase(value, str_fromd("yes")) || str_eq_ignorecase(value, str_fromd("1")) ) {
						f->as.b32 = 1;	
					} else if(str_eq_ignorecase(value, str_fromd("false")) || str_eq_ignorecase(value, str_fromd("no")) || str_eq_ignorecase(value, str_fromd("0")) ) {
						f->as.b32 = 0;
					} else {
						fprintf(stderr, "ERROR: Unsupported value '"str_fmt"' for boolean '"str_fmt"'. Supported are the following values: 'true', 'false', 'yes', 'no', '1' and '0'\n", 
								str_arg(value), str_arg(arg));
						flag_parse_impl_usage(fs, fs_len, master_len, program);
						return 0;
					}	
					break;
			}

		}


	}

	int missing_flags = 0;
	for(u64 i=0;i<fs_len;i++) {
		Flag *f = fs[i];
		if(f->attributes & FLAG_ATTRIBUTE_OPTIONAL) {
			continue;
		}

		if(f->attributes & FLAG_ATTRIBUTE_FOUND) {
			continue;
		}

		str dashes, value;
		flag_fmts(f, &dashes, &value);

		if(f->attributes & FLAG_ATTRIBUTE_MASTER) {
			fprintf(stderr, "ERROR: No value was provided for '"str_fmt"'\n", str_arg(f->name));
		} else {
			fprintf(stderr, "ERROR: No value was provided for flag '" str_fmt str_fmt" " str_fmt "'\n", str_arg(dashes), str_arg(f->name), str_arg(value));
		}


		missing_flags = 1;
	}
	if(missing_flags) {
		flag_parse_impl_usage(fs, fs_len, master_len, program);
		return 0;
	}

	return 1;
}


#endif // FLAG_IMPLEMENTATION

#endif // FLAG_H
