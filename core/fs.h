#ifndef FS_H
#define FS_H

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

#ifndef FS_ALLOC
#  include <stdlib.h>
#  define FS_ALLOC malloc
#endif // FS_ALLOC

#ifndef FS_FREE
#  include <stdlib.h>
#  define FS_FREE free
#endif // FS_FREE

#include <stdio.h>

#ifdef _WIN32
#  include <windows.h>

#  define FS_SEP "\r\n"
#  define FS_DELIM '\\'
#  define FS_MAX_PATH MAX_PATH

#else // _WIN32
#  include <string.h>
#  include <fcntl.h>
#  include <unistd.h>
#  include <errno.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <linux/limits.h>
#  include <time.h>
#  include <dirent.h>

#  define FS_SEP "\n"
#  define FS_DELIM '/'
#  define FS_MAX_PATH PATH_MAX

#endif // _WIN32

#define FS_SEP_LEN sizeof(FS_SEP) - 1

typedef unsigned char Fs_u8;
typedef int Fs_s32;
typedef unsigned int Fs_u32;
typedef unsigned long long int Fs_u64;
#define u8 Fs_u8
#define s32 Fs_s32
#define u32 Fs_u32
#define u64 Fs_u64

#ifndef FS_DEF
#  define FS_DEF static inline
#endif //FS_DEF

typedef u8 Fs_Path[FS_MAX_PATH];

typedef enum {
  FS_ERROR_NONE = 0,

  FS_ERROR_ALLOC_FAILED,

  FS_ERROR_EOF,
  FS_ERROR_FILE_NOT_FOUND,
  FS_ERROR_INVALID_NAME,
  FS_ERROR_ACCESS_DENIED,
} Fs_Error;

FS_DEF Fs_Error fs_error_last();

typedef struct {
  s32 day;
  s32 month;
  s32 year;
  s32 hour;
  s32 min;
  s32 sec;
} Fs_Time;

FS_DEF void fs_time_get(Fs_Time *t);

typedef struct {
#ifdef _WIN32
  HANDLE handle;
#else
  s32 fd;
#endif // _WIN32
  u64 size;
  u64 pos;
} Fs_File;

FS_DEF Fs_Error fs_file_stdin(Fs_File *f);
FS_DEF Fs_Error fs_file_ropen(Fs_File *f,
			      u8 *name,
			      u64 name_len);
#define fs_file_ropenc(f, n) fs_file_ropen((f), (Fs_u8 *) (n), strlen(n))
#define fs_file_ropens(f, s) fs_file_ropen((f), (s).data, (s).len)
FS_DEF Fs_Error fs_file_wopen(Fs_File *f,
			      u8 *name,
			      u64 name_len);
#define fs_file_wopenc(f, n) fs_file_wopen((f), (Fs_u8 *) (n), strlen(n))
#define fs_file_wopens(f, s) fs_file_wopen((f), (s).data, (s).len)

FS_DEF Fs_Error fs_file_read(Fs_File *f,
			     u8 *buf,
			     u64 len,
			     u64 *read);
FS_DEF Fs_Error fs_file_write(Fs_File *f,
			      u8 *buf,
			      u64 len,
			      u64 *written);
FS_DEF Fs_Error fs_file_seek(Fs_File *f, u64 offset);
FS_DEF void fs_file_close(Fs_File *f);

////////////////////////////////////////////////////

#define FS_DIR_ENTRY_IS_DIR 0x1
#define FS_DIR_ENTRY_FROM_SYSTEM 0x2

typedef struct {
  Fs_Path name;
  u64 name_len;

  Fs_Path name_abs;
  u64 name_abs_len;

  u32 flags;
  u64 size;
  Fs_Time time;
} Fs_Dir_Entry;

typedef struct {
  Fs_Path name;
  u64 name_len;

#ifdef _WIN32
  HANDLE handle;
  WIN32_FIND_DATAW find_data;
#else
  DIR *handle;
  struct dirent *ent;
  struct stat stat;
#endif // _WIN32
  Fs_Error error;
} Fs_Dir;

FS_DEF Fs_Error fs_dir_open(Fs_Dir *d,
			    u8 *name,
			    u64 name_len);
#define fs_dir_openc(d, cstr) fs_dir_open((d), (Fs_u8 *) (cstr), strlen(cstr))
#define fs_dir_opens(d, s) fs_dir_open((d), (s).data, (s).len)
FS_DEF Fs_Error fs_dir_next(Fs_Dir *d, Fs_Dir_Entry *e);
FS_DEF void fs_dir_close(Fs_Dir *d);

