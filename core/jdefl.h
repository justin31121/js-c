#ifndef JDEFL_H
#define JDEFL_H

// https://github.com/fxfactorial/sdefl

// I modified the 'sdefl' deflate implementation, to
// be able to estimate the compressed/uncompressed
// size, like:
// 
//    u64 compressed_size = jdeflate(NULL, 0, in, in _len, 3);
//

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

// ------------------------------------------------------------------------------
// This software is available under 2 licenses -- choose whichever you prefer.
// ------------------------------------------------------------------------------
// ALTERNATIVE A - MIT License
// Copyright (c) 2020 Micha Mettke
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// ------------------------------------------------------------------------------
// ALTERNATIVE B - Public Domain (www.unlicense.org)
// This is free and unencumbered software released into the public domain.
// Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
// software, either in source code form or as a compiled binary, for any purpose,
// commercial or non-commercial, and by any means.
// In jurisdictions that recognize copyright laws, the author or authors of this
// software dedicate any and all copyright interest in the software to the public
// domain. We make this dedication for the benefit of the public at large and to
// the detriment of our heirs and successors. We intend this dedication to be an
// overt act of relinquishment in perpetuity of all present and future rights to
// this software under copyright law.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// ------------------------------------------------------------------------------
  
#include <string.h>

typedef unsigned char Jdefl_u8;
typedef char Jdefl_s8;
typedef short Jdefl_s16;
typedef int Jdefl_s32;
typedef unsigned int Jdefl_u32;
typedef unsigned long long int Jdefl_u64;
#define u8 Jdefl_u8
#define s8 Jdefl_s8
#define s16 Jdefl_s16
#define u32 Jdefl_u32
#define s32 Jdefl_s32
#define u64 Jdefl_u64

#ifndef JDEFL_DEF
#  define JDEFL_DEF static inline
#endif // JDEFL_DEF

#define JDEFL_MAX_OFF       (1 << 15)
#define JDEFL_WIN_SIZ       JDEFL_MAX_OFF
#define JDEFL_WIN_MSK       (JDEFL_WIN_SIZ-1)

#define JDEFL_MIN_MATCH     4
#define JDEFL_MAX_MATCH     258

#define JDEFL_HASH_BITS     19
#define JDEFL_HASH_SIZ      (1 << JDEFL_HASH_BITS)
#define JDEFL_HASH_MSK      (JDEFL_HASH_SIZ-1)
#define JDEFL_NIL           (-1)

#define JDEFL_LVL_MIN       0
#define JDEFL_LVL_DEF       5
#define JDEFL_LVL_MAX       8

JDEFL_DEF u32 jdefl_uload32(const void *p);
JDEFL_DEF u32 jdefl_hash32(const void *p);
JDEFL_DEF s32 jdefl_ilog2(s32 n);
JDEFL_DEF s32 jdefl_npow2(s32 n);

typedef struct {
  s32 bits, cnt;
  s32 tbl[JDEFL_HASH_SIZ];
  s32 prv[JDEFL_WIN_SIZ];
  u64 i, max;
} jdefl;

JDEFL_DEF u8 *jdefl_put(u8 *dst, jdefl *j, s32 code, s32 bitcnt);
JDEFL_DEF u8 *jdefl_match(u8 *dst, jdefl *j, s32 dist, s32 len);
JDEFL_DEF u8 *jdefl_lit(u8 *dst, jdefl *j, s32 c);

JDEFL_DEF u64 jdeflate(jdefl *j,
		       u8 *out,
		       u64 out_len,
		       u8 *in,
		       u64 in_len,
		       s32 lvl);

//////////////////////////////////////////////////////////////////

JDEFL_DEF s32 jinfl_build(u32 *tree, u8 *lens, s32 symcnt);

typedef struct  {
  s32 bits, bitcnt;
  u32 lits[288];
  u32 dsts[32];
  u32 lens[19];
  s32 tlit, tdist, tlen;
} jinfl;

