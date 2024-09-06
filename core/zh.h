#ifndef ZH_H
#define ZH_H

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <windows.h>

typedef unsigned long long int Zh_u64;
typedef int Zh_s32;
typedef unsigned char Zh_u8;
#define s32 Zh_s32
#define u64 Zh_u64
#define u8 Zh_u8

#ifndef ZH_DEF
#  define ZH_DEF static inline
#endif // ZH_DEF

#define zh_da_append_many(n, xs, xs_len) do{				\
    size_t new_cap = (n)->cap;						\
    while((n)->len + xs_len >= new_cap) {				\
      new_cap *= 2;							\
      if(new_cap == 0) new_cap = 16;					\
    }									\
    if(new_cap != (n)->cap) {						\
      (n)->cap = new_cap;						\
      (n)->data = realloc((n)->data, (n)->cap * sizeof(*((n)->data))); \
      assert((n)->data);						\
    }									\
    memcpy((n)->data + (n)->len, xs, xs_len * sizeof(*((n)->data)));	\
    (n)->len += xs_len;							\
  }while(0)

ZH_DEF char *zh_last_error_cstr(char *buf, u64 buf_len);

typedef struct {
  u8 *data;
  u64 len;
  u64 cap;
} Zh_str_builder;

typedef Zh_str_builder Zh;

#define zh_append(z, ...) zh_append_impl((z), __VA_ARGS__, NULL)
ZH_DEF void zh_append_impl(Zh *z, ...);
ZH_DEF int zh_run(Zh *zh, s32 *exit_code, Zh_str_builder *sb_out, Zh_str_builder *sb_err);
ZH_DEF s32 __zh_run(Zh *zh);

#define ZH_LOG_TYPE_XS()			\
  ZH_LOG_TYPE_X(CMD)				\
       ZH_LOG_TYPE_X(INFO)			\
       ZH_LOG_TYPE_X(ERROR)			\

typedef enum {
#define ZH_LOG_TYPE_X(t) ZH_##t ,
  ZH_LOG_TYPE_XS()
#undef ZH_LOG_TYPE_X
} Zh_Log_Type;

ZH_DEF void zh_log(Zh_Log_Type type, char *fmt, ...);
ZH_DEF char *zh_next(s32 *argc, char ***argv);

typedef FILETIME Zh_Time;

ZH_DEF int zh_time_get(Zh_Time *time, char *file_path);

#define zh_needs_rebuild(n, dst, ...) zh_needs_rebuild_impl((n), (dst), __VA_ARGS__, NULL)
ZH_DEF int zh_needs_rebuild_impl(int *needs_rebuild, char *dst, ...);
#define __zh_needs_rebuild(dst, ...) __zh_needs_rebuild_impl((dst), __VA_ARGS__, NULL)
ZH_DEF int __zh_needs_rebuild_impl(char *dst, ...);
ZH_DEF int zh_needs_rebuild_impl_impl(int *needs_rebuild, char *dst, va_list srcs);

#ifdef __GNUC__
#  define __zh_keep_updated_compiler() zh_append(&zh, "gcc", "-o", dst, src)
#else
#  define __zh_keep_updated_compiler() do{				\
    zh_log(ZH_ERROR, "Unknown Compiler: implement '__sh_keep_updated_compiler()' in zh.h"); \
    return 1;								\
  }while(0)
#endif

#define zh_keep_updated(argc, agrv) do {		\
  char dst[MAX_PATH];							\
  if(strstr(argv[0], ".exe")) {						\
    assert((size_t) snprintf(dst, sizeof(dst), "%s", argv[0]) < sizeof(dst)); \
  } else {								\
    assert((size_t) snprintf(dst, sizeof(dst), "%s.exe", argv[0]) < sizeof(dst)); \
  }									\
									\
  char dst_old[MAX_PATH];						\
  snprintf(dst_old, sizeof(dst_old), "%s.old", dst);			\
									\
  char buf[1024];							\
  char *src = __FILE__;							\
									\
  Zh_Time time_src, time_dst;						\
  if(!zh_time_get(&time_src, src)) return 1;				\
  if(!zh_time_get(&time_dst, dst)) return 1;				\
  if(CompareFileTime(&time_src, &time_dst) > 0) {			\
									\
    if(!DeleteFile(dst_old) && GetLastError() != ERROR_FILE_NOT_FOUND) { \
      zh_log(ZH_ERROR, "Can not delete '%s': %s",			\
	     dst_old, zh_last_error_cstr(buf, sizeof(buf)));		\
      return 1;								\
    }									\
									\
    if(!MoveFile(dst, dst_old)) {					\
      zh_log(ZH_ERROR, "Can not move file '%s' to '%s': %s",		\
	     dst, dst_old, zh_last_error_cstr(buf, sizeof(buf)));	\
      return 1;								\
    }									\
									\
    Zh zh = {0};							\
    __zh_keep_updated_compiler();					\
									\
    int exit_code;							\
    if(!zh_run(&zh, &exit_code, NULL, NULL)) {					\
      return 1;								\
    }									\
    if(exit_code != 0) {						\
									\
      if(!MoveFile(dst_old, dst)) {					\
	zh_log(ZH_ERROR, "Failed to move file: '%s' to '%s' : %s\n",	\
	       dst_old, dst, zh_last_error_cstr(buf, sizeof(buf)));	\
	return 1;							\
      }									\
									\
      return exit_code;							\
    }									\
									\
    zh.len = 0;								\
    zh_append(&zh, dst);						\
    for(int i=1;i<argc;i++) {						\
      zh_append(&zh, argv[i]);						\
    }									\
    									\
    if(!zh_run(&zh, &exit_code, NULL, NULL)) {					\
      return 1;								\
    } else {								\
      return exit_code;							\
    }									\
									\
    } else {								\
  }									\
									\
  }while(0)