////////////////////////////////////////////////////

FS_DEF Fs_Error fs_slurp_file(u8 *name,
			      u64 name_len,
			      u8 **data,
			      u64 *data_len);
#define fs_slurp_filec(cstr, d, ds) fs_slurp_file((Fs_u8 *) (cstr), strlen((cstr)), (d), (ds))
#define fs_slurp_files(s, d, ds) fs_slurp_file((s).data, (s).len, (d), (ds))

FS_DEF Fs_Error fs_write_file(u8 *name,
			      u64 name_len,
			      u8 *data,
			      u64 data_len);
#define fs_write_filec(cstr, d, ds) fs_write_file((Fs_u8 *) (cstr), strlen((cstr)), (d), (ds))
#define fs_write_files(s, d, ds) fs_write_file((s).data, (s).len, (d), (ds))

FS_DEF int fs_exists(u8 *name, u64 name_len, int *is_file);
#define fs_existsc(cstr, f) fs_exists((Fs_u8 *) (cstr), strlen(cstr), f)
#define fs_existss(s, f) fs_exists((s).data, (s).len, f)

FS_DEF Fs_Error fs_delete(u8 *name, u64 name_len);
#define fs_deletec(cstr) fs_delete((Fs_u8 *) (cstr), strlen(cstr))
#define fs_deletes(s) fs_delete((s).data, (s).len)

FS_DEF Fs_Error fs_move(u8 *src, u64 src_len, u8 *dst, u64 dst_len);
#define fs_movec(src_cstr, dst_cstr) fs_move((Fs_u8 *) (src_cstr), strlen(cstr), (u8 *) (dst_cstr), strlen(dst_cstr))
#define fs_moves(src_s, dst_s) fs_move((src_s).data, (src_s).len, (dst_s).data, (dst_s).len)

FS_DEF Fs_Error fs_mkdir(u8 *name, u64 name_len);
#define fs_mkdirc(cstr) fs_mkdir((Fs_u8 *) (cstr), strlen(cstr))
#define fs_mkdirs(s) fs_mkdir((s).data, (s).len)

FS_DEF Fs_Error fs_rmdir(u8 *name, u64 name_len);
#define fs_rmdirc(cstr) fs_rmdir((Fs_u8 *) (cstr), strlen(cstr))
#define fs_rmdirs(s) fs_rmdir((s).data, (s).len)


#ifdef FS_IMPLEMENTATION

#ifdef _WIN32

FS_DEF void fs_time_get(Fs_Time *t) {
  SYSTEMTIME time;
  GetSystemTime(&time);
  t->day = time.wDay;
  t->month = time.wMonth;
  t->year = time.wYear;
  t->hour = time.wHour;
  t->min = time.wMinute;
  t->sec = time.wSecond;
}

FS_DEF Fs_Error fs_error_last() {
  DWORD last_error = GetLastError();

  switch(last_error) {
  case 0:
    return FS_ERROR_NONE;
  case 123:
    return FS_ERROR_INVALID_NAME;
  case 2:
  case 3:
    return FS_ERROR_FILE_NOT_FOUND;
  case 5:
    return FS_ERROR_ACCESS_DENIED;
  default:
    fprintf(stderr, "FS_ERROR: Unhandled last_error: %ld\n", last_error); fflush(stderr);
    exit(1);
  }
}

FS_DEF Fs_Error fs_file_stdin(Fs_File *f) {
  f->handle = GetStdHandle(STD_INPUT_HANDLE);
  if(f->handle == INVALID_HANDLE_VALUE) {
    return fs_error_last();
  }

  return FS_ERROR_NONE;
}

FS_DEF Fs_Error fs_file_ropen(Fs_File *f,
			      u8 *name,
			      u64 name_len) {

  wchar_t filepath[MAX_PATH];
  int n = MultiByteToWideChar(CP_UTF8, 0, (char *) name, (s32) name_len, filepath, FS_MAX_PATH);
  filepath[n] = 0;

  f->handle = CreateFileW(filepath,
			  GENERIC_READ,
			  FILE_SHARE_READ,
			  NULL,
			  OPEN_EXISTING,
			  FILE_ATTRIBUTE_NORMAL,
			  NULL);
  if(f->handle == INVALID_HANDLE_VALUE) {
    return fs_error_last();
  }

  f->size = GetFileSize(f->handle, NULL);
  if(f->size == INVALID_FILE_SIZE) {
    CloseHandle(f->handle);
    return fs_error_last();
  }

  f->pos = 0;

  return FS_ERROR_NONE;

}

