#include <stdio.h>

#define FRAME_IMPLEMENTATION
#include <core/frame.h>

#define RENDERER_IMPLEMENTATION
#include <core/renderer.h>

#define MUI_IMPLEMENTATION
#include <core/mui.h>

// Decoding

#define PNM_IMPLEMENTATION
#include <core/pnm.h>

#define STB_IMAGE_IMPLEMENTATION
#include <thirdparty/stb_image.h>

#define QOI_IMPLEMENTATION
#include <thirdparty/qoi.h>

#include <core/types.h>

#include <stdbool.h>

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

#define PADDING 48
#define BORDER_PADDING 4

static Frame frame;
static Renderer renderer;
static Mui mui;
static Renderer_Texture texture;

float zoom = 1.f;
float initial_zoom = 1.f;
bool show_border = false;

float y_off = 0.f;
float y_start = 0.f;
bool y_drag = false;

float x_off = 0.f;
float x_start = 0.f;
bool x_drag = false;

unsigned int tex;
char *last_path = NULL;
int img_width, img_height;

void load_file(char *path) {

  qoi_desc desc;	
  unsigned char *data = NULL;
  
  data = qoi_read(path, &desc, 4);
  img_width = desc.width;
  img_height = desc.height;
  if(!data) {
    data = pnm_load(path, &img_width, &img_height, NULL, 4);
  }
  if(!data) {
    data = stbi_load(path, &img_width, &img_height, 0, 4);
  }  
  if(!data) {
    fprintf(stderr, "ERROR: Can not open '%s'\n", path); fflush(stderr);
    return; 
  }
  
  last_path = path;
  frame_set_title(&frame, path);

  renderer.texture_count = 0;
  if(!renderer_texture_create(&texture,
			      &renderer,
			      data,
			      (f32) img_width,
			      (f32) img_height,
			      0)) {
    TODO();
  }

  free(data); // all libs use 'free'

  if(img_width > img_height) {
    zoom = ((float) frame.width - 2 * PADDING) / (float) img_width;
  } else {
    zoom = ((float) frame.height - 2 * PADDING) / (float) img_height;
  }
  y_off = 0.f;
  x_off = 0.f;
  initial_zoom = zoom;
}

#define FRAME_DOUBLE_CLICK_TIME_MS 200

int main(int argc, char **argv) {
 
  switch(frame_open(&frame, 800, 800, FRAME_DRAG_N_DROP)) {
  case FRAME_ERROR_NONE:
    // pass
    break;
  default:
    TODO();
    break;
  }

  if(!renderer_open(&renderer)) {
    TODO();
  }

  mui = (Mui) {0};
  mui.render = mui_render;
  mui.render_userdata = &renderer;

  if(argc > 1) {
    load_file(argv[1]);
  }

  ///////////////////////////////////////////////////////////////////////////  

  s32 click_inactive_frames = 0;
#define DOUBLE_CLICK_FRAMES 16
  
  Frame_Event event;
  while(frame.running) {
      
    while(frame_peek(&frame, &event)) {

      switch(event.type) {

      case FRAME_EVENT_MOUSEWHEEL: {
	zoom += 0.025f * (float) event.as.amount;
      } break;

      case FRAME_EVENT_MOUSEPRESS: {

	if(click_inactive_frames < DOUBLE_CLICK_FRAMES) {
	  frame_toggle_fullscreen(&frame);
	} else {
	  y_start = frame.mouse_y - y_off;
	  y_drag = true;
	  x_start = frame.mouse_x - x_off;
	  x_drag = true;
	}	
	click_inactive_frames = 0;
	
      } break;

      case FRAME_EVENT_MOUSERELEASE: {
	y_drag = false;
	x_drag = false;
      } break;
      case FRAME_EVENT_KEYPRESS: {

	switch(event.as.key) {
	case 'B': {
	  show_border = !show_border;
	} break;
	case 'R': {
	  zoom = initial_zoom;
	  x_off = 0.f;
	  y_off = 0.f;
	} break;
	case 'Q': {
	  frame.running = false;
	} break;
	case 'F':{
	  frame_toggle_fullscreen(&frame);
	} break;
	case FRAME_ESCAPE: {
	  if(frame.running & FRAME_FULLSCREEN) {
	    frame_toggle_fullscreen(&frame);
	  }
	} break;
	}
	
      }break;
	
      case FRAME_EVENT_FILEDROP: {

	Frame_Dragged_Files files;
	if(frame_dragged_files_open(&files, &event)) {
	  
	  if(frame_dragged_files_next(&files)) {
	    load_file(files.path);
	  }

	  frame_dragged_files_close(&files);
	}

      } break;
      default: {	
      } break;
      }
      
    }

    if(click_inactive_frames >= DOUBLE_CLICK_FRAMES) click_inactive_frames = DOUBLE_CLICK_FRAMES;
    else click_inactive_frames += 1;

    if(y_drag) {
      y_off = frame.mouse_y - y_start;
    }
    if(x_drag) {
      x_off = frame.mouse_x - x_start;
    }

    renderer_begin(&renderer, frame.width, frame.height);

    float target_width = (float) img_width * zoom;
    float target_height = (float) img_height * zoom;
    
    if(last_path != NULL) {
      float ratio =  (float) img_width / (float) img_height;
      (void) ratio;

      Mui_Vec2f pos = mui_vec2f((float) frame.width/2 - target_width/2 + x_off,
					  (float) frame.height/2 - target_height/2 + y_off);
      Mui_Vec2f size = mui_vec2f(target_width, target_height);

      if(show_border) {

	mui_rect(&mui,
		 mui_vec2f(pos.x - BORDER_PADDING,
			   pos.y - BORDER_PADDING),
		 mui_vec2f(size.x + 2 * BORDER_PADDING,
			   size.y + 2 * BORDER_PADDING),
		 mui_vec4f(1, 1, 1, 1));

      }

      mui_texture(&mui,
		  &texture,
		  pos,
		  size,
		  mui_vec2f(0, 0),
		  mui_vec2f(1, 1),
		  mui_vec4f(1, 1, 1, 1));
    }

    renderer_end(&renderer);
    
    frame_swap_buffers(&frame);
  }

  frame_close(&frame);
  
  return 0;
}