#ifdef ZH_IMPLEMENTATION

ZH_DEF char *zh_last_error_cstr(char *buf, u64 buf_len) {
  FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
		 NULL,
		 GetLastError(),
		 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		 (LPSTR) buf,
		 buf_len,
		 NULL);

  u64 len = strlen(buf);
  if(buf[len] == '\n') {
    buf[len - 1] = 0;    
  }
  
  return buf;
}

ZH_DEF void zh_append_impl(Zh *zh, ...) {
  va_list cs;
  va_start(cs, zh);

  while(1) {
    char *item = va_arg(cs, char *);
    if(!item) break;

    zh_da_append_many(zh, item, strlen(item));
    zh_da_append_many(zh, " ", 1);
  }

  va_end(cs);
}

ZH_DEF int zh_run(Zh *zh, s32 *exit_code, Zh_str_builder *sb_out, Zh_str_builder *sb_err) {
  if(zh->len == 0) {
    zh_log(ZH_ERROR, "Cannot start empty process");
    return 0;
  }

  // replace last ' ' with 0
  zh->data[zh->len - 1] = '\0';

  zh_log(ZH_CMD, "%s", zh->data);

	HANDLE stdout_read = INVALID_HANDLE_VALUE;
	HANDLE stdout_write = INVALID_HANDLE_VALUE;
	HANDLE stderr_read = INVALID_HANDLE_VALUE;
	HANDLE stderr_write = INVALID_HANDLE_VALUE;

	int use_stdout = sb_out != NULL;
	int use_stderr = sb_err != NULL;
	int redirect_output = use_stdout || use_stderr;

  STARTUPINFO si;
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);

	if(redirect_output) {
		si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
		si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		si.dwFlags |= STARTF_USESTDHANDLES;
	}

	SECURITY_ATTRIBUTES attr;
	attr.nLength = sizeof(attr);
	attr.bInheritHandle = TRUE;
	attr.lpSecurityDescriptor = NULL;

	if(use_stdout) {
		if(!CreatePipe(&stdout_read, &stdout_write, &attr, 0)) {
			TODO();	
		}

		if(!SetHandleInformation(stdout_read, HANDLE_FLAG_INHERIT, 0)) {
			TODO();
		}
		si.hStdOutput = stdout_write;
	}

	if(use_stderr) {
		if(!CreatePipe(&stderr_read, &stderr_write, &attr, 0)) {
			TODO();	
		}

		if(!SetHandleInformation(stderr_read, HANDLE_FLAG_INHERIT, 0)) {
			TODO();
		}
		si.hStdError = stderr_write;
	}

  PROCESS_INFORMATION pi;
  memset(&pi, 0, sizeof(pi));

  if(!CreateProcessA(NULL,
		    (char *) zh->data,
		    NULL,
		    NULL,
		    redirect_output,
		    0,
		    NULL,
		    NULL,
		    &si,
		    &pi)) {
    char buffer[1024];    
    zh_log(ZH_ERROR, "Cannot start process '%s': %s",
	   zh->data, zh_last_error_cstr(buffer, sizeof(buffer)));
    return 0;
  }

	if(redirect_output) {
		CloseHandle(pi.hThread);

		if(use_stdout) {
			CloseHandle(stdout_write);
			stdout_write = INVALID_HANDLE_VALUE;
		}

		if(use_stderr) {
			CloseHandle(stderr_write);
			stderr_write = INVALID_HANDLE_VALUE;	
		}

		DWORD read;
		u8 buf[1024];
		int success = 0;

		int finished[2];
		finished[0] = use_stdout;
		finished[1] = use_stderr;

		for(;;) {

			if(use_stdout) {
				success = ReadFile(stdout_read, buf, sizeof(buf), &read, NULL);
				if(!success || read == 0) {
					finished[0] = 1;	
				} else {
					zh_da_append_many(sb_out, buf, read);
				}
	
			}

			if(use_stderr) {
				success = ReadFile(stderr_read, buf, sizeof(buf), &read, NULL);
				if(!success || read == 0) {
					finished[1] = 1;
				} else {
					zh_da_append_many(sb_err, buf, read);
				}
	

			}

			if(finished[0] && finished[1]) {
				break;
			}
		}
	}

  WaitForSingleObject(pi.hProcess, INFINITE);
  DWORD code;
  GetExitCodeProcess(pi.hProcess, &code);

  *exit_code = (s32) code;

	if(stdout_read != INVALID_HANDLE_VALUE) CloseHandle(stdout_read);
	if(stdout_write != INVALID_HANDLE_VALUE) CloseHandle(stdout_write);
	if(stderr_read != INVALID_HANDLE_VALUE) CloseHandle(stderr_read);
	if(stderr_write != INVALID_HANDLE_VALUE) CloseHandle(stderr_write);

  return 1;
}