JDEFL_DEF s32 jinfl_get(u8 **src, u8 *end, jinfl *j, s32 n);
JDEFL_DEF s32 jinfl_decode(u8 **in, u8 *end, jinfl *j, u32 *tree, s32 max);

JDEFL_DEF u64 jinflate(u8 *out,
		       u64 out_len,
		       u8 *in,
		       u64 in_len);

#ifdef JDEFL_IMPLEMENTATION

JDEFL_DEF u32 jdefl_uload32(const void *p) {
  /* hopefully will be optimized to an unaligned read */
  u32 n = 0;
  memcpy(&n, p, sizeof(n));
  return n;
}


JDEFL_DEF u32 jdefl_hash32(const void *p) {
  u32 n = jdefl_uload32(p);
  return (n*0x9E377989)>>(32-JDEFL_HASH_BITS);
}


JDEFL_DEF s32 jdefl_ilog2(s32 n) {
#define lt(n) n,n,n,n, n,n,n,n, n,n,n,n ,n,n,n,n
  static const u8 tbl[256] = {-1,0,1,1,2,2,2,2,3,3,3,3,
			      3,3,3,3,lt(4),lt(5),lt(5),lt(6),lt(6),lt(6),lt(6),
			      lt(7),lt(7),lt(7),lt(7),lt(7),lt(7),lt(7),lt(7)
  }; s32 tt, t;
  if ((tt = (n >> 16)))
    return (t = (tt >> 8)) ? 24+tbl[t]: 16+tbl[tt];
  else return (t = (n >> 8)) ? 8+tbl[t]: tbl[n];
#undef lt
}

JDEFL_DEF s32 jdefl_npow2(s32 n) {
  n--;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  return (s32) ++n;  
}

static const u8 jdefl_mirror[256] = {
    #define R2(n) n, n + 128, n + 64, n + 192
    #define R4(n) R2(n), R2(n + 32), R2(n + 16), R2(n + 48)
    #define R6(n) R4(n), R4(n +  8), R4(n +  4), R4(n + 12)
    R6(0), R6(2), R6(1), R6(3),
    #undef R2
    #undef R4
    #undef R6
};

JDEFL_DEF u8 *jdefl_put(u8 *dst, jdefl *j, s32 code, s32 bitcnt) {
  j->bits |= (code << j->cnt);
  j->cnt += bitcnt;
  while (j->cnt >= 8) {
    if(j->i++ < j->max) {
      *dst = (u8) (j->bits & 0xFF);
    }
    dst++; 
    j->bits >>= 8;
    j->cnt -= 8;
  } return dst;
}

JDEFL_DEF u8 *jdefl_match(u8 *dst, jdefl *j, s32 dist, s32 len) {
  static const s16 lxmin[] = {0,11,19,35,67,131};
  static const s16 dxmax[] = {0,6,12,24,48,96,192,384,768,1536,3072,6144,12288,24576};
  static const s16 lmin[] = {11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227};
  static const s16 dmin[] = {1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,
			       385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577};

  /* length encoding */
  s32 lc = len;
  s32 lx = jdefl_ilog2(len - 3) - 2;
  if (!(lx = (lx < 0) ? 0: lx)) lc += 254;
  else if (len >= 258) lx = 0, lc = 285;
  else lc = ((lx-1) << 2) + 265 + ((len - lxmin[lx]) >> lx);

  if (lc <= 279)
    dst = jdefl_put(dst, j, jdefl_mirror[(lc - 256) << 1], 7);
  else dst = jdefl_put(dst, j, jdefl_mirror[0xc0 - 280 + lc], 8);
  if (lx) dst = jdefl_put(dst, j, len - lmin[lc - 265], lx);

  /* distance encoding */
  {
    s32 dc = dist - 1;
    s32 dx = jdefl_ilog2(jdefl_npow2(dist) >> 2);
    if ((dx = (dx < 0) ? 0: dx)) {
      dc = ((dx + 1) << 1) + (dist > dxmax[dx]);
    }
    dst = jdefl_put(dst, j, jdefl_mirror[dc << 3], 5);
    if (dx) dst = jdefl_put(dst, j, dist - dmin[dc], dx);
  }
  return dst;
}

