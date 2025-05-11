#ifndef RENDERER_H
#define RENDERER_H

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

#ifndef RENDERER_DEF
#  define RENDERER_DEF static inline
#endif // RENDERER_DEF

typedef float Renderer_f32;
typedef int Renderer_s32;
typedef unsigned long long Renderer_u64;
#define f32 Renderer_f32
#define s32 Renderer_s32
#define u64 Renderer_u64

RENDERER_DEF int renderer_shader_compile(GLuint *shader, GLenum type, char *source);
RENDERER_DEF int renderer_program_link(GLuint *program, GLuint vertex_shader, GLuint fragment_shader);

typedef struct {
  f32 x, y;
} Renderer_Vec2f;

#define renderer_vec2f(_x, _y) (Renderer_Vec2f) { .x = (_x), .y = (_y) }

typedef struct {
  f32 x, y, z, w;
} Renderer_Vec4f;

#define renderer_vec4f(_x, _y, _z, _w) (Renderer_Vec4f) { .x = (_x), .y = (_y), .z = (_z), .w = (_w) }

typedef struct {
  Renderer_Vec2f pos;
  Renderer_Vec4f color;
  Renderer_Vec2f uv;
} Renderer_Vertex;

#define renderer_vertex(p, c, u) (Renderer_Vertex) { .pos = (p), .color = (c), .uv = (u) }

#define RENDERER_VERTICIES_CAP 1024

typedef struct {
  Renderer_Vertex verticies[RENDERER_VERTICIES_CAP];
  size_t verticies_len;

  GLuint vao, vbo;
  GLuint vertex_shader, fragment_shader, program;

  GLuint texture;
  s32 texture_active;
  s32 texture_count;

  Renderer_Vec2f viewport;
  Renderer_Vec4f background;
} Renderer;

RENDERER_DEF int renderer_open(Renderer *r);
RENDERER_DEF void renderer_begin(Renderer *r, f32 width, f32 height);
RENDERER_DEF void renderer_end(Renderer *r);
RENDERER_DEF void renderer_triangle(Renderer *r,
				    Renderer_Vec2f p1, Renderer_Vec4f c1, Renderer_Vec2f uv1,
				    Renderer_Vec2f p2, Renderer_Vec4f c2, Renderer_Vec2f uv2,
				    Renderer_Vec2f p3, Renderer_Vec4f c3, Renderer_Vec2f uv3);
RENDERER_DEF void renderer_close(Renderer *r);

typedef struct {
  s32 index;
  f32 width, height;
  int is_grey;
} Renderer_Texture;


RENDERER_DEF int renderer_texture_create(Renderer_Texture *t,
					 Renderer *r,
					 void *data,
					 f32 width,
					 f32 height,
					 int is_grey);
RENDERER_DEF int renderer_texture_prepare(Renderer_Texture *t,
					  Renderer *r,
					  f32 width,
					  f32 height,
					  int is_grey);
RENDERER_DEF int renderer_texture_write(Renderer_Texture *t,
					void *data,
					Renderer_Vec2f off,
					Renderer_Vec2f size);
RENDERER_DEF void renderer_texture(Renderer *r,
				   Renderer_Texture *t, 
				   Renderer_Vec2f pos,
				   Renderer_Vec2f size,
				   Renderer_Vec2f uvp,
				   Renderer_Vec2f uvs,
				   Renderer_Vec4f tint);

typedef struct {
  f32 x0, y0, x1, y1;
  f32 dx, dy;
  f32 xadvance;
} Renderer_Glyph;

typedef struct {
  f32 size, margin;
  
  Renderer_Glyph *glyphs;
  u64 glyphs_len;
  
  Renderer_Texture texture;
} Renderer_Font;

RENDERER_DEF void renderer_text(Renderer *r,
			        Renderer_Font *f,				
				char *text,
				u64 text_len,
				Renderer_Vec2f pos,
				Renderer_Vec4f tint);
RENDERER_DEF void renderer_text_measure(Renderer *r,
					Renderer_Font *f,				
					char *text,
					u64 text_len,
					Renderer_Vec2f *out);

#ifdef RENDERER_IMPLEMENTATION

#define RENDERER_SHADER_SOURCE(...) #__VA_ARGS__

