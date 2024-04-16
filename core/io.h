#ifndef IO_H
#define IO_H

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

#ifndef IO_ALLOC
#  include <stdlib.h>
#  define IO_ALLOC malloc
#endif // IO_ALLOC

#ifndef IO_FREE
#  include <stdlib.h>
#  define IO_FREE free
#endif // IO_FREE

#ifdef _WIN32
#  include <windows.h>

#  define IO_SEP "\r\n"
#  define IO_DELIM '\\'
#  define IO_MAX_PATH MAX_PATH

#else // _WIN32
#  include <string.h>
#  include <fcntl.h>
#  include <unistd.h>
#  include <errno.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <linux/limits.h>
#  include <time.h>

#  define IO_SEP "\n"
#  define IO_DELIM '/' 
#  define IO_MAX_PATH PATH_MAX

#endif // _WIN32

#define IO_SEP_LEN sizeof(IO_SEP) - 1

typedef unsigned char Io_u8;
typedef int Io_s32;
typedef unsigned long long int Io_u64;
#define u8 Io_u8
#define s32 Io_s32
#define u64 Io_u64

#ifndef IO_DEF
#  define IO_DEF static inline
#endif //IO_DEF

typedef enum {
  IO_ERROR_NONE = 0,

  IO_ERROR_ALLOC_FAILED,
  
  IO_ERROR_EOF,
  IO_ERROR_FILE_NOT_FOUND,
  IO_ERROR_INVALID_NAME,
} Io_Error;

IO_DEF Io_Error io_error_last();

typedef struct {
  s32 day;
  s32 month;
  s32 year;
  s32 hour;
  s32 min;
  s32 sec;
} Io_Time;

IO_DEF void io_time_get(Io_Time *t);

typedef struct {
#ifdef _WIN32
  HANDLE handle;
#else  
  s32 fd;
#endif // _WIN32
  u64 size;
  u64 pos;
} Io_File;

IO_DEF Io_Error io_file_stdin(Io_File *f);
IO_DEF Io_Error io_file_ropen(Io_File *f,
			      u8 *name,
			      u64 name_len);
#define io_file_ropenc(f, n) io_file_ropen((f), (n), strlen(n))
#define io_file_ropens(f, s) io_file_ropen((f), (s).data, (s).len)

IO_DEF Io_Error io_file_read(Io_File *f,
			     u8 *buf,
			     u64 len,
			     u64 *read);
  
IO_DEF void io_file_close(Io_File *f);


IO_DEF Io_Error io_slurp_file(u8 *name,
			      u64 name_len,
			      u8 **data,
			      u64 *data_len);
#define io_slurp_filec(cstr, d, ds) io_slurp_file((cstr), strlen((cstr)), (d), (ds))
#define io_slurp_files(cstr, d, ds) io_slurp_file((s).data, (s).data, (d), (ds))

#ifdef IO_IMPLEMENTATION

#ifdef _WIN32

IO_DEF void io_time_get(Io_Time *t) {
  SYSTEMTIME time;
  GetSystemTime(&time);
  t->day = time.wDay;
  t->month = time.wMonth;
  t->year = time.wYear;
  t->hour = time.wHour;
  t->min = time.wMinute;
  t->sec = time.wSecond;
}

IO_DEF Io_Error io_error_last() {
  DWORD last_error = GetLastError();
  
  switch(last_error) {
  case 0:
    return IO_ERROR_NONE;
  case 123:
    return IO_ERROR_INVALID_NAME;
  case 2:
    return IO_ERROR_FILE_NOT_FOUND;
  default:
    fprintf(stderr, "IO_ERROR: Unhandled last_error: %ld\n", last_error); fflush(stderr);
    exit(1);
  }
}

IO_DEF Io_Error io_file_stdin(Io_File *f) {
  f->handle = GetStdHandle(STD_INPUT_HANDLE);
  if(f->handle == INVALID_HANDLE_VALUE) {
    return io_error_last();
  }

  return IO_ERROR_NONE;
}

