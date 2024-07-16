#ifndef B64_H
#define B64_H

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

#ifndef B64_DEF
#  define B64_DEF static inline
#endif // B64_DEF

typedef unsigned char B64_u8;
typedef int B64_s32;
typedef unsigned int B64_u32;
typedef unsigned long long int B64_u64;

#define u8 B64_u8
#define s32 B64_s32
#define u32 B64_u32
#define u64 B64_u64

B64_DEF u64 base64_encode(u8 *out, u64 out_len, u8 *in, u64 in_len);
B64_DEF u64 base64_decode(u8 *out, u64 out_len, u8 *in, u64 in_len);

#ifdef B64_IMPLEMENTATION

u64 base64_encode(u8 *out, u64 out_len, u8 *in, u64 in_len) {
  static u8 b64[256] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  
  u64 required_len = ((in_len + 2) / 3 * 4);
  if(out_len < required_len) return required_len;

  s32 padding = in_len % 3;
  u64 last = in_len - padding;

  u64 j = 0;

  for(u64 i=0;i<last;i+=3) {
    u32 value =
      (in[i + 0] << 16) |
      (in[i + 1] <<  8) |
      (in[i + 2] <<  0);

    out[j++] = b64[value >> 18];
    out[j++] = b64[(value >> 12) & 0x3f];
    out[j++] = b64[(value >>  6) & 0x3f];
    out[j++] = b64[value & 0x3f];
  }
  if(padding) {
    padding -= 1;

    s32 value;
    if(padding) {
      value = (in[last] << 8) | (in[last + 1]);
      out[j++] = b64[(value >> 10) & 0x3f];
      out[j++] = b64[(value >> 4) & 0x3f];
      out[j++] = b64[(value << 2) & 0x3f];
      out[j++] = '=';
      
    } else {
      value = in[last];
      out[j++] = b64[value >> 2];
      out[j++] = b64[(value << 4) & 0x3f];
      out[j++] = '=';
      out[j++] = '=';
      
    }

  }

  return j;

}

u64 base64_decode(u8 *out, u64 out_len, u8 *in, u64 in_len) {

  static u32 b64[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0, 62, 63, 62, 62, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61,  0,  0,
    0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,
    7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
    19, 20, 21, 22, 23, 24, 25,  0,  0,  0,  0, 63,
    0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
    37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51 };

  int has_padding = in_len > 0 && (in_len % 4 || in[in_len - 1] == '=');
  u64 len = ((in_len + 3) / 4 - has_padding) * 4;
  u64 needed_len = (len * 3 / 4 + has_padding + 1);
  if(out_len < needed_len) return needed_len;

  u64 j = 0;

  for(u64 i=0;i<len;i+=4) {
    u32 value =
      (b64[in[i + 0]] << 18) |
      (b64[in[i + 1]] << 12) |
      (b64[in[i + 2]] <<  6) |
      (b64[in[i + 3]]);
    
    out[j++] = (u8) (value >> 16);
    out[j++] = (u8) ((value >> 8) & 0xff);
    out[j++] = (u8) (value & 0xff);
  }
  if(has_padding) {
    u32 value =
      (b64[in[len + 0]] << 18) |
      (b64[in[len + 1]] << 12);

    out[j++] = (u8) (value >> 16);

    if(in_len > len + 2 && in[len + 2] != '=') {
      value |= b64[in[len + 2]] << 6;
      out[j++] = (u8) ((value >> 8) & 0xff);
    }
  }
  
  return j;
}

#endif // B64_IMPLEMENTATION

#undef u8
#undef s32
#undef u32
#undef u64

#endif // B64_H