static char* renderer_vertex_shader_source =
  "#version 330 core\n"
  RENDERER_SHADER_SOURCE(
			 uniform float width;
			 uniform float height;
			 
			 attribute vec2 in_pos;
			 attribute vec4 in_color;
			 attribute vec2 in_uv;

			 varying vec4 color;
			 varying vec2 uv;
			 
			 void main() {
			   vec2 resolution = vec2(width, height);
			   
			   gl_Position = vec4( (in_pos / resolution) * 2.0 - 1.0, 0.0, 1.0);
			   color       = in_color;
			   uv          = in_uv;
			 }
			 );

static char *renderer_fragment_shader_source =
  "#version 330 core\n"
  RENDERER_SHADER_SOURCE(			 
			 uniform sampler2D tex;
			 
			 varying vec4 color;
			 varying vec2 uv;

			 void main() {
			   if(color.a < -1.0) {
			     vec4 c = vec4(color.r, color.g, color.b, -1 * (color.a + 1.0));
			     
			     gl_FragColor = texture2D(tex, vec2(uv.x, 1.0 - uv.y)) * c;
			   } else if(color.a < 0.0) {
			     ivec2 size = textureSize(tex, 0);
			     vec2 actual_uv =
			       vec2(uv.x / float(size.x), 1.0 - uv.y / float(size.y));

			     float f = 1.0;
			     float p = texture2D(tex, actual_uv).r * f;
			     float d = -1.0 * color.a * p;
			     
			     float aaf = fwidth(d);
			     float alpha = smoothstep(0.5 - aaf, 0.5 + aaf, d);

			     gl_FragColor = vec4(color.r,
						 color.g,
						 color.b,
						 d);
			   } else {
			     gl_FragColor = color;
			   }
			   
			 }

			 );

RENDERER_DEF int renderer_shader_compile(GLuint *shader, GLenum type, char *source) {

  *shader = glCreateShader(type);
  glShaderSource(*shader, 1, (const char **) &source, NULL);
  glCompileShader(*shader);  

  GLint compiled = 0;
  glGetShaderiv(*shader, GL_COMPILE_STATUS, &compiled);

  return compiled;  
}

RENDERER_DEF int renderer_program_link(GLuint *program, GLuint vertex_shader, GLuint fragment_shader) {

  *program = glCreateProgram();
  glAttachShader(*program, vertex_shader);
  glAttachShader(*program, fragment_shader);

  glLinkProgram(*program);
  
  GLint linked = 0;
  glGetProgramiv(*program, GL_LINK_STATUS, &linked);

  return linked;
  
}

#define RENDERER_VERTEX_ATTR_POS 0
#define RENDERER_VERTEX_ATTR_COLOR 1
#define RENDERER_VERTEX_ATTR_UV 2