ZH_DEF s32 __zh_run(Zh *zh) {
  s32 exit_code;
  if(!zh_run(zh, &exit_code, NULL, NULL)) {
    return 1;
  } else {
    return exit_code;
  }
}

ZH_DEF void zh_log(Zh_Log_Type type, char *fmt, ...) {
  char *type_cstr = "";
  switch(type) {
#define ZH_LOG_TYPE_X(t) case ZH_##t : type_cstr = #t ; break;
    ZH_LOG_TYPE_XS()
#undef ZH_LOG_TYPE_X
  }
  fprintf(stderr, "[%s] ", type_cstr);

  va_list argv;
  va_start(argv, fmt);

  vfprintf(stderr, fmt, argv);
  fprintf(stderr, "\n");
  fflush(stderr);

  va_end(argv);
}

ZH_DEF char *zh_next(s32 *argc, char ***argv) {
  if((*argc) == 0) return NULL;
  
  char *next = (*argv)[0];

  *argc = (*argc) - 1;
  *argv = (*argv) + 1;

  return next;
}

ZH_DEF int zh_time_get(Zh_Time *time, char *file_path) {
  
  HANDLE handle = CreateFile(file_path, GENERIC_READ, FILE_SHARE_READ,
			     NULL, OPEN_EXISTING, 0, NULL);
  if(handle == INVALID_HANDLE_VALUE) {
    return 0;
  }

  int result = GetFileTime(handle, NULL, NULL, time);  
  CloseHandle(handle);

  if(result) {
    return 1;
  } else {
    return 0;
  }
  
}

ZH_DEF int zh_needs_rebuild_impl_impl(int *needs_rebuild, char *dst, va_list srcs) {

  Zh_Time time_dst, time_src;

  char buffer[1024];

  if(!zh_time_get(&time_dst, dst)) {
    *needs_rebuild = 1;
    return 1;
  }
  
  while(1) {
    char *src = va_arg(srcs, char *);
    if(src == NULL) break;

    if(!zh_time_get(&time_src, src)) {
      zh_log(ZH_ERROR, "Cannot retrieve time of file '%s': %s",
	     src, zh_last_error_cstr(buffer, sizeof(buffer)));
      return 0;
    }

    if(CompareFileTime(&time_src, &time_dst) > 0) {
      *needs_rebuild = 1;
      return 1;
    }
  }

  *needs_rebuild = 0;
  return 1;
}

ZH_DEF int zh_needs_rebuild_impl(int *needs_rebuild, char *dst, ...) {
    
  va_list srcs;
  va_start(srcs, dst);
  int result = zh_needs_rebuild_impl_impl(needs_rebuild, dst, srcs);
  va_end(srcs);  

  return result;
}

ZH_DEF int __zh_needs_rebuild_impl(char *dst, ...) {

  int needs_rebuild;
  
  va_list srcs;
  va_start(srcs, dst);
  int result = zh_needs_rebuild_impl_impl(&needs_rebuild, dst, srcs);
  va_end(srcs);

  if(result) {
    return needs_rebuild;
  } else {
    // i guess, do nothing ?
    return 0;
  }

}

#endif // ZH_IMPLEMENTATION

#undef s32
#undef u64
#undef u8

#endif // ZH_H
