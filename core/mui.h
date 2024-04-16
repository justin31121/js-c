#ifndef MUI_H
#define MUI_H

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

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef MUI_DEF
#  define MUI_DEF static inline
#endif // MUI_DEF

typedef unsigned char Mui_u8;
typedef int Mui_s32;
typedef unsigned int Mui_u32;
typedef float Mui_f32;
typedef unsigned long long Mui_u64;
#define u8 Mui_u8
#define u32 Mui_u32
#define s32 Mui_s32
#define f32 Mui_f32
#define u64 Mui_u64

MUI_DEF f32 mui_clip(f32 t, f32 min, f32 max);

typedef struct {
  f32 x, y;
} Mui_Vec2f;

#define mui_vec2f(_x, _y) (Mui_Vec2f) { .x = (_x), .y = (_y) }

typedef struct {
  f32 x, y, z, w;
} Mui_Vec4f;

#define mui_vec4f(_x, _y, _z, _w) (Mui_Vec4f) { .x = (_x), .y = (_y), .z = (_z), .w = (_w) }

MUI_DEF void mui_color(u32 hex, Mui_Vec4f *color);
MUI_DEF void mui_hsv_to_rgb(Mui_Vec4f *hsv, Mui_Vec4f *rgb);
MUI_DEF void mui_rgb_to_hsv(Mui_Vec4f *rgb, Mui_Vec4f *hsv);
MUI_DEF int mui_point_in_rect(Mui_Vec2f point, Mui_Vec2f pos, Mui_Vec2f size);

typedef enum {
  MUI_TYPE_COLOR,
  MUI_TYPE_TEXTURE,
} Mui_Type;

typedef struct {
  f32 x0, y0, x1, y1;
  f32 dx, dy;
  f32 xadvance;
} Mui_Glyph;

typedef struct {
  void *userdata;
  
  f32 size, margin;
  Mui_Glyph *glyphs;
  u64 glyphs_len;
} Mui_Font;

#define mui_font(u, s, m, gs, gl) (Mui_Font) {	\
    .userdata = (u),				\
      .size = (s),				\
      .margin = (m),				\
      .glyphs = (gs),				\
      .glyphs_len = (gl)			\
      }

typedef struct Mui Mui;

typedef void (*Mui_Render)(void *userdata, Mui *m);

struct Mui {
  Mui_Vec2f p1,  p2,  p3;
  Mui_Vec4f c1,  c2,  c3;
  Mui_Vec2f uv1, uv2, uv3;
  Mui_Type type;
  void *userdata;

  Mui_Render render;
  void *render_userdata;  
  Mui_Font font;

  Mui_Vec2f partition_pos;
  Mui_Vec2f partition_size;
  u64 partition_i;
  u64 partition_n;
};

MUI_DEF void mui_partition_begin(Mui *m,
				 Mui_Vec2f pos,
				 Mui_Vec2f size,
				 u64 n);
MUI_DEF void mui_partition(Mui *m,
			   Mui_Vec2f *pos,
			   Mui_Vec2f *size);
MUI_DEF void mui_partition_end(Mui *m);


MUI_DEF void mui_rect(Mui *m,
		      Mui_Vec2f pos,
		      Mui_Vec2f size,
		      Mui_Vec4f color);

MUI_DEF void mui_circle_impl(Mui *m,
			     Mui_Vec2f pos,
			     f32 radius,
			     Mui_Vec4f color,
			     s32 off,
			     s32 len);

MUI_DEF void mui_rect_rounded(Mui *m,
			      Mui_Vec2f pos,
			      Mui_Vec2f size,
			      Mui_Vec4f color);

MUI_DEF void mui_texture(Mui *m,
			 void *userdata,
			 Mui_Vec2f pos,
			 Mui_Vec2f size,
			 Mui_Vec2f uvp,
			 Mui_Vec2f uvs,
			 Mui_Vec4f color);

typedef enum {  
  MUI_ALIGNMENT_LEFT,
  MUI_ALIGNMENT_CENTER,
  MUI_ALIGNMENT_RIGHT,  
} Mui_Alignment;

#define mui_textc(m, p, t, c) mui_text((m), (p), (t), strlen(t), (c))
MUI_DEF void mui_text(Mui *m,
		      Mui_Vec2f pos,
		      char *text,
		      u64 text_len,
		      Mui_Vec4f color);
MUI_DEF void mui_text_measure(Mui *m,
			      char *text,
			      u64 text_len,
			      Mui_Vec2f *size);
MUI_DEF void mui_text_wrapped(Mui *m,
			      Mui_Vec2f pos,
			      Mui_Vec2f size,
			      Mui_Alignment align,
			      char *text,
			      u64 text_len,
			      Mui_Vec4f color);

#ifdef MUI_IMPLEMENTATION