JDEFL_DEF u8 *jdefl_lit(u8 *dst, jdefl *j, s32 c) {
  if (c <= 143)
    return jdefl_put(dst, j, jdefl_mirror[0x30+c], 8);
  else return jdefl_put(dst, j, 1 + 2 * jdefl_mirror[0x90 - 144 + c], 9);
}

JDEFL_DEF u64 jdeflate(jdefl *j,
		       u8 *out,
		       u64 out_len,
		       u8 *in,
		       u64 in_len,
		       s32 lvl) {
  j->i   = 0;
  j->max = out_len;
  
  u64 p = 0;
  u64 max_chain = (lvl < 8) ? (1<<(lvl+1)): (1<<13);
  u8 *q = out;

  j->bits = j->cnt = 0;
  for (p = 0; p < JDEFL_HASH_SIZ; ++p)
    j->tbl[p] = JDEFL_NIL;

  p = 0;
  q = jdefl_put(q, j, 0x01, 1); /* block */
  q = jdefl_put(q, j, 0x01, 2); /* static huffman */
  while (p < in_len) {
    s32 run, best_len = 0, dist = 0;
    s32 max_match = ((in_len-p)>JDEFL_MAX_MATCH) ? JDEFL_MAX_MATCH:(in_len-p);
    if (max_match > JDEFL_MIN_MATCH) {
      s32 limit;
      if(p < JDEFL_WIN_SIZ -1) {
	limit = JDEFL_NIL;
      } else {
	limit = (s32) (p - JDEFL_WIN_SIZ);
      }
      
      s32 chain_len = max_chain;
      s32 i = j->tbl[jdefl_hash32(&in[p])];
      while (i > limit) {
	if (in[i+best_len] == in[p+best_len] &&
	    (jdefl_uload32(&in[i]) == jdefl_uload32(&in[p]))){
	  s32 n = JDEFL_MIN_MATCH;
	  while (n < max_match && in[i+n] == in[p+n]) n++;
	  if (n > best_len) {
	    best_len = n;
	    dist = p - i;
	    if (n == max_match)
	      break;
	  }
	}
	if (!(--chain_len)) break;
	i = j->prv[i&JDEFL_WIN_MSK];
      }
    }
    if (lvl >= 5 && best_len >= JDEFL_MIN_MATCH && best_len < max_match){
      const s32 x = p + 1;
      s32 tar_len = best_len + 1;
      s32 limit = ((x-JDEFL_WIN_SIZ)<JDEFL_NIL)?JDEFL_NIL:(x-JDEFL_WIN_SIZ);
      s32 chain_len = max_chain;
      s32 i = j->tbl[jdefl_hash32(&in[p])];
      while (i > limit) {
	if (in[i+best_len] == in[x+best_len] &&
	    (jdefl_uload32(&in[i]) == jdefl_uload32(&in[x]))){
	  s32 n = JDEFL_MIN_MATCH;
	  while (n < tar_len && in[i+n] == in[x+n]) n++;
	  if (n == tar_len) {
	    best_len = 0;
	    break;
	  }
	}
	if (!(--chain_len)) break;
	i = j->prv[i&JDEFL_WIN_MSK];
      }
    }
    if (best_len >= JDEFL_MIN_MATCH) {
      q = jdefl_match(q, j, dist, best_len);
      run = best_len;
    } else {
      q = jdefl_lit(q, j, in[p]);
      run = 1;
    }
    while (run-- != 0) {
      unsigned h = jdefl_hash32(&in[p]);
      j->prv[p&JDEFL_WIN_MSK] = j->tbl[h];
      j->tbl[h] = p++;
    }
  }
  /* zlib partial flush */
  q = jdefl_put(q, j, 0, 7);
  q = jdefl_put(q, j, 2, 10);
  q = jdefl_put(q, j, 2, 3);
  return (u64) (q - out);
}

