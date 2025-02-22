#ifndef PNM_H
#define PNM_H

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

typedef unsigned char Pnm_u8;
typedef unsigned int Pnm_u32;
typedef unsigned long long int Pnm_u64;

#define u8 Pnm_u8
#define u32 Pnm_u32
#define u64 Pnm_u64

#ifndef PNM_BUFFER_CAP
#  define PNM_BUFFER_CAP 2048
#endif // PNM_BUFFER_CAP

#ifndef PNM_DEF
#  define PNM_DEF static inline
#endif // PNM_DEF

#ifndef PNM_ASSERT
#  include <assert.h>
#  define PNM_ASSERT assert
#endif // PNM_ASSERT

#ifndef PNM_MALLOC
#  include <stdlib.h>
#  define PNM_MALLOC malloc
#endif // PNM_MALLOC

#ifndef PNM_FREE
#include <stdlib.h>
#  define PNM_FREE free
#endif // PNM_FREE

#if defined(PNM_MALLOC) && defined(PNM_FREE)
// ok
#elif !defined(PNM_MALLOC) && !defined(PNM_FREE)
// ok
#else
#  error "Must define all or none of PNM_MALLOC and PMN_FREE"
#endif 

#ifndef PNM_NO_STDIO
#  ifdef _WIN32
#    include <windows.h>
typedef HANDLE Pnm_Fd;
#  else // linux
#  include <fcntl.h>
#  include <sys/stat.h>
#  include <unistd.h>
typedef int Pnm_Fd;
#  endif // _WIN32
#endif // PNM_NO_STDIO

PNM_DEF int pnm_is_digit(u8 c);
PNM_DEF int pnm_is_whitespace(u8 c);
PNM_DEF size_t pnm_strlen(const char *cstr);

typedef enum {
	PNM_FORMAT_NONE = 0,
	PNM_FORMAT_P1,
	PNM_FORMAT_P2,
	PNM_FORMAT_P3,   
} Pnm_Format;

typedef enum {
	PNM_ERROR_NONE = 0,
	PNM_ERROR_UNIMPLEMENTED,
	PNM_ERROR_IO,
	PNM_ERROR_EOF,
	PNM_ERROR_INVALID_INPUT,
	PNM_ERROR_INVALID_FORMAT,
	PNM_ERROR_UNSUPPORTED_VERSION,
	PNM_ERROR_UNSUPPORTED_MAX_VALUE,
	PNM_ERROR_NO_MEMORY,
}Pnm_Error;

typedef Pnm_Error (*pnm_read_callback)(void *user, u8 *buffer, u64 buffer_size, u64 *written);
typedef Pnm_Error (*pnm_write_callback)(void *user, const u8 *buffer, u64 buffer_size);

typedef struct{
	union{
		pnm_read_callback read;
		pnm_write_callback write;
	}as;
	void *userdata;
}Pnm_Callbacks;

typedef struct{
	const u8 *data;
	u64 len;
	u64 pos;
}Pnm_Memory;

#ifndef PNM_NO_STDIO

typedef struct{
	Pnm_Fd fd;
	u64 len;
	u64 pos;
}Pnm_File;

PNM_DEF int pnm_file_init(Pnm_File *f, const char *filepath, int for_reading);
PNM_DEF void pnm_file_free(Pnm_File *f);

#endif // PNM_NO_STDIO

typedef enum {
	PNM_MODE_NONE = 0,

#ifndef PNM_NO_STDIO
	PNM_MODE_FILE,
#endif //PNM_NO_STDIO

	PNM_MODE_MEMORY,
	PNM_MODE_CALLBACKS,
}Pnm_Mode;

typedef struct{
	Pnm_Error error;
	Pnm_Mode mode;
	union {
#ifndef PNM_NO_STDIO
		Pnm_File file;
#endif // PNM_NO_STDIO
		Pnm_Memory memory;
		Pnm_Callbacks callbacks;
	}as;

	u8 buf[PNM_BUFFER_CAP];
	u64 buf_off;
	u64 buf_len;
}Pnm_Reader;

PNM_DEF u8 pnm_reader_u8(Pnm_Reader *r);
PNM_DEF u8 pnm_reader_peek_u8(Pnm_Reader *r);
PNM_DEF void pnm_reader_skip_whitespace(Pnm_Reader *r);
PNM_DEF u32 pnm_reader_parse_u32(Pnm_Reader *r);
PNM_DEF void pnm_reader_parse_cstr(Pnm_Reader *r, const char *cstr);
PNM_DEF int pnm_reader_parse_pam_tupletype(Pnm_Reader *r);
PNM_DEF u32 pnm_reader_parse_cstr_u32(Pnm_Reader *r, const char *cstr);

