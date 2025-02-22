#ifndef CANVAS_H
#define CANVAS_H

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

typedef unsigned long long int Canvas_u64;
typedef unsigned int Canvas_u32;
typedef int Canvas_s32;
typedef float Canvas_f32;

#define u64 Canvas_u64
#define u32 Canvas_u32
#define f32 Canvas_f32
#define s32 Canvas_s32

#ifndef CANVAS_DEF
#  define CANVAS_DEF static inline
#endif // CANVAS_DEF

CANVAS_DEF u32 canvas_lerpf(u32 src, u32 dst, f32 t);

typedef struct{
  u32 *pixels;
  u64 width, height, stride;
}Canvas;

#define canvas_from(p, w, h, s) (Canvas) {(p), (w), (h), (s)}

CANVAS_DEF Canvas canvas_subcanvas(Canvas src, u64 x_off, u64 y_off, u64 width, u64 height);

CANVAS_DEF void canvas_fill(Canvas src, u32 color);
CANVAS_DEF void canvas_circle(Canvas src, u64 cx, u64 cy, u64 r, u32 color);
CANVAS_DEF void canvas_interpolate(Canvas src, Canvas dst);
CANVAS_DEF void canvas_interpolate_bilinear(Canvas src, Canvas dst);

#ifdef CANVAS_IMPLEMENTATION

//0x AA BB GG RR
CANVAS_DEF u32 canvas_lerpf(u32 src, u32 dst, f32 t) {
  if(src == dst) return src;

  u32 result = 0xff000000;

  u32 down   = 16;
  u32 it     = 0x00ff0000;

  for(s32 i=0;i<3;i++) {

    f32 s = (f32) ( (src & it) >> down );
    f32 d = (f32) ( (dst & it) >> down );

    result |= ((u32) (s + (d - s) * t)) << down;
    
    it  >>= 8;
    down -= 8;
  }
  
  return result;
}


CANVAS_DEF Canvas canvas_subcanvas(Canvas src, u64 x_off, u64 y_off, u64 width, u64 height) {
  return canvas_from(&src.pixels[y_off * src.stride + x_off], width, height, src.stride);
}

CANVAS_DEF void canvas_fill(Canvas src, u32 color) {
  for(u32 y=0;y<src.height;y++) {
    for(u32 x=0;x<src.width;x++) {
      src.pixels[y * src.stride + x] = color;
   }
  }				  
}

CANVAS_DEF void canvas_circle(Canvas src, u64 cx, u64 cy, u64 r, u32 color) {
	u64 r_squared = r*r;

	for(u64 dx=0;dx<r;dx++)  {
		for(u64 dy=0;dy<r;dy++) {
			if(dx*dx + dy*dy > r_squared) {
				continue;
			}

			if(cx + dx < src.width) {
				u64 x = cx + dx;
				if(cy + dy < src.height) {
					u64 y = cy + dy;
					src.pixels[y*src.width + x] = color;
				}
				if(dy < cy) {
					u64 y = cy - dy;
					src.pixels[y*src.width + x] = color;
				}
			}
			if(dx < cx) {
				u64 x = cx - dx;
				if(cy + dy < src.height) {
					u64 y = cy + dy;
					src.pixels[y*src.width + x] = color;
				}
				if(dy < cy) {
					u64 y = cy - dy;
					src.pixels[y*src.width + x] = color;
				}
			}

		}
	}
}

CANVAS_DEF void canvas_interpolate(Canvas src, Canvas dst) {
  for(u64 y=0;y<dst.height;y++) {
    for(u64 x=0;x<dst.width;x++) {
      u64 src_x = x * src.width / dst.width;
      u64 src_y = y * src.height / dst.height;
      
      dst.pixels[y * dst.stride + x] = src.pixels[src_y * src.stride + src_x];
    }
  }
}

CANVAS_DEF void canvas_interpolate_bilinear(Canvas src, Canvas dst) {

  for(u64 y=0;y<dst.height;y++) {
    for(u64 x=0;x<dst.width;x++) {
      
      u64 nx = x * src.width;
      u64 ny = y * src.height;

      u64 px = nx % dst.width;
      u64 py = ny % dst.height;

      u64 x1 = nx / dst.width,  x2 = nx / dst.width;
      if(px < dst.width / 2) {
	
	px += dst.width / 2;
	if(x1 > 0) x1 -= 1;
      } else {
	
        px -= dst.width / 2;
	if(x2 < src.width - 1) x2 += 1;
      }

      u64 y1 = ny / dst.height, y2 = ny /dst.height;
      if(py < dst.height / 2) {

	py += dst.height / 2;
	if(y1 > 0) y1 -= 1;
      } else {

	py -= dst.height / 2;
	if(y2 < src.height - 1) y2 += 1;
      }

      f32 tx = (f32) px / (f32) dst.width;
      f32 ty = (f32) py / (f32) dst.height;

      dst.pixels[y * dst.stride + x] =
	canvas_lerpf(canvas_lerpf(src.pixels[y1 * src.stride + x1], src.pixels[y1 * src.stride + x2], tx),
		     canvas_lerpf(src.pixels[y2 * src.stride + x1], src.pixels[y2 * src.stride + x2], tx), ty);
    }
  }
}

#undef u64
#undef u32
#undef f32
#undef s32

#endif // CANVAS_IMPLEMENTATION

#endif // CANVAS_H