FS_DEF Fs_Error fs_file_wopen(Fs_File *f,
			      u8 *name,
			      u64 name_len) {

  wchar_t filepath[MAX_PATH];
  int n = MultiByteToWideChar(CP_UTF8, 0, (char *) name, (s32) name_len, filepath, FS_MAX_PATH);
  filepath[n] = 0;

  f->handle = CreateFileW(filepath,
			  GENERIC_WRITE,
			  0,
			  NULL,
			  CREATE_ALWAYS,
			  FILE_ATTRIBUTE_NORMAL,
			  NULL);
  if(f->handle == INVALID_HANDLE_VALUE) {
    return fs_error_last();
  }

  f->pos = 0;
  f->size = INVALID_FILE_SIZE;

  return FS_ERROR_NONE;
}

FS_DEF Fs_Error fs_file_read(Fs_File *f,
			     u8 *buf,
			     u64 len,
			     u64 *read) {

  DWORD bytes_read;

  if(!ReadFile(f->handle, buf, (DWORD) len, &bytes_read, NULL)) {
    return fs_error_last();
  } else {
    *read = (u64) bytes_read;
    f->pos += *read;
    if(bytes_read == 0) {
      *read = 0;
      return FS_ERROR_EOF;
    } else {
      return FS_ERROR_NONE;
    }
  }

}

FS_DEF Fs_Error fs_file_write(Fs_File *f,
			      u8 *buf,
			      u64 len,
			      u64 *written) {

  DWORD bytes_written;
  if(!WriteFile(f->handle, buf, len, &bytes_written, NULL)) {
    return fs_error_last();
  } else {
    *written = (u64) bytes_written;
    f->pos += *written;
    return FS_ERROR_NONE;
  }

}

FS_DEF Fs_Error fs_file_seek(Fs_File *f, u64 offset) {

  f->pos = SetFilePointer(f->handle, (LONG) offset, NULL, FILE_BEGIN);
  if(f->pos == INVALID_SET_FILE_POINTER) {
    return fs_error_last();
  }

  return FS_ERROR_NONE;
}

FS_DEF Fs_Error fs_dir_open(Fs_Dir *d,
			    u8 *name,
			    u64 name_len) {

  wchar_t filepath[FS_MAX_PATH];
  int n = MultiByteToWideChar(CP_UTF8, 0, (char *) name, (s32) name_len, filepath, FS_MAX_PATH);
  filepath[n++] = '*';
  filepath[n] = 0;

  d->handle = FindFirstFileW(filepath, &d->find_data);
  if(d->handle == INVALID_HANDLE_VALUE) {
    return fs_error_last();
  }

  d->error = FS_ERROR_NONE;

  memcpy(d->name, name, name_len);
  d->name_len = name_len;
  d->name[d->name_len] = 0;

  return FS_ERROR_NONE;
}

FS_DEF void fs_file_close(Fs_File *f) {
  CloseHandle(f->handle);
}

FS_DEF Fs_Error fs_dir_next(Fs_Dir *d, Fs_Dir_Entry *e) {

  if(d->error != FS_ERROR_NONE && d->error != FS_ERROR_EOF) {
    return d->error;
  }

  int n = WideCharToMultiByte(CP_UTF8,
			      0,
			      d->find_data.cFileName,
			      -1,
			      (char *) e->name,
			      FS_MAX_PATH,
			      0,
			      NULL);
  e->name_len = (u64) (n - 1);
  e->flags = 0;
  if(d->find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
    e->flags |= FS_DIR_ENTRY_IS_DIR;
  }
  if((e->name_len == 1 && e->name[0] == '.') ||
     (e->name_len == 2 && e->name[0] == '.' && e->name[1] == '.')) {
    e->flags |= FS_DIR_ENTRY_FROM_SYSTEM;
  }
  e->size = ((u64) d->find_data.nFileSizeLow | (((u64) d->find_data.nFileSizeHigh) << 32));

  memcpy(e->name_abs, d->name, d->name_len);
  memcpy(e->name_abs + d->name_len, e->name, e->name_len);
  e->name_abs_len = d->name_len + e->name_len;
  e->name_abs[e->name_abs_len] = 0;

  SYSTEMTIME time;
  FileTimeToSystemTime(&d->find_data.ftLastWriteTime,
		       &time);
  e->time.day = time.wDay;
  e->time.month = time.wMonth;
  e->time.year = time.wYear;
  e->time.hour = time.wHour;
  e->time.min = time.wMinute;
  e->time.sec = time.wSecond;

  if(d->error == FS_ERROR_EOF) {
    return FS_ERROR_EOF;
  }

  if(FindNextFileW(d->handle,
		   &d->find_data)) {
    d->error = FS_ERROR_NONE;
    return d->error;

  } else {
    if(GetLastError() == ERROR_NO_MORE_FILES) {
      d->error = FS_ERROR_EOF;
      return FS_ERROR_NONE;
    } else {
      d->error = fs_error_last();
      return d->error;
    }

  }

}