MUI_DEF f32 mui_clip(f32 t, f32 min, f32 max) {
  if(t < min) return min;
  if(t > max) return max;
  return t;
}

MUI_DEF void mui_color(u32 hex, Mui_Vec4f *color) {
  color->x = (f32) ((hex & 0xff000000) >> 24) / (f32) 0xff;
  color->y = (f32) ((hex & 0x00ff0000) >> 16) / (f32) 0xff;  
  color->z = (f32) ((hex & 0x0000ff00) >>  8) / (f32) 0xff;
  color->w = (f32) ((hex & 0x000000ff) >>  0) / (f32) 0xff;  
}

MUI_DEF void mui_hsv_to_rgb(Mui_Vec4f *hsv, Mui_Vec4f *rgb) {
  
  f32 h = mui_clip(hsv->x, 0.0f, 1.0f);
  if(h == 1.0f) h = 0.0f;
  f32 s = mui_clip(hsv->y, 0.0f, 1.0f);
  f32 v = mui_clip(hsv->z, 0.0f, 1.0f);

  f32 hh = h * 6.f;
  s32 kind = (s32) hh;
  f32 kind_inverse = hh - (f32) kind;

  f32 p = v * (1.0f - s);
  f32 q = v * (1.0f - (s * kind_inverse));
  f32 t = v * (1.0f - (s * (1.0f - kind_inverse)));

  f32 r, g, b;
  switch(kind) {
  case 0:
    r = v;
    g = t;
    b = p;
    break;
  case 1:
    r = q;
    g = v;
    b = p;
    break;
  case 2:
    r = p;
    g = v;
    b = t;
    break;
  case 3:
    r = p;
    g = q;
    b = v;
    break;
  case 4:
    r = t;
    g = p;
    b = v;
    break;
  case 5:
    // fallthrough
  default: // case 6:
    r = v;
    g = p;
    b = q;
    break;
  }
    
  *rgb = mui_vec4f(r, g, b, hsv->w);
}

MUI_DEF void mui_rgb_to_hsv(Mui_Vec4f *rgb, Mui_Vec4f *hsv) {

  f32 r = mui_clip(rgb->x, 0.0f, 1.0f);
  f32 g = mui_clip(rgb->y, 0.0f, 1.0f);
  f32 b = mui_clip(rgb->z, 0.0f, 1.0f);

  f32 min = r;
  if(g < min) min = g;
  if(b < min) min = b;

  f32 max = r;
  if(max < g) max = g;
  if(max < b) max = b;


  f32 h, s, v;
  
  v = max;
  f32 delta = max - min;
  if(delta < 0.0001) {
    s = 0.0f;
    h = 0.0f;
    
    *hsv = mui_vec4f(h, s, v, rgb->w);
    return;
  }

  if(max > 0.0) {
    s = (delta / max);
  } else {
    s = 0.0f;
    h = 0.0f;
    
    *hsv = mui_vec4f(h, s, v, rgb->w);
    return;
  }

  if(r >= max) {
    h = (g - b) / delta;
  } else if(g >= max) {
    h = 2.0f + (b - r) / delta;
  } else { // b >= max
    h = 4.0f + (r - g) / delta;
  }

  h /= 6.0;
  if(h < 0.0f) h = 1.0f;
  
  *hsv = mui_vec4f(h, s, v, rgb->w);
}

MUI_DEF int mui_point_in_rect(Mui_Vec2f point, Mui_Vec2f pos, Mui_Vec2f size) {
  return
    pos.x <= point.x &&
    point.x - pos.x <= size.x &&    
    pos.y <= point.y &&
    point.y - pos.y <= size.y;
}

MUI_DEF void mui_partition_begin(Mui *m,
				 Mui_Vec2f pos,
				 Mui_Vec2f size,
				 u64 n) {
  m->partition_pos  = pos;
  m->partition_size = size;
  m->partition_n    = n;
  m->partition_i    = 0;
  
}

MUI_DEF void mui_partition(Mui *m,
			   Mui_Vec2f *pos,
			   Mui_Vec2f *size) {

  float x  = 0;
  float h  = m->partition_size.y / m->partition_n;
  float dy = (m->partition_size.y - m->partition_n * h) / (m->partition_n + 1);

  *pos  = mui_vec2f(m->partition_pos.x + x,
		    m->partition_pos.y + m->partition_size.y - h
		    - (dy + m->partition_i * (dy + h))		    
		    );
  *size = mui_vec2f(m->partition_size.x,
		   h);

  m->partition_i++;
}

MUI_DEF void mui_partition_end(Mui *m) {
  (void) m;
}

