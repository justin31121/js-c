#include <stdio.h>

#define FRAME_IMPLEMENTATION
#include <core/frame.h>

#define RENDERER_IMPLEMENTATION
#include <core/renderer.h>

#define MUI_IMPLEMENTATION
#include <core/mui.h>

#define IO_IMPLEMENTATION
#include <core/io.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <thirdparty/stb_truetype.h>

#include <core/types.h>

int load_font(char *font_file_path,
	      float font_height,
	      Renderer *r,
	      Renderer_Font *f) {

  int width   = (int) (font_height * 10.0f);
  int height  = 0;
  int PADDING = 1;
  int NUMBER_OF_GLYPHS = 96;

  int result = 1;
  char *ttf_data = NULL; // io_slurp_file
  size_t ttf_data_len;
  char *bitmap = NULL;   // malloc
  f->glyphs = NULL;      // malloc
  
  if(io_slurp_filec(font_file_path, (u8 **) &ttf_data, &ttf_data_len) != IO_ERROR_NONE) {
    return_defer(0);
  }

  stbtt_fontinfo font_info;
  if (!stbtt_InitFont(&font_info, (const unsigned char *) ttf_data, 0)) {
    return_defer(0);
  }
  float scale = stbtt_ScaleForPixelHeight(&font_info, font_height);

  int x = 1;
  int y = 1;  
  for(int i=0;i<NUMBER_OF_GLYPHS;i++) {
    int glyph = stbtt_FindGlyphIndex(&font_info, ' ' + i);

    int x0, y0, x1, y1;
    stbtt_GetGlyphBitmapBox(&font_info,
			    glyph,
			    scale,
			    scale, &x0, &y0, &x1, &y1);

    int glyph_width  = x1-x0 + PADDING;
    int glyph_height = y1-y0 + PADDING;

    if(x + glyph_width + 1 >= width) {
      y = height;
      x = 1;
    }    
    x = x + glyph_width + 1;
    
    if (y+glyph_height+1 > height) {
      height = y + glyph_height + 1;
    }
    
  }

  bitmap = malloc(width * height);
  if(!bitmap) {
    return_defer(0);
  }
  memset(bitmap, 0, width * height);

  f->glyphs = malloc(sizeof(Renderer_Glyph) * NUMBER_OF_GLYPHS);
  if(!f->glyphs) {
    return_defer(0);
  }
  f->glyphs_len = NUMBER_OF_GLYPHS;
  f->size = font_height;
  f->margin = ceilf(font_height * 1.25f);    

  x = 1;
  y = 1;
  int bottom_y = 1;
  for(int i=0;i<NUMBER_OF_GLYPHS;i++) {
    char c = ' ' + (char) i;

    int glyph = stbtt_FindGlyphIndex(&font_info, c);
    
    int advance, lsb;
    stbtt_GetGlyphHMetrics(&font_info,
			   glyph,
			   &advance,
			   &lsb);

    int x0, y0, x1, y1;
    stbtt_GetGlyphBitmapBox(&font_info,
			   glyph,
			   scale,
			   scale,
			   &x0, &y0, &x1, &y1);

    int glyph_width  = x1-x0 + PADDING;
    int glyph_height = y1-y0 + PADDING;

    if(x + glyph_width + 1 >= width) {
      y = bottom_y;
      x = 1;
    }

    int bw, bh, bx ,by;
    unsigned char *glyph_bitmap =
      stbtt_GetCodepointSDF(&font_info, scale, c, PADDING, 128, 256.0, &bw, &bh, &bx, &by);
    //stbtt_GetCodepointBitmap(&font_info, 0, scale, mc, &mw, &mh, &mx, &my);
    
    for (int q=0;q<glyph_height;++q) {
      for (int p=0;p<glyph_width; ++p) {
	if(glyph_bitmap && p<bw && q<bh) {
	  bitmap[(y + q)*width + (x + p)] = glyph_bitmap[q*bw+p];
	}
      }
    }

    if(glyph_bitmap) stbtt_FreeBitmap(glyph_bitmap, font_info.userdata);

    f->glyphs[i] = (Renderer_Glyph) {
      .x0 = (float) x,
      .y0 = (float) height - (float) (y + glyph_height),
      .x1 = (float) (x + glyph_width),
      .y1 = (float) height - (float) y,      
      .dx = (float) x0,
      .dy = -1 * ((float) y0) - (float) glyph_height,
      .xadvance = scale * (float) advance,
    };

    x = x + glyph_width + 1;
    if (y + glyph_height + 1 > bottom_y) {
      bottom_y = y+glyph_height+1;  
    }
  }

  if(!renderer_texture_create(&f->texture,
			      r,
			      bitmap,
			      (f32) width,
			      (f32) height,
			      1)) {
    return_defer(0);
  }
  
 defer:
  if(ttf_data != NULL) free(ttf_data);
  if(bitmap != NULL) free(bitmap);
  if(!result && f->glyphs) free(f->glyphs);
  
  return result;
}