PNM_DEF int pnm_reader_info(Pnm_Reader *r, int *width, int *height, int *channels);
PNM_DEF void *pnm_reader_decode(Pnm_Reader *r, int *width, int *height, int *channels, int desired_channels);

typedef struct{
	Pnm_Error error;
	Pnm_Mode mode;
	union{
#ifndef PNM_NO_STDIO
		Pnm_File file;
#endif // PNM_NO_STDIO
		Pnm_Callbacks callbacks;
	}as;

	u8 buf[PNM_BUFFER_CAP];
	u64 buf_len;
}Pnm_Writer;

PNM_DEF void pnm_writer_write(Pnm_Writer *w, const u8 *buf, u64 buf_len);
PNM_DEF void pnm_writer_format_cstr(Pnm_Writer *w, const char *cstr);
PNM_DEF void pnm_writer_format_u32(Pnm_Writer *w, u32 n);
PNM_DEF void pnm_writer_flush(Pnm_Writer *w);

PNM_DEF void pnm_writer_write_header(Pnm_Writer *w, u32 width, u32 height, u32 comp);
PNM_DEF void pnm_writer_encode(Pnm_Writer *w, int width, int height, int comp, const void *data);

// load-functions

#ifndef PNM_NO_STDIO
PNM_DEF int pnm_info(const char *filepath, int *width, int *height, int *channels);
PNM_DEF void *pnm_load(const char *filepath, int *width, int *height, int *channels, int desired_channels);
PNM_DEF int pnm_write(const char *filepath, int width, int height, int comp, const void *data);
#endif // PNM_NO_STDIO

PNM_DEF int pnm_info_from_memory(const unsigned char *data, u64 data_len, int *width, int *height, int *channels);
PNM_DEF void *pnm_load_from_memory(const unsigned char *data, u64 data_len, int *width, int *height, int *channels, int desired_channels);

PNM_DEF int pnm_info_from_callbacks(void *userdata, pnm_read_callback read, int *width, int *height, int *channels);
PNM_DEF void *pnm_load_from_callbacks(void *userdata, pnm_read_callback read, int *width, int *height, int *channels, int desired_channels);
PNM_DEF int pnm_write_to_callbacks(void *userdata, pnm_write_callback write, int width, int height, int comp, const void *data);

#ifdef PNM_IMPLEMENTATION

PNM_DEF int pnm_is_digit(u8 c) {
	return '0' <= c && c <= '9';
}