MUI_DEF void mui_rect(Mui *m,
		     Mui_Vec2f pos,
		     Mui_Vec2f size,
		     Mui_Vec4f color) {
  m->type = MUI_TYPE_COLOR;
  
  m->p1 = pos;                              m->c1 = color; m->uv1 = mui_vec2f(0, 0);
  m->p2 = mui_vec2f(pos.x + size.x, pos.y); m->c2 = color; m->uv2 = mui_vec2f(0, 0);
  m->p3 = mui_vec2f(pos.x, pos.y + size.y); m->c3 = color; m->uv3 = mui_vec2f(0, 0);
  m->render(m->render_userdata, m);
    
  m->p2 = mui_vec2f(pos.x + size.x, pos.y);          m->c2 = color; m->uv2 = mui_vec2f(0, 0);
  m->p3 = mui_vec2f(pos.x, pos.y + size.y);          m->c3 = color; m->uv3 = mui_vec2f(0, 0);
  m->p1 = mui_vec2f(pos.x + size.x, pos.y + size.y); m->c1 = color; m->uv1 = mui_vec2f(0, 0);
  m->render(m->render_userdata, m);
  
}

#define MUI_CIRCLE_N 32

MUI_DEF void mui_circle_impl(Mui *m,
			     Mui_Vec2f pos,
			     f32 radius,
			     Mui_Vec4f color,
			     s32 off,
			     s32 len) {
  m->type = MUI_TYPE_COLOR;

  for(s32 i=off;i<MUI_CIRCLE_N && i<len;i++) {
    
    f32 alpha = (i + 1.0f) * 2.0f * (f32) M_PI / MUI_CIRCLE_N;
    f32 dx = radius * cosf(alpha);
    f32 dy = radius * sinf(alpha);

    f32 alpha_last = i * 2.0f * (f32) M_PI / MUI_CIRCLE_N;
    f32 dx_last = radius * cosf(alpha_last);
    f32 dy_last = radius * sinf(alpha_last);
  
    m->p1 = pos;                                         m->c1 = color; m->uv1 = mui_vec2f(0, 0);
    m->p2 = mui_vec2f(pos.x + dx, pos.y + dy);           m->c2 = color; m->uv2 = mui_vec2f(0, 0);
    m->p3 = mui_vec2f(pos.x + dx_last, pos.y + dy_last); m->c3 = color; m->uv3 = mui_vec2f(0, 0);
    m->render(m->render_userdata, m);    
  }
  
}

#define MUI_RADIUS_ROUNDED 20.f

MUI_DEF void mui_rect_rounded(Mui *m,
			     Mui_Vec2f pos,
			     Mui_Vec2f size,
			     Mui_Vec4f color) {

  //float radius = MUI_RADIUS_ROUNDED;
  float side = size.x;
  if(size.y < side) side = size.y;
  float radius = side / 8;

  pos.x += radius;
  pos.y += radius;
  size.x -= 2 * radius;
  size.y -= 2 * radius;

  mui_circle_impl(m,
		  mui_vec2f(pos.x + size.x, pos.y + size.y),
		  radius,
		  color,
		  0,
		  MUI_CIRCLE_N / 4);
  mui_circle_impl(m,
		  mui_vec2f(pos.x, pos.y + size.y),
		  radius,
		  color,
		  MUI_CIRCLE_N / 4,
		  MUI_CIRCLE_N * 2 / 4);
  mui_circle_impl(m,
		  pos,
		  radius,
		  color,
		  MUI_CIRCLE_N * 2 / 4,
		  MUI_CIRCLE_N * 3 / 4);
  mui_circle_impl(m,
		  mui_vec2f(pos.x + size.x, pos.y),
		  radius,
		  color,
		  MUI_CIRCLE_N * 3 / 4,
		  MUI_CIRCLE_N * 4 / 4);

  mui_rect(m,
	   mui_vec2f(pos.x, pos.y - radius),
	   mui_vec2f(size.x, size.y + 2 * radius),
	   color);

  mui_rect(m,
	   mui_vec2f(pos.x - radius, pos.y),
	   mui_vec2f(size.x + 2 * radius, size.y),
	   color);
}

MUI_DEF void mui_texture(Mui *m,
			void *userdata,
			Mui_Vec2f pos,
			Mui_Vec2f size,
			Mui_Vec2f uvp,
			Mui_Vec2f uvs,
			Mui_Vec4f color) {
  m->type = MUI_TYPE_TEXTURE;
  m->userdata = userdata;

  m->p1 = pos;                              m->c1 = color; m->uv1 = uvp;
  m->p2 = mui_vec2f(pos.x + size.x, pos.y); m->c2 = color; m->uv2 = mui_vec2f(uvp.x + uvs.x, uvp.y);
  m->p3 = mui_vec2f(pos.x, pos.y + size.y); m->c3 = color; m->uv3 = mui_vec2f(uvp.x, uvp.y + uvs.y);
  m->render(m->render_userdata, m);
  
  m->p2 = mui_vec2f(pos.x + size.x, pos.y); m->c2 = color; m->uv2 = mui_vec2f(uvp.x + uvs.x, uvp.y);
  m->p3 = mui_vec2f(pos.x, pos.y + size.y); m->c3 = color; m->uv3 = mui_vec2f(uvp.x, uvp.y + uvs.y);
  m->p1 = mui_vec2f(pos.x + size.x, pos.y + size.y); m->c1 = color; m->uv1 = mui_vec2f(uvp.x + uvs.x, uvp.y + uvs.y);
  m->render(m->render_userdata, m);

}