void mui_render(void *userdata, Mui* m) {
  Renderer *r = userdata;
  
  if(m->type == MUI_TYPE_TEXTURE) {
    Renderer_Texture *t = m->userdata;

    if(t->is_grey) {
      m->c1.w = -1 * m->c1.w;
      m->c2.w = -1 * m->c2.w;
      m->c3.w = -1 * m->c3.w;      
    } else {
      m->c1.w = -1 - m->c1.w;
      m->c2.w = -1 - m->c2.w;
      m->c3.w = -1 - m->c3.w;
    }

    if(r->texture_active != t->index) {
      renderer_end(r);
      r->texture_active = t->index;
      glUniform1i(glGetUniformLocation(r->program, "tex"), r->texture_active);
    }    
  }  

  renderer_triangle(r,
		    cast(Renderer_Vec2f, m->p1), cast(Renderer_Vec4f, m->c1), cast(Renderer_Vec2f, m->uv1),
		    cast(Renderer_Vec2f, m->p2), cast(Renderer_Vec4f, m->c2), cast(Renderer_Vec2f, m->uv2),
		    cast(Renderer_Vec2f, m->p3), cast(Renderer_Vec4f, m->c3), cast(Renderer_Vec2f, m->uv3));  
}

typedef struct {
  float p;  
  int dragging;
  int clicking_up;
  int clicking_down;
} Bar;

void bar_update_render(Bar *b,
		       Mui_Vec2f mouse,
		       int mouse_pressed,
		       int mouse_released,
		       Mui *m,
		       Mui_Vec2f pos,
		       Mui_Vec2f size,
		       Mui_Vec4f color) {

  float PADDING        = 0.375f;
  float BAR_HEIGHT     =  8.f;
  Mui_Vec2f KNOB_SIZE  = mui_vec2f(16.f, 16.f);
  
  float px = size.x * PADDING;

  pos  = mui_vec2f(pos.x + px/2, pos.y + size.y/2 - BAR_HEIGHT/2);
  size = mui_vec2f(size.x - px, BAR_HEIGHT);

  Mui_Vec2f sign_size = mui_vec2f(px * 3 / 16, BAR_HEIGHT);
  if(mouse_released) {
    b->clicking_up = 0;
    b->clicking_down = 0;
  }

  Mui_Vec2f plus_pos  = mui_vec2f(pos.x - px/4 - sign_size.x/2, pos.y + BAR_HEIGHT * 3 / 4);
  int mouse_in_plus = mui_point_in_rect(mouse, plus_pos, sign_size);
  float plus_grey = 1.0f;
  if(mouse_in_plus || b->clicking_up) {
    plus_grey = .8f;
  }

  Mui_Vec4f plus_color = mui_vec4f(plus_grey, plus_grey, plus_grey, 1.0f);
  mui_rect(m, plus_pos, sign_size, plus_color);
  float foo = sign_size.y * 0.25f;
  mui_rect(m,
	   mui_vec2f(plus_pos.x + sign_size.x/2 - sign_size.y/2,
		     plus_pos.y + sign_size.y/2 - foo/2),
	   mui_vec2f(sign_size.y, foo),
	   mui_vec4f(0, 0, 0, 1));
  mui_rect(m,
	   mui_vec2f(plus_pos.x + sign_size.x/2 - foo/2,
		     plus_pos.y),
	   mui_vec2f(foo, sign_size.y),
	   mui_vec4f(0, 0, 0, 1));
  
  if(mouse_pressed && mui_point_in_rect(mouse, plus_pos, sign_size)) {
    b->clicking_up = 1;
  }  
  if(b->clicking_up) {
    b->p = mui_clip(b->p + 1.f * 0.16f / 255.f, 0.0f, 1.0f);
  }
  
  Mui_Vec2f minus_pos = mui_vec2f(pos.x - px/4 - sign_size.x/2, pos.y - BAR_HEIGHT * 3 / 4);  
  int mouse_in_minus = mui_point_in_rect(mouse, minus_pos, sign_size);
  float minus_grey = 1.0f;
  if(mouse_in_minus || b->clicking_down) {
    minus_grey = .8f;
  }
  
  Mui_Vec4f minus_color = mui_vec4f(minus_grey, minus_grey, minus_grey, 1.0f);  
  mui_rect(m, minus_pos, sign_size, minus_color);
  mui_rect(m,
	   mui_vec2f(minus_pos.x + sign_size.x/2 - sign_size.y/2,
		     minus_pos.y + sign_size.y/2 - foo/2),
	   mui_vec2f(sign_size.y, foo),
	   mui_vec4f(0, 0, 0, 1));
  
  if(mouse_pressed && mouse_in_minus) {
    b->clicking_down = 1;
  }
  if(b->clicking_down) {
    b->p = mui_clip(b->p - 1.f * 0.16f / 255.f, 0.0f, 1.0f);
  }
  
  Mui_Vec2f knob_pos = mui_vec2f(pos.x + (b->p * size.x) - KNOB_SIZE.x/2,
				 pos.y + size.y/2 - KNOB_SIZE.y/2);
  
  int mouse_in_knob = mui_point_in_rect(mouse,
					knob_pos,
					KNOB_SIZE);
  if(mouse_pressed && mouse_in_knob) {
    b->dragging = 1;
  }
  if(mouse_released) {
    b->dragging = 0;
  }

  if(b->dragging) {
    b->p     = mui_clip((mouse.x - pos.x) / size.x, 0.0f, 1.0f);
    knob_pos = mui_vec2f(pos.x + (b->p * size.x) - KNOB_SIZE.x/2,
			 pos.y + size.y/2 - KNOB_SIZE.y/2);
  }

  float grey = 1.0f;
  if(mouse_in_knob || b->dragging) {
    grey = .8f;
  }
  else if(mui_point_in_rect(mouse, pos, size)) {
    grey = .9f;
  }

  Mui_Vec4f knob_color = color;
  knob_color.x *= grey;
  knob_color.y *= grey;
  knob_color.z *= grey;

  mui_rect(m,
	   pos,
	   size,
	   color);
    
  mui_rect(m,
	   knob_pos,
	   KNOB_SIZE,
	   knob_color);
}