FS_DEF void fs_dir_close(Fs_Dir *d) {
  FindClose(d->handle);
}

FS_DEF int fs_exists(u8 *name, u64 name_len, int *is_file) {
  wchar_t filepath[MAX_PATH];
  int n = MultiByteToWideChar(CP_UTF8, 0, (char *) name, (s32) name_len, filepath, FS_MAX_PATH);
  filepath[n] = 0;

  DWORD attribs = GetFileAttributesW(filepath);
  if(is_file) *is_file = !(attribs & FILE_ATTRIBUTE_DIRECTORY);
  return attribs != INVALID_FILE_ATTRIBUTES;
}

FS_DEF Fs_Error fs_delete(u8 *name, u64 name_len) {
  wchar_t filepath[MAX_PATH];
  int n = MultiByteToWideChar(CP_UTF8, 0, (char *) name, (s32) name_len, filepath, FS_MAX_PATH);
  filepath[n] = 0;

  if(DeleteFileW(filepath)) {
    return FS_ERROR_NONE;
  } else {
    return fs_error_last();
  }
}

FS_DEF Fs_Error fs_move(u8 *src, u64 src_len, u8 *dst, u64 dst_len) {
  wchar_t src_filepath[MAX_PATH];
  int n = MultiByteToWideChar(CP_UTF8, 0, (char *) src, (s32) src_len, src_filepath, FS_MAX_PATH);
  src_filepath[n] = 0;

  wchar_t dst_filepath[MAX_PATH];
  n = MultiByteToWideChar(CP_UTF8, 0, (char *) dst, (s32) dst_len, dst_filepath, FS_MAX_PATH);
  dst_filepath[n] = 0;

  if(MoveFileW(src_filepath, dst_filepath)) {
    return FS_ERROR_NONE;
  } else {
    return fs_error_last();
  }
}

FS_DEF Fs_Error fs_mkdir(u8 *name, u64 name_len) {
  wchar_t filepath[MAX_PATH];
  int n = MultiByteToWideChar(CP_UTF8, 0, (char *) name, (s32) name_len, filepath, FS_MAX_PATH);
  filepath[n] = 0;

  if(CreateDirectoryW(filepath,
		      NULL)) {
    return FS_ERROR_NONE;
  } else {
    return fs_error_last();
  }
}

FS_DEF Fs_Error fs_rmdir(u8 *name, u64 name_len) {
  wchar_t filepath[MAX_PATH];
  int n = MultiByteToWideChar(CP_UTF8, 0, (char *) name, (s32) name_len, filepath, FS_MAX_PATH);
  filepath[n] = 0;

  if(RemoveDirectoryW(filepath)) {
    return FS_ERROR_NONE;
  } else {
    return fs_error_last();
  }
}



#else // _WIN32

FS_DEF Fs_Error fs_error_last() {
  switch(errno) {
  case 2:
    return FS_ERROR_FILE_NOT_FOUND;
  default:
    fprintf(stderr, "FS_ERROR: Unhandled last_error: %d\n", errno);
    fprintf(stderr, "FS_ERROR: '%s'\n", strerror(errno));
    fflush(stderr);
    exit(1);
  }
}

FS_DEF void fs_time_get(Fs_Time *t) {
  time_t _t = time(NULL);
  struct tm *tm = localtime(&_t);
  t->day = tm->tm_mday;
  t->month = tm->tm_mon;
  t->year = tm->tm_year;
  t->hour = tm->tm_hour;
  t->min = tm->tm_min;
  t->sec = tm->tm_sec;
}