//////////////////////////////////////////////////////////

static const u8 jinfl_mirror[256] = {
    #define R2(n) n, n + 128, n + 64, n + 192
    #define R4(n) R2(n), R2(n + 32), R2(n + 16), R2(n + 48)
    #define R6(n) R4(n), R4(n +  8), R4(n +  4), R4(n + 12)
    R6(0), R6(2), R6(1), R6(3),
#undef R2
#undef R4
#undef R6
};

JDEFL_DEF s32 jinfl_build(u32 *tree, u8 *lens, s32 symcnt) {
  s32 n, cnt[16], first[16], codes[16];
  memset(cnt, 0, sizeof(cnt));
  cnt[0] = first[0] = codes[0] = 0;
  for (n = 0; n < symcnt; ++n) cnt[lens[n]]++;
  for (n = 1; n <= 15; n++) {
    codes[n] = (codes[n-1] + cnt[n-1]) << 1;
    first[n] = first[n-1] + cnt[n-1];
  }
  for (n = 0; n < symcnt; n++) {
    s32 slot, code, len = lens[n];
    if (!len) continue;
    code = codes[len]++;
    slot = first[len]++;
    tree[slot] = (u32) ((code << (32-len)) | (n << 4) | len);
  } return first[15];
}

JDEFL_DEF s32 jinfl_get(u8 **src, u8 *end, jinfl *j, s32 n) {
  u8 *in = *src;
  s32 v = j->bits & ((1 << n)-1);
  j->bits >>= n;
  j->bitcnt = j->bitcnt - n;
  j->bitcnt = j->bitcnt < 0 ? 0 : j->bitcnt;
  while (j->bitcnt < 16 && in < end) {
    j->bits |= (*in++) << j->bitcnt;
    j->bitcnt += 8;
  }
  *src = in;
  return v;
}

JDEFL_DEF s32 jinfl_decode(u8 **in, u8 *end, jinfl *j, u32 *tree, s32 max) {
  
  /* bsearch next prefix code */
#define jinfl_rev16(n) ((jinfl_mirror[(n)&0xff] << 8) | jinfl_mirror[((n)>>8)&0xff])
  u32 key, lo = 0, hi = (u32) max;
  u32 search = (u32) (jinfl_rev16(j->bits) << 16) | 0xffff;
  while (lo < hi) {
    u32 guess = (lo + hi) / 2;
    if (search < tree[guess]) hi = guess;
    else lo = guess + 1;
  }
  /* pull out and check key */
  key = tree[lo-1];
  jinfl_get(in, end, j, key & 0x0f);
  
  return (key >> 4) & 0x0fff;
#undef jinfl_rev16
}

// 