MUI_DEF void mui_text(Mui *m,
		     Mui_Vec2f pos,
		     char *text,
		     u64 text_len,
		     Mui_Vec4f color) {
  
  m->type = MUI_TYPE_TEXTURE;
  m->userdata = m->font.userdata;

  Mui_Vec2f p = mui_vec2f(pos.x, pos.y);
  for(u64 i=0;i<text_len;i++) {
    u8 c = text[i];
    if(c > 127) c = '?';
  
    Mui_Glyph g = m->font.glyphs[c - 32];
    f32 w = g.x1 - g.x0;
    f32 h = g.y1 - g.y0;
    
    if(w <= 0 ||h <= 0) {
      continue;
    }
    
    m->p1 = mui_vec2f(p.x + g.dx, p.y + g.dy);     m->c1 = color; m->uv1 = mui_vec2f(g.x0, g.y0);
    m->p2 = mui_vec2f(p.x + g.dx + w, p.y + g.dy); m->c2 = color; m->uv2 = mui_vec2f(g.x0 + w, g.y0);
    m->p3 = mui_vec2f(p.x + g.dx, p.y + g.dy + h); m->c3 = color; m->uv3 = mui_vec2f(g.x0, g.y0 + h);
    m->render(m->render_userdata, m);

    m->p2 = mui_vec2f(p.x + g.dx + w, p.y + g.dy);     m->c2 = color; m->uv2 = mui_vec2f(g.x0 + w, g.y0);
    m->p3 = mui_vec2f(p.x + g.dx, p.y + g.dy + h);     m->c3 = color; m->uv3 = mui_vec2f(g.x0, g.y0 + h);
    m->p1 = mui_vec2f(p.x + g.dx + w, p.y + g.dy + h); m->c1 = color; m->uv1 = mui_vec2f(g.x0 + w, g.y0 + h);
    m->render(m->render_userdata, m);      

    p.x += g.xadvance;
  }

}

MUI_DEF void mui_text_measure(Mui *m,
			      char *text,
			      u64 text_len,
			      Mui_Vec2f *size) {
  
  size->x = 0;
  size->y = 0;

  for(u64 i=0;i<text_len;i++) {
      u8 c = text[i];
      if(c > 127) c = '?';
  
      Mui_Glyph g = m->font.glyphs[c - 32];
      f32 w = g.x1 - g.x0;
      f32 h = g.y1 - g.y0;

      if(w <= 0 || h <= 0) {
	continue;
      }
      
      size->x += g.xadvance;
      if(h > size->y) size->y = h;
  }
  
}

MUI_DEF void mui_text_wrapped(Mui *m,
			      Mui_Vec2f pos,
			      Mui_Vec2f size,
			      Mui_Alignment align,
			      char *text,
			      u64 text_len,
			      Mui_Vec4f color) {  
  pos.y += size.y;
  pos.y -= m->font.size;
  
  Mui_Vec2f text_size;
  text_size.x = 0.f;

  u64 off = 0;
  u64 len = 0;
  while(off < text_len) {
    
    len = 0;
    text_size.x = 0.f;
    int broke = 0;
    
    while(text_size.x < size.x && off + len < text_len) {
      len++;

      mui_text_measure(m, text + off, len, &text_size);
      if(text[off + len] == '\n') {
	broke = 1;
	break;
      }      
    }
    
    if(len > 1 && len > broke) {

      switch(align) {
      case MUI_ALIGNMENT_LEFT:
	mui_text(m, pos, text + off + broke, len - broke, color);
	break;
      case MUI_ALIGNMENT_CENTER:
	mui_text(m,
		 mui_vec2f(pos.x + size.x / 2 - text_size.x / 2, pos.y),
		 text + off + broke, len - broke,
		 color);

	break;
      case MUI_ALIGNMENT_RIGHT:
	mui_text(m, mui_vec2f(size.x - text_size.x, pos.y), text + off + broke, len - broke, color);
	break;
      }
      
    }    
    
    off += len;
    pos.y -= m->font.margin;
  }
  
}

#endif // MUI_IMPLEMENTATION

#undef u8
#undef u32
#undef s32
#undef f32
#undef u64

#endif // MUI_H