RENDERER_DEF int renderer_open(Renderer *r) {

  glGenVertexArrays(1, &r->vao);
  glBindVertexArray(r->vao);

  glGenBuffers(1, &r->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
  glBufferData(GL_ARRAY_BUFFER,
	       sizeof(Renderer_Vertex) * RENDERER_VERTICIES_CAP,
	       r->verticies,
	       GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(RENDERER_VERTEX_ATTR_POS);
  glVertexAttribPointer(RENDERER_VERTEX_ATTR_POS,
			sizeof(Renderer_Vec2f)/sizeof(f32),
			GL_FLOAT,
			GL_FALSE,
			sizeof(Renderer_Vertex),
			(GLvoid *) offsetof(Renderer_Vertex, pos));

  glEnableVertexAttribArray(RENDERER_VERTEX_ATTR_COLOR);
  glVertexAttribPointer(RENDERER_VERTEX_ATTR_COLOR,
			sizeof(Renderer_Vec4f)/sizeof(f32),
			GL_FLOAT,
			GL_FALSE,
			sizeof(Renderer_Vertex),
			(GLvoid *) offsetof(Renderer_Vertex, color));

  glEnableVertexAttribArray(RENDERER_VERTEX_ATTR_UV);
  glVertexAttribPointer(RENDERER_VERTEX_ATTR_UV,
			sizeof(Renderer_Vec2f)/sizeof(f32),
			GL_FLOAT,
			GL_FALSE,
			sizeof(Renderer_Vertex),
 (GLvoid *) offsetof(Renderer_Vertex, uv));
	
  if(!renderer_shader_compile(&r->vertex_shader, GL_VERTEX_SHADER, renderer_vertex_shader_source)) {
    return 0;
  }
    
  if(!renderer_shader_compile(&r->fragment_shader, GL_FRAGMENT_SHADER, renderer_fragment_shader_source)) {
    return 0;
  }

  if(!renderer_program_link(&r->program, r->vertex_shader, r->fragment_shader)) {
    return 0;
  }
  glUseProgram(r->program);

  r->verticies_len = 0;
  r->texture_count  = 0;
  r->texture_active  = -1;

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  return 1;
}

RENDERER_DEF void renderer_begin(Renderer *r, f32 width, f32 height) {

  glViewport(0, 0, (GLsizei) width, (GLsizei) height);
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(r->background.x, r->background.y, r->background.z, 1);

  glUniform1f(glGetUniformLocation(r->program, "width"), width);
  glUniform1f(glGetUniformLocation(r->program, "height"), height);
  if(r->texture_active >= 0) {
    glUniform1i(glGetUniformLocation(r->program, "tex"), r->texture_active);
  }

  r->viewport.x = width;
  r->viewport.y = height;
}

RENDERER_DEF void renderer_end(Renderer *r) {

  glBufferSubData(GL_ARRAY_BUFFER, 
		  0, 
		  r->verticies_len * sizeof(r->verticies[0]),
		  r->verticies);
  glDrawArrays(GL_TRIANGLES,
	       0, 	
	       (GLsizei) r->verticies_len);
  r->verticies_len = 0;
  
}

RENDERER_DEF int renderer_texture_create(Renderer_Texture *t,
					 Renderer *r,
					 void *data,
					 f32 width,
					 f32 height,
					 int is_grey) {

  if(!renderer_texture_prepare(t, r, width, height, is_grey)) {
    return 0;
  }

  return renderer_texture_write(t, data, renderer_vec2f(0, 0), renderer_vec2f(width, height));
}

RENDERER_DEF int renderer_texture_prepare(Renderer_Texture *t,
					  Renderer *r,
					  f32 width,
					  f32 height,
					  int is_grey) {
  GLenum current_texture;
  switch(r->texture_count) {
  case 0:
    current_texture = GL_TEXTURE0;
    break;
  case 1:
    current_texture = GL_TEXTURE1;
    break;
  case 2:
    current_texture = GL_TEXTURE2;
    break;
  case 3:
    current_texture = GL_TEXTURE3;
    break;
  default:
    return 0;
  }

  glActiveTexture(current_texture);

  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  if(is_grey) {

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    
    glTexImage2D(GL_TEXTURE_2D,
		 0,
		 GL_RED,
		 (GLsizei) width,
		 (GLsizei)height,
		 0,
		 GL_RED,
		 GL_UNSIGNED_BYTE,
		 NULL);
  } else {

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexImage2D(GL_TEXTURE_2D,
		 0,
		 GL_RGBA,
		 (GLsizei) width,
		 (GLsizei) height,
		 0,
		 GL_RGBA,
		 GL_UNSIGNED_INT_8_8_8_8_REV,
		 NULL);
  }

  t->index   = r->texture_count++;
  t->width   = width;
  t->height  = height;
  t->is_grey = is_grey;
    
  r->texture_active = t->index;
  return 1;
}

RENDERER_DEF int renderer_texture_write(Renderer_Texture *t,
					void *data,
					Renderer_Vec2f off,
					Renderer_Vec2f size) {

  if(off.x + size.x > t->width || off.y + size.y > t->height ||
     off.x < 0                 || off.y < 0) {
    return 0;
  }

  GLenum current_texture;
  switch(t->index) {
  case 0:
    current_texture = GL_TEXTURE0;
    break;
  case 1:
    current_texture = GL_TEXTURE1;
    break;
  case 2:
    current_texture = GL_TEXTURE2;
    break;
  case 3:
    current_texture = GL_TEXTURE3;
    break;
  default:
    return 0;
  }

  glActiveTexture(current_texture);

  if(t->is_grey) {
    glTexSubImage2D(GL_TEXTURE_2D,
		    0,
		    (GLsizei) off.x,
		    (GLsizei) off.y,
		    (GLsizei) size.x,
		    (GLsizei) size.y,
		    GL_RED,
		    GL_UNSIGNED_BYTE,
		    data);
  } else {
    glTexSubImage2D(GL_TEXTURE_2D,
		    0,
		    (GLsizei) off.x,
		    (GLsizei) off.y,
		    (GLsizei) size.x,
		    (GLsizei) size.y,
		    GL_RGBA,
		    GL_UNSIGNED_INT_8_8_8_8_REV,
		    data);
  }
  
  return 1;
}

RENDERER_DEF void renderer_triangle(Renderer *r,
				    Renderer_Vec2f p1, Renderer_Vec4f c1, Renderer_Vec2f uv1,
				    Renderer_Vec2f p2, Renderer_Vec4f c2, Renderer_Vec2f uv2,
				    Renderer_Vec2f p3, Renderer_Vec4f c3, Renderer_Vec2f uv3) {
  
  if(r->verticies_len + 3 > RENDERER_VERTICIES_CAP) {
    renderer_end(r);
  }

  r->verticies[r->verticies_len++] = renderer_vertex(p1, c1, uv1);
  r->verticies[r->verticies_len++] = renderer_vertex(p2, c2, uv2);
  r->verticies[r->verticies_len++] = renderer_vertex(p3, c3, uv3);
  
}

RENDERER_DEF void renderer_texture(Renderer *r,
				   Renderer_Texture *t,
				   Renderer_Vec2f pos,
				   Renderer_Vec2f size,
				   Renderer_Vec2f uvp,
				   Renderer_Vec2f uvs,
				   Renderer_Vec4f tint) {
  
  if(r->texture_active != t->index) {
    renderer_end(r);
    r->texture_active = t->index;
    glUniform1i(glGetUniformLocation(r->program, "tex"), r->texture_active);
  }  
  
  Renderer_Vec4f color;
  if(t->is_grey) {
    color = renderer_vec4f(tint.x, tint.y, tint.z, - tint.w);
  } else {
    color = renderer_vec4f(tint.x, tint.y, tint.z, -1 - tint.w);
  }
  renderer_triangle(r,
		    pos,                                            color, uvp,
		    renderer_vec2f(pos.x + size.x, pos.y),          color, renderer_vec2f(uvp.x + uvs.x, uvp.y),
		    renderer_vec2f(pos.x + size.x, pos.y + size.y), color, renderer_vec2f(uvp.x + uvs.x, uvp.y + uvs.y));
  renderer_triangle(r,
		    pos,                                            color, uvp,
		    renderer_vec2f(pos.x, pos.y + size.y),          color, renderer_vec2f(uvp.x, uvp.y + uvs.y),
		    renderer_vec2f(pos.x + size.x, pos.y + size.y), color, renderer_vec2f(uvp.x + uvs.x, uvp.y + uvs.y));

}

RENDERER_DEF void renderer_text(Renderer *r,
			        Renderer_Font *f,				
				char *text,
				u64 text_len,
				Renderer_Vec2f pos,
				Renderer_Vec4f tint) {

  for(u64 i=0;i<text_len;i++) {
    
    Renderer_Glyph g = f->glyphs[text[i] - 32];

    float w = g.x1 - g.x0;
    float h = g.y1 - g.y0;
      
    renderer_texture(r,
		     &f->texture,
		     renderer_vec2f(pos.x + g.dx, pos.y + g.dy),
		     renderer_vec2f(w, h),
		     renderer_vec2f(g.x0, g.y0),
		     renderer_vec2f(w, h),
		     tint);
    pos.x += g.xadvance;
    
  }
  
}

RENDERER_DEF void renderer_text_measure(Renderer *r,
					Renderer_Font *f,				
					char *text,
					u64 text_len,
					Renderer_Vec2f *out) {
  (void) r;
  out->x = 0;
  out->y = 0;
  
  for(u64 i=0;i<text_len;i++) {
    Renderer_Glyph g = f->glyphs[text[i] - 32];
    float h = g.y1 - g.y0;
    
    out->x += g.xadvance;
    if(h > out->y) out->y = h;
  }
  
}

RENDERER_DEF void renderer_close(Renderer *r) {
  (void) r;
}

#endif // RENDERER_IMPLEMENTATION

#undef f32
#undef s32
#undef u64

#endif // RENDERER_H