int main() {
  
  Frame frame;
  switch(frame_open(&frame, 600, 400, 0)) {
  case FRAME_ERROR_NONE:
    // pass
    break;
  default:
    TODO();
    break;
  }
  frame_set_title(&frame, "Color picker");

  Renderer renderer;
  if(!renderer_open(&renderer)) {
    return 1;
  }

  Renderer_Font font;
  if(!load_font("C:\\Windows\\Fonts\\segoeui.ttf",
		22.0f,
		&renderer,
		&font)) {
    return 1;
  }

  Mui mui = {0};
  mui.font = mui_font(&font.texture.index,
		      font.size,
		      font.margin,
		      (Mui_Glyph *) font.glyphs,
		      font.glyphs_len);
  mui.render = mui_render;
  mui.render_userdata = &renderer;
  
  Bar bar_hue	     = { .dragging = 0, .p = 1.0f, .clicking_up = 0, .clicking_down = 0 };
  Bar bar_saturation = { .dragging = 0, .p = .5f, .clicking_up = 0, .clicking_down = 0};
  Bar bar_value	     = { .dragging = 0, .p = .5f, .clicking_up = 0, .clicking_down = 0};

  Bar bar_red	= { .dragging = 0, .p = 1.0f, .clicking_up = 0, .clicking_down = 0};
  Bar bar_green	= { .dragging = 0, .p = .5f, .clicking_up = 0, .clicking_down = 0};
  Bar bar_blue	= { .dragging = 0, .p = .5f, .clicking_up = 0, .clicking_down = 0};

  int hsv_was_last_modified = 1;

  Mui_Vec4f hsv_last = mui_vec4f(bar_hue.p,
				 0.69f, // such that on the first frame => hsv_was_last_modified = 1
				 bar_value.p,
				 1.0f);

  Frame_Event event;
  while(frame.running) {
    int mouse_pressed = 0;
    int mouse_released = 0;
    
    while(frame_peek(&frame, &event)) {
      switch(event.type) {
      case FRAME_EVENT_KEYPRESS:
	if(event.as.key == 'Q') frame.running = 0;
	break;
      case FRAME_EVENT_MOUSEPRESS:
	mouse_pressed = 1;
	break;
      case FRAME_EVENT_MOUSERELEASE:
	mouse_released = 1;
	break;
      default:
	break;
      }
    }

    Mui_Vec4f rgb = mui_vec4f(bar_red.p,
			      bar_green.p,
			      bar_blue.p,
			      1.0f);
    Mui_Vec4f hsv = mui_vec4f(bar_hue.p,
			      bar_saturation.p,
			      bar_value.p,
			      1.0f);

    if(hsv_last.x != hsv.x ||
       hsv_last.y != hsv.y ||
       hsv_last.z != hsv.z) {
      hsv_was_last_modified = 1;
    } else {
      hsv_was_last_modified = 0;
    }
        
    if(hsv_was_last_modified) {
      hsv = mui_vec4f(bar_hue.p,
		      bar_saturation.p,
		      bar_value.p,
		      1.0f);
      mui_hsv_to_rgb(&hsv, &rgb);
      bar_red.p   = rgb.x;
      bar_green.p = rgb.y;
      bar_blue.p  = rgb.z; 
    } else {
      rgb = mui_vec4f(bar_red.p,
		      bar_green.p,
		      bar_blue.p,
		      1.0f);
      mui_rgb_to_hsv(&rgb, &hsv);
      bar_hue.p        = hsv.x;
      bar_saturation.p = hsv.y;
      bar_value.p      = hsv.z;
    }

    hsv_last = hsv;    
    
    renderer.background = cast(Renderer_Vec4f, rgb);
    renderer_begin(&renderer, frame.width, frame.height);

    unsigned int rgb_hex =
      ((unsigned char) (rgb.w * 0xff)) |
      ((unsigned char) (rgb.z * 0xff) << 8) |
      ((unsigned char) (rgb.y * 0xff) << 16) |
      ((unsigned char) (rgb.x * 0xff) << 24);
    char buf[1024];
    size_t buf_len = (size_t) snprintf(buf, sizeof(buf), "0x%08X", rgb_hex);
    if(buf_len >= sizeof(buf)) {
      TODO();
    }
    
    Mui_Vec2f text_size;
    mui_text_measure(&mui,
		     buf,
		     buf_len,
		     &text_size);
    Mui_Vec2f text_pos = mui_vec2f(ceilf(frame.width/2 - text_size.x/2),
				   ceilf(frame.height/2 - text_size.y/2));
    float TEXT_PADDING = 16.f;
    
    mui_rect_rounded(&mui,
		     mui_vec2f(text_pos.x - TEXT_PADDING,
			       text_pos.y - TEXT_PADDING),
		     mui_vec2f(text_size.x + 2 * TEXT_PADDING,
			       text_size.y + 2 * TEXT_PADDING),
		     mui_vec4f(0, 0, 0, 1));
    mui_textc(&mui,
	      text_pos,
	      buf,
	      mui_vec4f(1, 1, 1, 1));

    Mui_Vec2f pos, size;
    Mui_Vec4f bars_color = mui_vec4f(1, 1, 1, 1);
    Mui_Vec2f bars_size = mui_vec2f(200, 180);

    Mui_Vec2f bars_hsv_pos  = mui_vec2f(0, 0);    
    mui_partition_begin(&mui, bars_hsv_pos, bars_size, 3);
        
        mui_partition(&mui, &pos, &size);
        bar_update_render(&bar_hue,
	    	      mui_vec2f(frame.mouse_x, frame.mouse_y),
	    	      mouse_pressed,
	    	      mouse_released,
	    	      &mui,
	    	      pos,
	    	      size,
	    	      bars_color);
    
        mui_partition(&mui, &pos, &size);
        bar_update_render(&bar_saturation,
	    	      mui_vec2f(frame.mouse_x, frame.mouse_y),
	    	      mouse_pressed,
	    	      mouse_released,
	    	      &mui,
	    	      pos,
	    	      size,
	    	      bars_color);
    
        mui_partition(&mui, &pos, &size);
        bar_update_render(&bar_value,
	    	      mui_vec2f(frame.mouse_x, frame.mouse_y),
	    	      mouse_pressed,
	    	      mouse_released,
	    	      &mui,
	    	      pos,
	    	      size,
        	      bars_color);	
    mui_partition_end(&mui);

    Mui_Vec2f bars_rgb_pos  = mui_vec2f(frame.width - bars_size.x, 0);
    mui_partition_begin(&mui, bars_rgb_pos, bars_size, 3);
        
        mui_partition(&mui, &pos, &size);
        bar_update_render(&bar_red,
	    	      mui_vec2f(frame.mouse_x, frame.mouse_y),
	    	      mouse_pressed,
	    	      mouse_released,
	    	      &mui,
	    	      pos,
	    	      size,
	    	      bars_color);
    
        mui_partition(&mui, &pos, &size);
        bar_update_render(&bar_green,
	    	      mui_vec2f(frame.mouse_x, frame.mouse_y),
	    	      mouse_pressed,
	    	      mouse_released,
	    	      &mui,
	    	      pos,
	    	      size,
	    	      bars_color);
    
        mui_partition(&mui, &pos, &size);
        bar_update_render(&bar_blue,
	    	      mui_vec2f(frame.mouse_x, frame.mouse_y),
	    	      mouse_pressed,
	    	      mouse_released,
	    	      &mui,
	    	      pos,
	    	      size,
        	      bars_color);
    mui_partition_end(&mui);
    
    renderer_end(&renderer);

    frame_swap_buffers(&frame);
  }

  frame_close(&frame);
}