FS_DEF Fs_Error fs_file_stdin(Fs_File *f) {
  f->fd = STDIN_FILENO;
  return FS_ERROR_NONE;
}

FS_DEF Fs_Error fs_file_ropen(Fs_File *f,
			      u8 *name,
			      u64 name_len) {
  u8 buf[FS_MAX_PATH];
  memcpy(buf, name, name_len);
  buf[name_len] = 0;

  f->fd = open((char *) buf, O_RDONLY);
  if(f->fd < 0) {
    return fs_error_last();
  }

  struct stat stats;
  if(stat((char *) buf, &stats) < 0) {
    close(f->fd);
    return fs_error_last();
  }

  f->size = (u64) stats.st_size;
  f->pos  = 0;

  return FS_ERROR_NONE;
}

FS_DEF Fs_Error fs_file_read(Fs_File *f,
			     u8 *buf,
			     u64 len,
			     u64 *_read) {
  ssize_t ret = read(f->fd, buf, len);
  if(ret < 0) {
    return fs_error_last();
  } else if(ret == 0) {
    *_read = 0;
    return FS_ERROR_EOF;
  } else {
    *_read = (u64) ret;
    f->pos += *_read;
    return FS_ERROR_NONE;
  }
}

FS_DEF Fs_Error fs_file_wopen(Fs_File *f,
			      u8 *name,
			      u64 name_len) {
  u8 buf[FS_MAX_PATH];
  memcpy(buf, name, name_len);
  buf[name_len] = 0;

  f->fd = open((char *) buf, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IROTH | S_IRGRP);
  if(f->fd < 0) {
    return fs_error_last();
  }

  return FS_ERROR_NONE;
}

FS_DEF Fs_Error fs_file_write(Fs_File *f,
			      u8 *buf,
			      u64 len,
			      u64 *written) {
  ssize_t ret = write(f->fd, buf, len);
  if(ret < 0) {
    return fs_error_last();
  } else {
    *written = (u64) ret;
    return FS_ERROR_NONE;
  }
}

FS_DEF Fs_Error fs_file_seek(Fs_File *f, u64 offset) {

  off_t pos = lseek(f->fd, (off_t) offset, SEEK_SET);
  if(pos == -1) {
    return fs_error_last();
  } else {
    f->pos = (u64) pos;
    return FS_ERROR_NONE;
  }

}

FS_DEF int fs_exists(u8 *name, u64 name_len, int *is_file) {

  u8 buf[FS_MAX_PATH];
  memcpy(buf, name, name_len);
  buf[name_len] = 0;

  int result = access((char *) buf, F_OK);
  if(result < 0) {
    return 0;
  }

  if(is_file) {
    struct stat path_stat;
    stat((char *) buf, &path_stat);
    *is_file = S_ISREG(path_stat.st_mode) != 0;
  }

  return 1;

}

FS_DEF Fs_Error fs_delete(u8 *name, u64 name_len) {
  u8 buf[FS_MAX_PATH];
  memcpy(buf, name, name_len);
  buf[name_len] = 0;

  if(remove((char *) buf) == 0) {
    return FS_ERROR_NONE;
  } else {
    return fs_error_last();
  }
}

FS_DEF Fs_Error fs_move(u8 *src, u64 src_len, u8 *dst, u64 dst_len) {
  u8 src_filepath[FS_MAX_PATH];
  memcpy(src_filepath, src, src_len);
  src_filepath[src_len] = 0;

  u8 dst_filepath[FS_MAX_PATH];
  memcpy(dst_filepath, dst, dst_len);
  dst_filepath[dst_len] = 0;

  if(rename((char *) src, (char *) dst) == 0) {
    return FS_ERROR_NONE;
  } else {
    return fs_error_last();
  }
}

FS_DEF Fs_Error fs_mkdir(u8 *name, u64 name_len) {
  u8 buf[FS_MAX_PATH];
  memcpy(buf, name, name_len);
  buf[name_len] = 0;

  if(mkdir((char *) buf, 777) == 0) {
    return FS_ERROR_NONE;
  } else {
    return fs_error_last();
  }
}

