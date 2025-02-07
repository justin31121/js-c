#ifndef MD5_H_
#define MD5_H_

// https://www.ietf.org/rfc/rfc1321.txt
// https://github.com/Zunawe/md5-c

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

#include <string.h>

typedef unsigned char Md5_u8;
typedef unsigned int Md5_u32;
typedef unsigned long long int Md5_u64;

#define u8 Md5_u8
#define u32 Md5_u32
#define u64 Md5_u64

#ifndef MD5_DEF
#  define MD5_DEF static inline
#endif // MD5_DEF

#define MD5_CONTEXT_WINDOW_SIZE (512 / 8)
#define MD5_DIGEST_LEN 16

typedef struct { u8 bs[MD5_DIGEST_LEN]; } Md5;

#define Md5_fmt "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
#define Md5_arg(m) \
	(m).bs[0], (m).bs[1], (m).bs[2], (m).bs[3],\
	(m).bs[4], (m).bs[5], (m).bs[6], (m).bs[7],\
	(m).bs[8], (m).bs[9], (m).bs[10], (m).bs[11],\
	(m).bs[12], (m).bs[13], (m).bs[14], (m).bs[15]

typedef struct {
    u64 len;
    u8 window[MD5_CONTEXT_WINDOW_SIZE];
    Md5 digest;
} Md5_Context;

#define Md5_Context_base() (Md5_Context) { .len = 0, .window = {}, .digest = { .bs = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10 } } }

MD5_DEF void md5_context_process_impl(Md5_Context *self);
MD5_DEF void md5_context_process(Md5_Context *self, u8 *data, u64 data_len);
MD5_DEF Md5 md5_context_finalize(Md5_Context *self);

#ifdef MD5_IMPLEMENTATION

// static u8 MD5_DIGEST_BASE[MD5_DIGEST_LEN] = {
//     0x01, 0x23, 0x45, 0x67,
//     0x89, 0xab, 0xcd, 0xef,
//     0xfe, 0xdc, 0xba, 0x98,
//     0x76, 0x54, 0x32, 0x10
// };

static u32 MD5_K[] = {0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

static u32 MD5_S[] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

#define MD5_F(X, Y, Z) (((X) & (Y)) | (~(X) & (Z)))
#define MD5_G(X, Y, Z) (((X) & (Z)) | ((Y) & ~(Z)))
#define MD5_H(X, Y, Z) ((X) ^ (Y) ^ (Z))
#define MD5_I(X, Y, Z) ((Y) ^ ((X) | ~(Z)))
#define MD5_ROTATE(X, N) (((X) << (N)) | ((X) >> (32 - (N))))

MD5_DEF void md5_context_process_impl(Md5_Context *self) {

    u8 *digest = self->digest.bs;
    
    u32 a = ((u32 *)digest)[0];
    u32 b = ((u32 *)digest)[1];
    u32 c = ((u32 *)digest)[2];
    u32 d = ((u32 *)digest)[3];
    
    for(u32 i=0;i<64;i++) {

	u32 e, j;
	switch(i / 16) {
	case 0:
	    e = MD5_F(b, c, d);
	    j = i;
	    break;
	case 1:
	    e = MD5_G(b, c, d);
	    j = ((i * 5) + 1) % 16;
	    break;
	case 2:
	    e = MD5_H(b, c, d);
	    j = ((i * 3) + 5) % 16;
	    break;
	default:
	    e = MD5_I(b, c, d);
	    j = (i * 7) % 16;
	    break;
	}

	u32 temp = d;
	d = c;
	c = b;
	b = b + MD5_ROTATE(a + e + MD5_K[i] + ((u32 *) self->window)[j], MD5_S[i]);
	a = temp;
    }

    ((u32 *)digest)[0] += a;
    ((u32 *)digest)[1] += b;
    ((u32 *)digest)[2] += c;
    ((u32 *)digest)[3] += d;
}

MD5_DEF void md5_context_process(Md5_Context *self, u8 *data, u64 data_len) {
    u64 relative_pos = self->len % 64;

    u64 off = 0;
    while(off < data_len) {
	u64 len = 64 - relative_pos;
	if(data_len < off + len) len = data_len - off;
	memcpy(self->window + relative_pos, data + off, len);
	self->len += len;
	off += len;
	if(relative_pos + len == 64) {
	    md5_context_process_impl(self);
	    relative_pos = 0;
	}
    }    
}

MD5_DEF Md5 md5_context_finalize(Md5_Context *self) {
    u64 relative_pos = self->len % 64;
    
    Md5 temp = self->digest;

    self->window[relative_pos] = 0x80;
    memset(self->window + relative_pos + 1, 0, MD5_CONTEXT_WINDOW_SIZE - relative_pos - 8 - 1);
    *(u32 *) &self->window[MD5_CONTEXT_WINDOW_SIZE - 8] = self->len * 8;
    *(u32 *) &self->window[MD5_CONTEXT_WINDOW_SIZE- 4] = (self->len * 8) >> 32;

    md5_context_process_impl(self);
    
    Md5 digest = self->digest;
    self->digest = temp;
    
    return digest;
}

#endif // MD5_IMPLEMENTATION

#undef u8
#undef u32
#undef u64

#endif // MD5_H_