JDEFL_DEF u64 jinflate(u8 *out,
		       u64 out_len,
		       u8 *in,
		       u64 in_len) {
  u64 i = 0;
  static const s8 order[] = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
  static const s16 dbase[30+2] = {1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,
				  257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577};
  static const u8 dbits[30+2] = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,
				 10,10,11,11,12,12,13,13,0,0};
  static const s16 lbase[29+2] = {3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,
				  43,51,59,67,83,99,115,131,163,195,227,258,0,0};
  static const u8 lbits[29+2] = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,
				 4,4,4,5,5,5,5,0,0,0};

  u8 *e = in + in_len, *o = out;
  enum jinfl_states {hdr,stored,fixed,dyn,blk};
  enum jinfl_states state = hdr;
  jinfl j;
  s32 last = 0;

  memset(&j, 0, sizeof(j));
  jinfl_get(&in, e, &j, 0); /* buffer input */
  while (in < e || j.bitcnt) {
    switch (state) {
    case hdr: {
      int type = 0; /* block header */
      last = jinfl_get(&in, e, &j, 1);
      type = jinfl_get(&in, e, &j, 2);

      switch (type) {default: return (u64) (out-o);
      case 0x00: state = stored; break;
      case 0x01: state = fixed; break;
      case 0x02: state = dyn; break;}
    } break;
    case stored: {
      s32 len, nlen; /* uncompressed block */
      jinfl_get(&in, e, &j, j.bitcnt & 7);
      len = jinfl_get(&in, e, &j, 16);
      nlen = jinfl_get(&in, e, &j,16);
      (void) nlen;
      in -= 2; j.bitcnt = 0;

      if (len > (e-in) || !len) return (u64) (out-o);
      if(i + len < out_len) {
	memcpy(out, in, (u64) len);
      }
      i += len;
      in += len, out += len;
      state = hdr;
    } break;
    case fixed: {
      /* fixed huffman codes */
      s32 n; u8 lens[288+32];
      for (n = 0; n <= 143; n++) lens[n] = 8;
      for (n = 144; n <= 255; n++) lens[n] = 9;
      for (n = 256; n <= 279; n++) lens[n] = 7;
      for (n = 280; n <= 287; n++) lens[n] = 8;
      for (n = 0; n < 32; n++) lens[288+n] = 5;

      /* build trees */
      j.tlit  = jinfl_build(j.lits, lens, 288);
      j.tdist = jinfl_build(j.dsts, lens + 288, 32);
      state = blk;
    } break;
    case dyn: {
      /* dynamic huffman codes */
      s32 n, i, nlit, ndist, nlen;
      u8 nlens[19] = {0}, lens[288+32];
      nlit = 257 + jinfl_get(&in, e, &j, 5);
      ndist = 1 + jinfl_get(&in, e, &j, 5);
      nlen = 4 + jinfl_get(&in, e, &j, 4);
      for (n = 0; n < nlen; n++)
	nlens[(s32) order[n]] = (u8) jinfl_get(&in, e, &j, 3);
      j.tlen = jinfl_build(j.lens, nlens, 19);

      /* decode code lengths */
      for (n = 0; n < nlit + ndist;) {
	s32 sym = jinfl_decode(&in, e, &j, j.lens, j.tlen);
	switch (sym) {default: lens[n++] = (u8) sym; break;
	case 16: for (i=3+jinfl_get(&in, e, &j, 2);i;i--,n++) lens[n]=lens[n-1]; break;
	case 17: for (i=3+jinfl_get(&in, e, &j, 3);i;i--,n++) lens[n]=0; break;
	case 18: for (i=11+jinfl_get(&in, e, &j, 7);i;i--,n++) lens[n]=0; break;}
      }
      /* build lit/dist trees */
      j.tlit  = jinfl_build(j.lits, lens, nlit);
      j.tdist = jinfl_build(j.dsts, lens+nlit, ndist);
      state = blk;
    } break;
    case blk: {
      /* decompress block */
      s32 sym = jinfl_decode(&in, e, &j, j.lits, j.tlit);
      if (sym > 256) {sym -= 257; /* match symbol */
	{s32 len = jinfl_get(&in, e, &j, lbits[sym]) + lbase[sym];
	  s32 dsym = jinfl_decode(&in, e, &j, j.dsts, j.tdist);
	  u64 offs = (u64) jinfl_get(&in, e, &j, dbits[dsym]) + dbase[dsym];
	  if (offs > (u64) (out-o)) return (u64) (out-o);
	  while (len--) {
	    if(i < out_len && offs < out_len) *out = *(out-offs);
	    out++;
	    i++;
	  }}
      } else if (sym == 256) {
	if (last) return (u64) (out-o);
	state = hdr;
      } else {
	if(i < out_len) {
	  *out = (u8) sym;
	  i++;
	}
	out++;
	
      }
    } break;}
  }

  return (u64) (out-o);
}

#endif // JDEFL_IMPLEMENTATION

#undef u8
#undef s8
#undef s16
#undef u32
#undef s32
#undef u64

#endif // JDEFL_H