IO_DEF Io_Error io_file_ropen(Io_File *f,
			      u8 *name,
			      u64 name_len) {
  
  wchar_t filepath[MAX_PATH];
  int n = MultiByteToWideChar(CP_UTF8, 0, (char *) name, name_len, filepath, IO_MAX_PATH);
  filepath[n] = 0;

  f->handle = CreateFileW(filepath,
			  GENERIC_READ,
			  FILE_SHARE_READ,
			  NULL,
			  OPEN_EXISTING,
			  FILE_ATTRIBUTE_NORMAL,
			  NULL);
  if(f->handle == INVALID_HANDLE_VALUE) {
    return io_error_last();
  }

  f->size = GetFileSize(f->handle, NULL);
  if(f->size == INVALID_FILE_SIZE) {
    CloseHandle(f->handle);
    return io_error_last();
  }

  f->pos = 0;

  return IO_ERROR_NONE;
  
}

IO_DEF Io_Error io_file_read(Io_File *f,
			     u8 *buf,
			     u64 len,
			     u64 *read) {

  DWORD bytes_read;
  
  if(!ReadFile(f->handle, buf, len, &bytes_read, NULL)) {
    return io_error_last();
  } else {    
    *read = (u64) bytes_read;
    f->pos += *read;
    
    if(bytes_read == 0) {
      return IO_ERROR_EOF;
    } else {      
      return IO_ERROR_NONE;
    }
  }
  
}

IO_DEF void io_file_close(Io_File *f) {
  
  CloseHandle(f->handle);
  
}

#else // _WIN32

IO_DEF Io_Error io_error_last() {
    switch(errno) {
    case 2:
      return IO_ERROR_FILE_NOT_FOUND;
    default:
      fprintf(stderr, "IO_ERROR: Unhandled last_error: %d\n", errno);
      fprintf(stderr, "IO_ERROR: '%s'\n", strerror(errno));
      fflush(stderr);
      exit(1);
    }
}

IO_DEF void io_time_get(Io_Time *t) {
  time_t _t = time(NULL);
  struct tm *tm = localtime(&_t);
  t->day = tm->tm_mday;
  t->month = tm->tm_mon;
  t->year = tm->tm_year;
  t->hour = tm->tm_hour;
  t->min = tm->tm_min;
  t->sec = tm->tm_sec;
}

IO_DEF Io_Error io_file_stdin(Io_File *f) {
  f->fd = STDIN_FILENO;
  return IO_ERROR_NONE;
}

IO_DEF Io_Error io_file_ropen(Io_File *f,
			      u8 *name,
			      u64 name_len) {
  u8 buf[IO_MAX_PATH];
#include <string.h>
  memcpy(buf, name, name_len);
  buf[name_len] = 0;

  f->fd = open(buf, O_RDONLY);
  if(f->fd < 0) {
    return io_error_last();
  }
  
  struct stat stats;
  if(stat(buf, &stats) < 0) {
    close(f->fd);
    return io_error_last();
  }

  f->size = (u64) stats.st_size;
  f->pos  = 0;

  return IO_ERROR_NONE;
}

IO_DEF Io_Error io_file_read(Io_File *f,
			     u8 *buf,
			     u64 len,
			     u64 *_read) {
  ssize_t ret = read(f->fd, buf, len);
  if(ret < 0) {
    return io_error_last();
  } else if(ret == 0) {
    return IO_ERROR_EOF;
  } else {
    *_read = (u64) ret;
    f->pos += *_read;
    return IO_ERROR_NONE;
  }
}

IO_DEF void io_file_close(Io_File *f) {
  close(f->fd);
}

#endif // _WIN32

IO_DEF Io_Error io_slurp_file(u8 *name,
			      u64 name_len,
			      u8 **_data,
			      u64 *_data_len) {
  Io_Error error;
  
  Io_File file;
  error = io_file_ropen(&file, name, name_len);
  if(error != IO_ERROR_NONE) {
    return error;
  }

  u8 *data = IO_ALLOC(file.size);
  if(!data) {
    io_file_close(&file);
    return IO_ERROR_ALLOC_FAILED;
  }  

  u64 data_len = 0;  
  while(data_len < file.size) {

    u64 read;
    error = io_file_read(&file, data + data_len, file.size - data_len, &read);
    if(error != IO_ERROR_NONE) {
      io_file_close(&file);
      IO_FREE(data);
      return error;
    }

    data_len += read;
    
  }

  *_data     = data;
  *_data_len = data_len;
  
  return IO_ERROR_NONE;
}

#endif //IO_IMPLEMENTATION

#undef u8
#undef s32
#undef u64

#endif //IO_H