PNM_DEF int pnm_is_whitespace(u8 c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

PNM_DEF size_t pnm_strlen(const char *cstr) {
	size_t result = 0;
	while(*cstr++) result++;
	return result;
}

#ifndef PNM_NO_STDIO

PNM_DEF int pnm_file_init(Pnm_File *f, const char *filepath, int for_reading) {
#ifdef _WIN32
	if(for_reading) {
		f->fd = CreateFile(filepath, GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
		if(f->fd == INVALID_HANDLE_VALUE) {
			return 0;
		}

		f->len = (u64) GetFileSize(f->fd, NULL);
		if(f->len == INVALID_FILE_SIZE) {
			CloseHandle(f->fd);
			return 0;
		}

		f->pos = 0;

		return 1;
	} else {
		f->fd = CreateFile(filepath,
				GENERIC_WRITE, 0, NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
		if(f->fd == INVALID_HANDLE_VALUE) {
			return 0;
		}

		f->pos = 0;
		f->len = 0;

		return 1;
	}

#else // linux

	if(for_reading) {
		f->fd = open(filepath, O_RDONLY);
		if(f->fd <= 0) {
			return 0;
		}

		struct stat stats;
		if(stat(filepath, &stats) < 0) {
			close(f->fd);
			return 0;
		}

		f->pos = 0;
		f->len = (u64) stats.st_size;
	} else {
		f->fd = open(filepath, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IROTH | S_IRGRP);
		if(f->fd <= 0) {
			return 0;
		}

		f->pos = 0;
		f->len = 0;
	}

	return 1;

#endif // _WIN32
}

PNM_DEF void pnm_file_free(Pnm_File *f) {
#ifdef _WIN32
	CloseHandle(f->fd);

#else // linux
	close(f->fd);
#endif // _WIN32
}

PNM_DEF int pnm_info(const char *filepath, int *width, int *height, int *channels) {
	Pnm_Reader reader;
	if(!pnm_file_init(&reader.as.file, filepath, 1)) {
		return 0;
	}
	reader.mode = PNM_MODE_FILE;
	reader.error = PNM_ERROR_NONE;
	reader.buf_len = 0;

	int result = pnm_reader_info(&reader, width, height, channels);
	pnm_file_free(&reader.as.file);

	return result;
}

PNM_DEF void *pnm_load(const char *filepath, int *width, int *height, int *channels, int desired_channels) {

	Pnm_Reader reader;
	if(!pnm_file_init(&reader.as.file, filepath, 1)) {
		return NULL;
	}
	reader.mode = PNM_MODE_FILE;
	reader.error = PNM_ERROR_NONE;
	reader.buf_len = 0;

	unsigned char *data = pnm_reader_decode(&reader, width, height, channels, desired_channels);
	pnm_file_free(&reader.as.file);

	return data;
}

PNM_DEF int pnm_write(const char *filepath, int width, int height, int comp, const void *data) {
	Pnm_Writer writer;
	if(!pnm_file_init(&writer.as.file, filepath, 0)) {
		return 0;
	}
	writer.mode = PNM_MODE_FILE;
	writer.error = PNM_ERROR_NONE;
	writer.buf_len = 0;

	pnm_writer_encode(&writer, width, height, comp, data);

	pnm_file_free(&writer.as.file);

	return writer.error == 0;
}

#endif //  PNM_NO_STDIO

PNM_DEF int pnm_info_from_memory(const unsigned char *memory, u64 memory_len, int *width, int *height, int *channels) {
	Pnm_Reader reader;
	reader.as.memory = (Pnm_Memory) {
		memory,
			memory_len,
			0,
	};
	reader.mode = PNM_MODE_MEMORY;
	reader.error = PNM_ERROR_NONE;
	reader.buf_len = 0;

	return pnm_reader_info(&reader, width, height, channels);
}

PNM_DEF void *pnm_load_from_memory(const unsigned char *memory, u64 memory_len, int *width, int *height, int *channels, int desired_channels) {

	Pnm_Reader reader;
	reader.as.memory = (Pnm_Memory) {
		memory,
			memory_len,
			0,
	};
	reader.mode = PNM_MODE_MEMORY;
	reader.error = PNM_ERROR_NONE;
	reader.buf_len = 0;

	return pnm_reader_decode(&reader, width, height, channels, desired_channels);  
}

PNM_DEF int pnm_info_from_callbacks(void* userdata, pnm_read_callback read, int *width, int *height, int *channels) {
	Pnm_Reader reader;
	reader.as.callbacks = (Pnm_Callbacks) {
		{ read },
			userdata,
	};
	reader.mode = PNM_MODE_CALLBACKS;
	reader.error = PNM_ERROR_NONE;
	reader.buf_len = 0;

	return pnm_reader_info(&reader, width, height, channels);
}

PNM_DEF void *pnm_load_from_callbacks(void *userdata, pnm_read_callback read, int *width, int *height, int *channels, int desired_channels) {
	Pnm_Reader reader;
	reader.as.callbacks = (Pnm_Callbacks) {
		.userdata = userdata,
			.as.read = read,
	};
	reader.mode = PNM_MODE_CALLBACKS;
	reader.error = PNM_ERROR_NONE;
	reader.buf_len = 0;

	return pnm_reader_decode(&reader, width, height, channels, desired_channels);  

}

PNM_DEF int pnm_write_to_callbacks(void *userdata, pnm_write_callback write, int width, int height, int comp, const void *data) {
	Pnm_Writer writer;
	writer.as.callbacks = (Pnm_Callbacks) {
		.userdata = userdata,
			.as.write = write,
	};
	writer.mode = PNM_MODE_CALLBACKS;
	writer.error = PNM_ERROR_NONE;  
	writer.buf_len = 0;

	pnm_writer_encode(&writer, width, height, comp, data);

	return writer.error == 0;
}

PNM_DEF u8 pnm_reader_u8(Pnm_Reader *r) {
	if(r->error) return 0;
	
	switch(r->mode) {

#ifndef PNM_NO_STDIO
		case PNM_MODE_FILE: {

			Pnm_File *f = &r->as.file;
			if(r->buf_len == 0) {
				u64 remaining = f->len - f->pos;
				if(remaining == 0) {
					r->error = PNM_ERROR_EOF;
					return 0;
				}
				if(remaining > PNM_BUFFER_CAP) remaining = PNM_BUFFER_CAP;

#ifdef _WIN32 
				DWORD read = 0;
				if(!ReadFile(f->fd, r->buf, (DWORD) remaining, &read, NULL)) {
					r->error = PNM_ERROR_IO;
					return 0;
				}
				if(read == 0) {
					r->error = PNM_ERROR_EOF;
					return 0;
				}
				f->pos += read;

				r->buf_off = 0;
				r->buf_len = (u64) read;

#else // linux
				int n = read(f->fd, r->buf, remaining);
				if(n < 0) {
					r->error = PNM_ERROR_IO;
					return 0;
				}
				if(n == 0) {
					r->error = PNM_ERROR_EOF;
					return 0;
				}
				u64 read = (u64) n;
				f->pos += read;

				r->buf_off = 0;
				r->buf_len = read;
#endif //_WIN32
			}

			u8 b = r->buf[r->buf_off];
			r->buf_off++;
			r->buf_len--;
			return b;

		} break;
#endif // PNM_NO_STDIO

		case PNM_MODE_MEMORY: {

			Pnm_Memory *m = &r->as.memory;
			if(m->pos >= m->len) {
				r->error = PNM_ERROR_EOF;
				return 0;
			}

			return m->data[m->pos++];
		} break;

		case PNM_MODE_CALLBACKS: {

			Pnm_Callbacks *c = &r->as.callbacks;
			if(r->buf_len == 0) {

				u64 read;
				for(;;) {	
					Pnm_Error error = c->as.read(c->userdata, r->buf, PNM_BUFFER_CAP, &read);
					if(error != PNM_ERROR_NONE) {
						r->error = error;
						return 0;
					}

					if(read > 0) break;
				}
				r->buf_off = 0;
				r->buf_len = read;

			}

			r->buf_len--;
			return r->buf[r->buf_off++];
		} break;

		default: {
			PNM_ASSERT(!"unreachable");
			return 0;
		} break;
	}

}

PNM_DEF u8 pnm_reader_peek_u8(Pnm_Reader *r) {
	if(r->error) return 0;

	u8 b = pnm_reader_u8(r);

	switch(r->mode) {

#ifndef PNM_NO_STDIO
		case PNM_MODE_FILE: {
			PNM_ASSERT(r->buf_off > 0);
			r->buf_off--;
			r->buf_len++;

		} break;
#endif // PNM_NO_STDIO

		case PNM_MODE_MEMORY: {
			PNM_ASSERT(r->as.memory.pos > 0);
			r->as.memory.pos--;

		} break;

		case PNM_MODE_CALLBACKS: {
			PNM_ASSERT(r->buf_off > 0);
			r->buf_off--;
			r->buf_len++;

		} break;

		default: {
			PNM_ASSERT(!"unreachable");
			return 0;
		} break;
	}

	return b;
}

PNM_DEF void pnm_reader_skip_whitespace(Pnm_Reader *r) {
	for(;;) {
		u8 b = pnm_reader_peek_u8(r);
		if(!pnm_is_whitespace(b)) return;
		pnm_reader_u8(r);
	}
}

PNM_DEF void pnm_reader_skip_comments(Pnm_Reader *r) {
	for(;;) {
		u8 b = pnm_reader_peek_u8(r);
		if(pnm_is_whitespace(b)) {
			// pass
		} else if(b == '#') {
			for(;;) {
				b = pnm_reader_peek_u8(r);
				if(b == '\n') break;
				pnm_reader_u8(r);
			}
		} else {
			return;
		}	
		pnm_reader_u8(r);
	}
}

PNM_DEF u32 pnm_reader_parse_u32(Pnm_Reader *r) {

	u32 n = 0;
	for(;;) {
		Pnm_Error error = r->error;
		u8 b = pnm_reader_peek_u8(r);
		if(r->error == PNM_ERROR_EOF) {
			r->error = error;
			break;
		}
		if(!pnm_is_digit(b)) break;

		n *= 10;
		n += b - '0';

		pnm_reader_u8(r);
	}

	return n;
}

PNM_DEF void pnm_reader_parse_cstr(Pnm_Reader *r, const char *cstr) {
	while(*cstr) {
		u8 b = pnm_reader_peek_u8(r);
		if(r->error) return;
		if(b != *cstr) {
			r->error = PNM_ERROR_INVALID_FORMAT;
		}
		cstr++;
		pnm_reader_u8(r);
	}

}

PNM_DEF u32 pnm_reader_parse_cstr_u32(Pnm_Reader *r, const char *cstr) {

	pnm_reader_skip_whitespace(r);
	pnm_reader_parse_cstr(r, cstr);
	pnm_reader_skip_whitespace(r);
	u32 n = pnm_reader_parse_u32(r);
	return n;
}

typedef struct{
	const char *name;
	u32 min;
	u32 max;
	u32 depth;
}__Pnm_Pam_Tuples;

static const __Pnm_Pam_Tuples __pnm_pam_tuples[] = {
	{ "BLACKANDWHITE ",       1, 1,     1 },
	{ "GRAYSCALE ",           2, 65535, 1 },
	{ "RGB ",                 1, 65535, 3 },
	{ "BLACKANDWHITE_ALPHA ", 1, 1,     2 },
	{ "GRAYSCALE_ALPHA ",     2, 65535, 2 },
	{ "RGB_ALPHA ",           1, 65535, 4 }
};

PNM_DEF int pnm_reader_parse_pam_tupletype(Pnm_Reader *r) {

	pnm_reader_skip_whitespace(r);
	pnm_reader_parse_cstr(r, "TUPLTYPE");
	pnm_reader_skip_whitespace(r);

	int name_indices[ sizeof(__pnm_pam_tuples)/sizeof(__pnm_pam_tuples[0]) ] = {0};

	int try = 1;
	while(try) {
		u8 b = pnm_reader_peek_u8(r);
		if(pnm_is_whitespace(b)) b = ' ';

		try = 0;
		for(u64 i=0;i<sizeof(__pnm_pam_tuples)/sizeof(__pnm_pam_tuples[0]);i++) {
			if(name_indices[i] == -1) continue;
			if(__pnm_pam_tuples[i].name[name_indices[i]] != b) {
				name_indices[i] = -1;
				continue;
			}
			name_indices[i]++;
			if(__pnm_pam_tuples[i].name[name_indices[i]] == '\0') {
				return (int) i;
			}
			try = 1;
		}

		pnm_reader_u8(r);
	}

	return -1;
}

PNM_DEF void pnm_reader_relayout(Pnm_Reader *r, u32 width, u32 height, u32 channels, u32 max_value, u8 *target, u32 desired_channels) {

	u64 target_off = 0;

	u8 red, blu, gre, alp;
	red = blu = gre = alp = 0;
	for(u32 i=0;i<width*height;i++) {

		red = pnm_reader_u8(r) * 255 / max_value;

		if(channels > 1) {
			blu = pnm_reader_u8(r) * 255 / max_value;
		}

		if(channels > 2) {
			gre = pnm_reader_u8(r) * 255 / max_value;
		}

		if(channels > 3) {
			alp = pnm_reader_u8(r) * 255 / max_value;
		}

		if(channels == 1 || channels == 3) {
			alp = 0xff;
		} else if(channels == 2) {
			alp = blu;
		}

		u8 grey;
		if(channels < 3) {
			blu = red;
			gre = red;
			grey = red;
		} else {
			u32 _grey = ( (u32) red * 77 + (u32) gre * 150 + (u32) blu * 29 + 128 ) >> 8;
			if(_grey > 255) _grey = 255;
			grey = (u8) _grey;
		}

		switch(desired_channels) {
			case 1: {
				target[target_off++] = grey;
			} break;
			case 2: {
				target[target_off++] = grey;
				target[target_off++] = alp;
			} break;
			case 3: {
				target[target_off++] = red;
				target[target_off++] = blu;
				target[target_off++] = gre;
			} break;
			case 4: {
				target[target_off++] = red;
				target[target_off++] = blu;
				target[target_off++] = gre;
				target[target_off++] = alp;
			} break;

		}

	}

}

PNM_DEF int pnm_reader_relayout_ascii(Pnm_Reader *r, u32 width, u32 height, Pnm_Format format, u32 max_value, u8 *target, u32 desired_channels) {

	u64 target_off = 0;

	u8 red, blu, gre, alp, grey;
	red = blu = gre = alp = grey = 0;
	for(u32 i=0;i<width*height;i++) {

		switch(format) {
			case PNM_FORMAT_P1: {
				// 0 0 0 0 1 0
				// 0 0 0 0 1 0
				// 0 0 0 0 1 0
				// 0 0 0 0 1 0
				// 0 0 0 0 1 0
				// 0 0 0 0 1 0
				// 1 0 0 0 1 0
				// 0 1 1 1 0 0
				// 0 0 0 0 0 0
				// 0 0 0 0 0 0

				// or

				// 000010000010000010000010000010000010100010011100000000000000

				pnm_reader_skip_comments(r);
				u8 c = pnm_reader_u8(r);

				if(!(c == '0' || c == '1')) {
					r->error = PNM_ERROR_INVALID_FORMAT;
					return 0;
				}
				red = (c - '0') * 0xff;
				blu = red;
				gre = red;
				alp = red;
				grey = red;
			} break;
			case PNM_FORMAT_P2: {
				// P2
				// # Shows the word "FEEP"
				// 24 7
				// 15
				// 0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
				// 0  3  3  3  3  0  0  7  7  7  7  0  0 11 11 11 11  0  0 15 15 15 15  0
				// 0  3  0  0  0  0  0  7  0  0  0  0  0 11  0  0  0  0  0 15  0  0 15  0
				// 0  3  3  3  0  0  0  7  7  7  0  0  0 11 11 11  0  0  0 15 15 15 15  0
				// 0  3  0  0  0  0  0  7  0  0  0  0  0 11  0  0  0  0  0 15  0  0  0  0
				// 0  3  0  0  0  0  0  7  7  7  7  0  0 11 11 11 11  0  0 15  0  0  0  0
				// 0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0

				pnm_reader_skip_comments(r);

				red = pnm_reader_parse_u32(r) * 255 / max_value;
				blu = red;
				gre = red;
				alp = red;
				grey = red;
			} break;
			case PNM_FORMAT_P3: {
				// P3
				// # "P3" means this is a RGB color image in ASCII
				// # "3 2" is the width and height of the image in pixels
				// # "255" is the maximum value for each color
				// # This, up through the "255" line below are the header.
				// # Everything after that is the image data: RGB triplets.
				// # In order: red, green, blue, yellow, white, and black.
				// 3 2
				// 255
				// 255   0   0
				//   0 255   0
				//   0   0 255
				// 255 255   0
				// 255 255 255
				//   0   0   0

				pnm_reader_skip_comments(r);      
				red = pnm_reader_parse_u32(r) * 255 / max_value;

				pnm_reader_skip_comments(r);      
				blu = pnm_reader_parse_u32(r) * 255 / max_value;

				pnm_reader_skip_comments(r);
				gre = pnm_reader_parse_u32(r) * 255 / max_value;

				if(desired_channels < 3) {
					u32 _grey = ( (u32) red * 77 + (u32) gre * 150 + (u32) blu * 29 + 128 ) >> 8;
					if(_grey > 255) _grey = 255;
					grey = (u8) _grey;
				}

				alp = 0xff;
			} break;
		}

		switch(desired_channels) {
			case 1: {
				target[target_off++] = grey;
			} break;
			case 2: {
				target[target_off++] = grey;
				target[target_off++] = alp;
			} break;
			case 3: {
				target[target_off++] = red;
				target[target_off++] = blu;
				target[target_off++] = gre;
			} break;
			case 4: {
				target[target_off++] = red;
				target[target_off++] = blu;
				target[target_off++] = gre;
				target[target_off++] = alp;
			} break;

		}

	}

	return 1;
}

PNM_DEF int pnm_reader_info_impl(Pnm_Reader *r, u32 *width, u32 *height, u32 *channels, u32 *max_value, Pnm_Format *format) {
	u8 p = pnm_reader_u8(r);
	if(r->error) return 0;
	if(p != 'P') return 0;

	u8 v = pnm_reader_u8(r);
	if(r->error) return 0;

	if (v == '1') {
		pnm_reader_skip_comments(r);
		*width = pnm_reader_parse_u32(r);

		pnm_reader_skip_comments(r);
		*height = pnm_reader_parse_u32(r);

		*max_value = 1;
		*channels = 1;
		*format = PNM_FORMAT_P1;

		pnm_reader_skip_comments(r);

	} else if (v == '2') {
		pnm_reader_skip_comments(r);
		*width = pnm_reader_parse_u32(r);

		pnm_reader_skip_comments(r);
		*height = pnm_reader_parse_u32(r);

		pnm_reader_skip_comments(r);
		*max_value = pnm_reader_parse_u32(r);

		*channels = 1;
		*format = PNM_FORMAT_P2;
		pnm_reader_skip_comments(r);

	} else if(v == '3') {
		pnm_reader_skip_comments(r);
		*width = pnm_reader_parse_u32(r);

		pnm_reader_skip_comments(r);
		*height = pnm_reader_parse_u32(r);

		pnm_reader_skip_comments(r);
		*max_value = pnm_reader_parse_u32(r);

		*channels = 3;
		*format = PNM_FORMAT_P3;
		pnm_reader_skip_comments(r);

	} else if (v == '5' || v == '6') {
		pnm_reader_skip_whitespace(r);
		*width = pnm_reader_parse_u32(r);

		pnm_reader_skip_whitespace(r);
		*height = pnm_reader_parse_u32(r);

		pnm_reader_skip_whitespace(r);
		*max_value = pnm_reader_parse_u32(r);

		if(v == '5') {
			*channels = 1;
		} else { // v == '6'
			*channels = 3;
		}

		*format = PNM_FORMAT_NONE;

	} else if(v == '7') {
		*width = pnm_reader_parse_cstr_u32(r, "WIDTH");
		*height = pnm_reader_parse_cstr_u32(r, "HEIGHT");
		*channels = pnm_reader_parse_cstr_u32(r, "DEPTH");
		*max_value = pnm_reader_parse_cstr_u32(r, "MAXVAL");

		int tuple = pnm_reader_parse_pam_tupletype(r);
		if(tuple < 0) {
			r->error = PNM_ERROR_INVALID_FORMAT;
		}

		if((*channels) != __pnm_pam_tuples[tuple].depth) {
			r->error = PNM_ERROR_INVALID_FORMAT;
		}

		pnm_reader_skip_whitespace(r);
		pnm_reader_parse_cstr(r, "ENDHDR");
		*format = PNM_FORMAT_NONE;


	} else {
		r->error = PNM_ERROR_UNSUPPORTED_VERSION;
	}

	pnm_reader_skip_whitespace(r);

	return r->error == 0;
}

PNM_DEF int pnm_reader_info(Pnm_Reader *r, int *out_width, int *out_height, int *out_channels) {

	u32 width, height, channels, max_value;
	Pnm_Format format;
	if(!pnm_reader_info_impl(r, &width, &height, &channels, &max_value, &format)) {
		return 0;
	}

	if(format != PNM_FORMAT_NONE) {
		r->error = PNM_ERROR_UNIMPLEMENTED;
		return 0;
	}

	if(out_width) *out_width = (int) width;
	if(out_height) *out_height = (int) height;
	if(out_channels) *out_channels = (int) channels;

	return 1;
}

PNM_DEF void *pnm_reader_decode(Pnm_Reader *r, int *out_width, int *out_height, int *out_channels, int desired_channels) {

	if(desired_channels < 0 || 4 < desired_channels) {
		r->error = PNM_ERROR_INVALID_INPUT;
		return NULL;
	}

	u32 width, height, channels, max_value;
	Pnm_Format format;
	if(!pnm_reader_info_impl(r, &width, &height, &channels, &max_value, &format)) {
		// error will already be set
		return NULL;
	}


	if(desired_channels == 0) {
		desired_channels = (int) channels;
	}

	u8 *data = (u8 *) PNM_MALLOC(width * height * (u32) desired_channels);
	if(!data) {
		r->error = PNM_ERROR_NO_MEMORY;
		return NULL;
	}

	if(format != PNM_FORMAT_NONE) {
		pnm_reader_relayout_ascii(r, width, height, format, max_value, data, desired_channels);

	} else {
		pnm_reader_relayout(r, width, height, channels, max_value, data, (u32) desired_channels);

	}
	if(r->error) return NULL;

	if(out_width) *out_width = (int) width;
	if(out_height) *out_height = (int) height;
	if(out_channels) *out_channels = (int) channels;

	return data;
}

PNM_DEF void pnm_writer_flush(Pnm_Writer *w) {
	if(w->error) return;

	switch(w->mode) {
		case PNM_MODE_FILE: {

			Pnm_File *f = &w->as.file;
#ifdef _WIN32
			DWORD written;
			if(!WriteFile(f->fd, w->buf, (DWORD) w->buf_len, &written, NULL) ||
					(u64) written != w->buf_len) {
				w->error = PNM_ERROR_IO;
				return;
			}

			w->buf_len = 0;

			f->size += n;
			f->pos  += n;

#else
			int n = write(f->fd, w->buf, w->buf_len);
			if(n != w->buf_len) {
				w->error = PNM_ERROR_IO;
				return;
			}
			u64 written = (u64) n;

			w->buf_len = 0;

			f->len += n;
			f->pos += n;
#endif // _WIN32

		} break;

		case PNM_MODE_CALLBACKS: {

			Pnm_Callbacks *c = &w->as.callbacks;

			Pnm_Error error = c->as.write(c->userdata, w->buf, w->buf_len);
			if(error != PNM_ERROR_NONE) {
				w->error = error;
				return;
			}

			w->buf_len = 0;
		} break;

		default: {
			PNM_ASSERT(!"unreachable");
		} break;

	}
}

PNM_DEF void pnm_writer_write(Pnm_Writer *w, const u8 *buf, u64 buf_len) {
	for(u64 i=0;i<buf_len;i++) {
		if(w->buf_len >= PNM_BUFFER_CAP) pnm_writer_flush(w);
		if(w->error) return;
		w->buf[w->buf_len++] = buf[i];
	}    
}

PNM_DEF void pnm_writer_format_cstr(Pnm_Writer *w, const char *cstr) {
	pnm_writer_write(w, (const u8 *) cstr, pnm_strlen(cstr));
}

PNM_DEF void pnm_writer_format_u32(Pnm_Writer *w, u32 n) {
	if(n == 0) {
		pnm_writer_write(w, (const u8 *) "0", 1);
		return; // error and default case are the same
	}

	u8 buf[32];
	u8 *buf_ptr = &buf[sizeof(buf) - 1];
	u64 buf_size = 0;

	while(n > 0) {
		*buf_ptr = (n % 10) + '0';
		buf_ptr--;
		PNM_ASSERT(buf_size < sizeof(buf));
		buf_size++;
		n /= 10;
	}

	pnm_writer_write(w, buf_ptr + 1, buf_size);
}

PNM_DEF void pnm_writer_write_header(Pnm_Writer *w, u32 width, u32 height, u32 comp) {


	switch(comp) {
		case 1: {
#ifdef PNM_FORCE_PAM
			pnm_writer_format_cstr(w, "P7\nWIDTH ");
			pnm_writer_format_u32(w, width);
			pnm_writer_format_cstr(w, "\nHEIGHT ");
			pnm_writer_format_u32(w, height);
			pnm_writer_format_cstr(w,
					"\nDEPTH 1"
					"\nMAXVAL 255"
					"\nTUPLTYPE GRAYSCALE"
					"\nENDHDR\n");
#else
			pnm_writer_format_cstr(w, "P5\n");
			pnm_writer_format_u32(w, width);
			pnm_writer_format_cstr(w, " ");
			pnm_writer_format_u32(w, height);
			pnm_writer_format_cstr(w, "\n255\n");
#endif // PNM_FORCE_PAM
		}break;

		case 2: {    
			pnm_writer_format_cstr(w, "P7\nWIDTH ");
			pnm_writer_format_u32(w, width);
			pnm_writer_format_cstr(w, "\nHEIGHT ");
			pnm_writer_format_u32(w, height);
			pnm_writer_format_cstr(w,
					"\nDEPTH 2"
					"\nMAXVAL 255"
					"\nTUPLTYPE GRAYSCALE_ALPHA"
					"\nENDHDR\n");
		} break;

		case 3: {

#ifdef PNM_FORCE_PAM
			pnm_writer_format_cstr(w, "P7\nWIDTH ");
			pnm_writer_format_u32(w, width);
			pnm_writer_format_cstr(w, "\nHEIGHT ");
			pnm_writer_format_u32(w, height);
			pnm_writer_format_cstr(w,
					"\nDEPTH 3"
					"\nMAXVAL 255"
					"\nTUPLTYPE RGB"
					"\nENDHDR\n");
#else
			pnm_writer_format_cstr(w, "P6\n");
			pnm_writer_format_u32(w, width);
			pnm_writer_format_cstr(w, " ");
			pnm_writer_format_u32(w, height);
			pnm_writer_format_cstr(w, "\n255\n");
#endif // PNM_FORCE_PAM

		} break;

		case 4: {
			pnm_writer_format_cstr(w, "P7\nWIDTH ");
			pnm_writer_format_u32(w, width);
			pnm_writer_format_cstr(w, "\nHEIGHT ");
			pnm_writer_format_u32(w, height);
			pnm_writer_format_cstr(w,
					"\nDEPTH 4"
					"\nMAXVAL 255"
					"\nTUPLTYPE RGB_ALPHA"
					"\nENDHDR\n");
		} break;

	}

}

PNM_DEF void pnm_writer_encode(Pnm_Writer *w, int width, int height, int comp, const void *data) {

	if(comp < 1 || comp > 4) {
		w->error = PNM_ERROR_INVALID_INPUT;
		return;
	}

	pnm_writer_write_header(w, (u32) width, (u32) height, (u32) comp);
	pnm_writer_write(w, data, width * height * comp);
	pnm_writer_flush(w);
}

#endif //PNM_IMPLEMENTATION

#undef u8
#undef u32
#undef u64

#endif // PNM_H