FS_DEF Fs_Error fs_rmdir(u8 *name, u64 name_len) {
  u8 buf[FS_MAX_PATH];
  memcpy(buf, name, name_len);
  buf[name_len] = 0;

  if(rmdir((char *) buf) == 0) {
    return FS_ERROR_NONE;
  } else {
    return fs_error_last();
  }
}

FS_DEF void fs_file_close(Fs_File *f) {
  close(f->fd);
}

FS_DEF Fs_Error fs_dir_open(Fs_Dir *d,
			    u8 *name,
			    u64 name_len) {
  u8 buf[FS_MAX_PATH];
  memcpy(buf, name, name_len);
  buf[name_len] = 0;

  d->handle = opendir((char *) buf);
  if(!d->handle) {
    return fs_error_last();
  }

  memcpy(d->name, name, name_len);
  d->name_len = name_len;
  d->name[d->name_len] = 0;

  return FS_ERROR_NONE;
}

FS_DEF Fs_Error fs_dir_next(Fs_Dir *d, Fs_Dir_Entry *e) {

  errno = 0;
  d->ent = readdir(d->handle);
  if(!d->ent) {
    if(errno == 0) {
      d->error = FS_ERROR_EOF;
    } else {
      d->error = fs_error_last();
    }
    return d->error;
  }

  e->name_len = strlen(d->ent->d_name);
  memcpy(e->name, d->ent->d_name, e->name_len + 1);

  memcpy(e->name_abs, d->name, d->name_len);
  memcpy(e->name_abs + d->name_len, e->name, e->name_len);
  e->name_abs_len = d->name_len + e->name_len;
  e->name_abs[e->name_abs_len] = 0;

  if(stat((char *) e->name_abs, &d->stat) < 0) {
    e->time = (Fs_Time) {0};
    e->size = 0;
  } else {
    struct tm *tm = localtime(&d->stat.st_mtim.tv_sec);
    e->time.day = tm->tm_mday;
    e->time.month = tm->tm_mon;
    e->time.year = tm->tm_year;
    e->time.hour = tm->tm_hour;
    e->time.min = tm->tm_min;
    e->time.sec = tm->tm_sec;

    e->size = d->stat.st_size;
  }

  e->flags = 0;
  if((d->ent->d_type == DT_DIR) != 0) {
    e->flags |= FS_DIR_ENTRY_IS_DIR;
  }

  if((e->name_len == 1 && e->name[0] == '.') ||
     (e->name_len == 2 && e->name[0] == '.' && e->name[1] == '.')) {
    e->flags |= FS_DIR_ENTRY_FROM_SYSTEM;
  }

  d->error = FS_ERROR_NONE;
  return FS_ERROR_NONE;
}

FS_DEF void fs_dir_close(Fs_Dir *d) {
  closedir(d->handle);
}


#endif // _WIN32

FS_DEF Fs_Error fs_slurp_file(u8 *name,
			      u64 name_len,
			      u8 **_data,
			      u64 *_data_len) {
  Fs_Error error;

  Fs_File file;
  error = fs_file_ropen(&file, name, name_len);
  if(error != FS_ERROR_NONE) {
    return error;
  }

  u8 *data = FS_ALLOC(file.size);
  if(!data) {
    fs_file_close(&file);
    return FS_ERROR_ALLOC_FAILED;
  }

  u64 data_len = 0;
  while(data_len < file.size) {

    u64 read;
    error = fs_file_read(&file, data + data_len, file.size - data_len, &read);
    if(error != FS_ERROR_NONE) {
      fs_file_close(&file);
      FS_FREE(data);
      return error;
    }

    data_len += read;

  }

  *_data     = data;
  *_data_len = data_len;

  return FS_ERROR_NONE;
}

FS_DEF Fs_Error fs_write_file(u8 *name,
			      u64 name_len,
			      u8 *data,
			      u64 data_len) {
  Fs_Error error;

  Fs_File file;
  error = fs_file_wopen(&file, name, name_len);
  if(error != FS_ERROR_NONE) {
    return error;
  }

  u64 data_written = 0;
  while(data_written < data_len) {

    u64 written;
    error = fs_file_write(&file,
			  data + data_written,
			  data_len - data_written,
			  &written);
    if(error != FS_ERROR_NONE) {
      fs_file_close(&file);
      return error;
    }
    data_written += written;

  }

  fs_file_close(&file);

  return FS_ERROR_NONE;

}

#endif //FS_IMPLEMENTATION

#undef u8
#undef s32
#undef u32
#undef u64

#endif //FS_H
