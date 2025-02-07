#ifndef FRAME_H
#define FRAME_H

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

// # Building for Windows
//
// ``` console
// cl main.c gdi32.lib opengl32.lib user32.lib (shell32.lib)
// gcc -o main main.c -lgdi32 -lopengl32
// ```

// # Building for linux
//
// ``` console
// gcc -o main main.c -lGLX -lX11 -lGL -lm
// ```

#define GL_ARRAY_BUFFER 0x8892
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE3 0x84C3
#define GL_TEXTURE4 0x84C4
#define GL_TEXTURE5 0x84C5
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367

#define FRAME_WIN32_OPENGL_FUNCS			\
  FRAME_OPENGL_FUNC(void, glActiveTexture, GLenum)	\
    FRAME_OPENGL_FUNC(void, wglSwapIntervalEXT, int)

#ifdef _WIN32
#  include <windows.h>
#  include <GL/GL.h>

typedef size_t GLsizeiptr;
typedef char GLchar;
typedef size_t GLintptr;

#  define FRAME_PLATFORM_OPENGL_FUNCS FRAME_WIN32_OPENGL_FUNCS
#  define FRAME_PATH_MAX MAX_PATH
#else // linux
#  include <linux/limits.h>
#  include <X11/Xlib.h>
#  include <X11/XKBlib.h>
#  include <X11/Xutil.h>
#  include <GL/gl.h>
#  include <GL/glx.h>
#  include <errno.h>
#  include <stdlib.h>

#  define FRAME_PLATFORM_OPENGL_FUNCS
#  define FRAME_PATH_MAX PATH_MAX
#endif // _WIN32

#define FRAME_OPENGL_FUNCS						\
  FRAME_PLATFORM_OPENGL_FUNCS						\
  FRAME_OPENGL_FUNC(GLuint, glCreateShader, GLenum)			\
       FRAME_OPENGL_FUNC(void, glShaderSource, GLuint, GLsizei, const GLchar**, const GLint*) \
       FRAME_OPENGL_FUNC(void, glCompileShader, GLuint)			\
       FRAME_OPENGL_FUNC(void, glGetShaderiv, GLuint, GLenum, GLint*)	\
       FRAME_OPENGL_FUNC(void, glGetShaderInfoLog, GLuint, GLsizei, GLsizei*, GLchar*) \
       FRAME_OPENGL_FUNC(GLuint, glCreateProgram, void)			\
       FRAME_OPENGL_FUNC(void, glAttachShader, GLuint, GLuint)		\
       FRAME_OPENGL_FUNC(void, glLinkProgram, GLuint)			\
       FRAME_OPENGL_FUNC(void, glGetProgramiv, GLuint, GLenum, GLint*)	\
       FRAME_OPENGL_FUNC(void, glGetProgramInfoLog, GLuint, GLsizei, GLsizei*, GLchar*) \
       FRAME_OPENGL_FUNC(void, glGenVertexArrays, GLsizei, GLuint*)	\
       FRAME_OPENGL_FUNC(void, glUseProgram, GLuint)			\
       FRAME_OPENGL_FUNC(void, glBindVertexArray, GLuint)		\
       FRAME_OPENGL_FUNC(void, glGenBuffers, GLsizei, GLuint*)		\
  FRAME_OPENGL_FUNC(void, glBindBuffer, GLenum, GLuint)			\
       FRAME_OPENGL_FUNC(void, glBufferData, GLenum, GLsizeiptr, const void*, GLenum) \
       FRAME_OPENGL_FUNC(void, glEnableVertexAttribArray, GLuint)	\
       FRAME_OPENGL_FUNC(void, glVertexAttribPointer, GLuint, GLint, GLenum, GLboolean, GLsizei, const void*) \
  FRAME_OPENGL_FUNC(void, glBufferSubData, GLenum, GLintptr, GLsizeiptr, const void*) \
  FRAME_OPENGL_FUNC(void, glUniform1f, GLint, GLfloat)			\
  FRAME_OPENGL_FUNC(void, glUniform1i, GLint, GLint)			\
       FRAME_OPENGL_FUNC(GLint, glGetUniformLocation, GLuint, const GLchar*)

typedef unsigned char Frame_u8;
typedef int Frame_s32;
typedef unsigned int Frame_u32;
typedef float Frame_f32;
typedef long long Frame_s64;
typedef unsigned long long Frame_u64;

#define u8 Frame_u8
#define s32 Frame_s32
#define u32 Frame_u32
#define f32 Frame_f32
#define s64 Frame_s64
#define u64 Frame_u64

#ifndef FRAME_DEF
#  define FRAME_DEF static inline
#endif // FRAME_DEF

typedef enum {
  FRAME_ERROR_NONE = 0,
  
  FRAME_ERROR_CANNOT_LOAD_OPENGL_FUNC,
} Frame_Error;

FRAME_DEF Frame_Error frame_error_last();

typedef struct {
#ifdef _WIN32
  HWND hwnd;
  HDC hdc;
  RECT rect;
  POINT point;
#else // linux
  Display *display;
  Window window;
  Colormap colormap;
  s32 atom_delete_window;
  GLXContext context;
  s32 fd;
#endif //_WIN32
  
  s32 running;
  f32 width;
  f32 height;
  
  int mouse_visible;
  f32 mouse_x, mouse_y;
} Frame;

typedef enum {
  FRAME_EVENT_NONE = 0,
  FRAME_EVENT_KEYPRESS,
  FRAME_EVENT_KEYRELEASE,
  FRAME_EVENT_MOUSEPRESS,
  FRAME_EVENT_MOUSERELEASE,
  FRAME_EVENT_MOUSEWHEEL,
  FRAME_EVENT_FILEDROP,
} Frame_Event_Type;

#define FRAME_PLUS VK_OEM_PLUS
#define FRAME_MINUS VK_OEM_MINUS
#define FRAME_BACKSPACE 8
#define FRAME_TAB VK_TAB
#define FRAME_ESCAPE 27
#define FRAME_SPACE 32
#define FRAME_ARROW_LEFT 37
#define FRAME_ARROW_UP 38
#define FRAME_ARROW_RIGHT 39
#define FRAME_ARROW_DOWN 40
#define FRAME_SHIFT VK_SHIFT
#define FRAME_ALT VK_MENU
#define FRAME_CTRL VK_CONTROL

typedef struct {
#ifdef _WIN32
  MSG msg;
#else // linux
  XEvent x_event;
#endif // _WIN32
  
  Frame_Event_Type type;
  union {
    char key;
    s32 amount;
    s64 value;
  } as;
} Frame_Event;

typedef struct{
#ifdef _WIN32
  HDROP h_drop;
  wchar_t wpath[FRAME_PATH_MAX];
#endif // _WIN32
    
  char path[FRAME_PATH_MAX];

  s32 count;
  s32 index;
}Frame_Dragged_Files;

#define FRAME_RUNNING       0x1
#define FRAME_NOT_RESIZABLE 0x2
#define FRAME_DRAG_N_DROP   0x4
#define FRAME_FULLSCREEN    0x8

FRAME_DEF Frame_Error frame_open(Frame *f, s32 width, s32 height, s32 flags);
FRAME_DEF int frame_peek(Frame *f, Frame_Event *e);
FRAME_DEF void frame_swap_buffers(Frame *f);
FRAME_DEF Frame_Error frame_toggle_fullscreen(Frame *f);
FRAME_DEF void frame_set_mouse_visible(Frame *f, int visible);
FRAME_DEF Frame_Error frame_set_icon(Frame *f, u8 *pixels, u64 pixels_width, u64 pixels_height);
FRAME_DEF Frame_Error frame_set_title(Frame *f, char *title);
FRAME_DEF void frame_close(Frame *f);

FRAME_DEF int frame_dragged_files_open(Frame_Dragged_Files *files, Frame_Event *event);
FRAME_DEF int frame_dragged_files_next(Frame_Dragged_Files *files);
FRAME_DEF void frame_dragged_files_close(Frame_Dragged_Files *files);

#define FRAME__EXPAND(args) args
#define FRAME_CONCAT(a, b) a ## b
#define FRAME_HEAD(x, ...) x

#define FRAME_BAZZ_default() return
#define FRAME_BAZZ_GLuint FRAME_BAZZ_default
#define FRAME_BAZZ_GLint FRAME_BAZZ_default
#define FRAME_BAZZ_void()

#define FRAME_BAZZ(t) FRAME_CONCAT(FRAME_BAZZ_, t)()

#define FRAME_BAR1(a) z
#define FRAME_BAR2(a,b) z, y
#define FRAME_BAR3(a,b,c) z, y, x
#define FRAME_BAR4(a,b,c,d) z, y, x, w
#define FRAME_BAR5(a,b,c,d,e) z, y, x, w, v
#define FRAME_BAR6(a,b,c,d,e,f) z, y, x, w, v, u
#define FRAME_BAR7(a,b,c,d,e,f,g) z, y, x, w, v, u, t
#define GETFRAME_BAR(_1,_2,_3,_4,_5,_6,_7, FRAME_BARN,...) FRAME_BARN

#define FRAME_BAR_default(...) FRAME__EXPAND(GETFRAME_BAR(__VA_ARGS__, FRAME_BAR7, FRAME_BAR6, FRAME_BAR5, FRAME_BAR4, FRAME_BAR3, FRAME_BAR2, FRAME_BAR1)(__VA_ARGS__))
#define FRAME_BAR_GLenum FRAME_BAR_default
#define FRAME_BAR_GLuint FRAME_BAR_default
#define FRAME_BAR_GLsizei FRAME_BAR_default
#define FRAME_BAR_GLint FRAME_BAR_default
#define FRAME_BAR_int FRAME_BAR_default
#define FRAME_BAR_void(...)

#define FRAME_BAR_IMPL(h, ...) FRAME_CONCAT(FRAME_BAR_, h)(__VA_ARGS__)
#define FRAME_BAR(...) FRAME_BAR_IMPL(FRAME_HEAD(__VA_ARGS__), __VA_ARGS__)

#define FRAME_FOO1(a) a z
#define FRAME_FOO2(a,b) a z, b y
#define FRAME_FOO3(a,b,c) a z, b y, c x
#define FRAME_FOO4(a,b,c,d) a z, b y, c x, d w
#define FRAME_FOO5(a,b,c,d,e) a z, b y, c x, d w, e v
#define FRAME_FOO6(a,b,c,d,e,f) a z, b y, c x, d w, e v, f u
#define FRAME_FOO7(a,b,c,d,e,f,g) a z, b y, c x, d w, e v, f u, g t
#define GETFRAME_FOO(_1,_2,_3,_4,_5,_6,_7, FRAME_FOON, ...) FRAME_FOON

#define FRAME_FOO_default(...) FRAME__EXPAND(GETFRAME_FOO(__VA_ARGS__, FRAME_FOO7, FRAME_FOO6, FRAME_FOO5, FRAME_FOO4, FRAME_FOO3, FRAME_FOO2, FRAME_FOO1)(__VA_ARGS__))
#define FRAME_FOO_GLenum FRAME_FOO_default
#define FRAME_FOO_GLuint FRAME_FOO_default
#define FRAME_FOO_GLsizei FRAME_FOO_default
#define FRAME_FOO_GLint FRAME_FOO_default
#define FRAME_FOO_int FRAME_FOO_default
#define FRAME_FOO_void(...)

#define FRAME_FOO_IMPL(h, ...) FRAME_CONCAT(FRAME_FOO_, h)(__VA_ARGS__)
#define FRAME_FOO(...) FRAME_FOO_IMPL(FRAME_HEAD(__VA_ARGS__), __VA_ARGS__)

#define FRAME_OPENGL_FUNC(return_type, func_name, ...)		\
  FRAME_DEF return_type func_name ( FRAME_FOO(__VA_ARGS__) );
FRAME_OPENGL_FUNCS
#undef FRAME_OPENGL_FUNC

#ifdef FRAME_IMPLEMENTATION

#define FRAME_OPENGL_FUNC(return_type, func_name, ...)			\
  typedef return_type ( * func_name##_t ) (__VA_ARGS__);		\
  static func_name##_t __##func_name = NULL;				\
  FRAME_DEF return_type func_name ( FRAME_FOO(__VA_ARGS__) ) {		\
    FRAME_BAZZ(return_type) __##func_name ( FRAME_BAR(__VA_ARGS__) ) ;	\
  }
FRAME_OPENGL_FUNCS
#undef FRAME_OPENGL_FUNC

#ifdef _WIN32

FRAME_DEF Frame_Error frame_error_last() {
  DWORD last_error = GetLastError();
  
  switch(last_error) {
  case 0:
    return FRAME_ERROR_NONE;
  default:
    fprintf(stderr, "FRAME_ERROR: Unhandled last_error: %ld\n", last_error); fflush(stderr);
    exit(1);
  }

}

FRAME_DEF void frame_set_mouse_visible(Frame *f, int visible) {
  if((!(!visible)) != (!(!f->mouse_visible))) {
    ShowCursor(visible);  
  }
  f->mouse_visible = visible;
}

// TODO: This probaply leaks memory
FRAME_DEF Frame_Error frame_set_icon(Frame *f, u8 *pixels, u64 pixels_width, u64 pixels_height) {

  HICON icon;
  
  ICONINFO icon_info = {0};
  icon_info.fIcon = TRUE;
  
  // swap RED and BLUE for windows
  u32 *ps = (u32 *) pixels;
  for(u64 i=0;i<pixels_width*pixels_height;i++) {
    u32 p = ps[i];    
    u8 r = (u8) ((p & 0xff) >> 0);
    u8 b = (u8) ((p & 0xff0000) >> 16);
    ps[i] = (b) | (p & 0xff00) | (r << 16) | (p & 0xff000000);
  }
  
  icon_info.hbmColor = CreateBitmap((s32) pixels_width, (s32) pixels_height, 1, 32, pixels);
  if (icon_info.hbmColor) {
    icon_info.hbmMask = CreateCompatibleBitmap(f->hdc, (s32) pixels_width, (s32) pixels_height);
    if (icon_info.hbmMask) {
      icon = CreateIconIndirect(&icon_info);
      if (icon == NULL) {
	return frame_error_last();
      }
      DeleteObject(icon_info.hbmMask);
    } else {
      return frame_error_last();
    }
    DeleteObject(icon_info.hbmColor);
  } else {
    return frame_error_last();
  }

  // swap BLUE and RED back
  for(u64 i=0;i<pixels_width*pixels_height;i++) {
    u32 p = ps[i];    
    u8 b = (u8) ((p & 0xff) >> 0);
    u8 r = (u8) ((p & 0xff0000) >> 16);
    ps[i] = (r) | (p & 0xff00) | (b << 16) | (p & 0xff000000);
  }
  
  SendMessage(f->hwnd, WM_SETICON, ICON_SMALL, (LPARAM) icon);
  SendMessage(f->hwnd, WM_SETICON, ICON_BIG, (LPARAM) icon);
  SendMessage(f->hwnd, WM_SETICON, ICON_SMALL2, (LPARAM) icon);

  return FRAME_ERROR_NONE;
}

FRAME_DEF Frame_Error frame_set_title(Frame *f, char *title) {
  if(SetWindowTextA(f->hwnd, title)) {
    return FRAME_ERROR_NONE;
  } else {
    return frame_error_last();
  }
}

LRESULT CALLBACK frame_window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

  if(message == WM_CLOSE ||
     message == WM_DESTROY) {
    Frame *f = (Frame *) GetWindowLongPtr(hWnd, 0);
    if(f != NULL) {
      f->running = 0;
    }
    PostQuitMessage(0);
    return 0;
  } else {
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  
}

FRAME_DEF Frame_Error frame_open(Frame *f, s32 width, s32 height, s32 flags) {
  
  HMODULE instance = GetModuleHandle(NULL);
  STARTUPINFO startup_info;
  GetStartupInfo(&startup_info);

  SetProcessDPIAware();

  WNDCLASSEX wnd_class = {0};
  wnd_class.cbSize = sizeof(WNDCLASSEX);
  wnd_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  wnd_class.lpfnWndProc = frame_window_proc;
  wnd_class.hInstance = instance;
  wnd_class.lpszClassName = " ";
  wnd_class.cbWndExtra = sizeof(LONG_PTR);
  wnd_class.hCursor = LoadCursor(NULL, IDC_ARROW);
  HICON icon = LoadIcon(instance, MAKEINTRESOURCE(1));
  wnd_class.hIcon = icon; // ICON when tabbing
  wnd_class.hIconSm = icon; //ICON default
  if(!RegisterClassExA(&wnd_class)) {
    return frame_error_last();
  }

#define FRAME_STYLE (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)
#define FRAME_STYLE_RESIZABLE (WS_THICKFRAME | WS_MAXIMIZEBOX)
  DWORD style = FRAME_STYLE;
  if(!(flags & FRAME_NOT_RESIZABLE)) {
    style |= FRAME_STYLE_RESIZABLE;
  } else {
    /* width -= 10; */
    /* height -= 10; */
  }

  s32 screen_width = GetSystemMetrics(SM_CXSCREEN);
  s32 screen_height = GetSystemMetrics(SM_CYSCREEN);

  width += 16;
  height += 39;

  f->hwnd = CreateWindowExA(WS_EX_ACCEPTFILES * (!!(flags & FRAME_DRAG_N_DROP)),
			    wnd_class.lpszClassName,
			    wnd_class.lpszClassName,
			    style,
			    screen_width / 2 - width / 2,
			    screen_height / 2 - height / 2,
			    width,
			    height,
			    NULL,
			    NULL,
			    instance,
			    NULL);
  width -= 16;
  height -= 39;
  if(!f->hwnd) {
    return frame_error_last();
  }
  f->hdc = GetDC(f->hwnd);

  //BEGIN opengl
  HDC w_dc = GetDC(f->hwnd);

  PIXELFORMATDESCRIPTOR desired_format = {0};
  desired_format.nSize = sizeof(desired_format);
  desired_format.nVersion = 1;
  desired_format.dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
  desired_format.cColorBits = 32;
  desired_format.cAlphaBits = 8;

  int suggested_format_index = ChoosePixelFormat(w_dc, &desired_format);
  PIXELFORMATDESCRIPTOR suggested_format;
  DescribePixelFormat(w_dc, suggested_format_index, sizeof(suggested_format), &suggested_format);
  SetPixelFormat(w_dc, suggested_format_index, &suggested_format);
  
  HGLRC opengl_rc = wglCreateContext(w_dc);
  if(!wglMakeCurrent(w_dc, opengl_rc)) {
    return frame_error_last();
  }
  ReleaseDC(f->hwnd, w_dc);

  //END opengl

  LONG_PTR lptr = {0};
  memcpy(&lptr, &f, sizeof(f));
  SetWindowLongPtr(f->hwnd, 0, lptr);
	
  ShowWindow(f->hwnd, startup_info.wShowWindow);
  UpdateWindow(f->hwnd);

#define FRAME_OPENGL_FUNC(return_type, func_name, ...)			\
  do{									\
    __##func_name = (func_name##_t) (void *) wglGetProcAddress( #func_name ) ; \
    if(! __##func_name ) {						\
      return FRAME_ERROR_CANNOT_LOAD_OPENGL_FUNC;			\
    }									\
  }while(0);
  FRAME_OPENGL_FUNCS
#undef FRAME_OPENGL_FUNC

    f->mouse_visible = 1;
  f->width = (f32) width;
  f->height = (f32) height;
  f->running = FRAME_RUNNING;

  
  if((flags & FRAME_FULLSCREEN)) {
    Frame_Error error = frame_toggle_fullscreen(f);
    if(error != FRAME_ERROR_NONE) {
      return error;
    }    
  }

  wglSwapIntervalEXT(1);

  return FRAME_ERROR_NONE;
}

FRAME_DEF int frame_peek(Frame *f, Frame_Event *e) {

  while(1) {
    if(!PeekMessageA(&e->msg, f->hwnd, 0, 0, PM_REMOVE)) {
      break;
    }

    e->type = FRAME_EVENT_NONE;

    TranslateMessage(&e->msg);
    DispatchMessage(&e->msg);

    switch(e->msg.message) {
    case WM_DROPFILES: {
      e->type = FRAME_EVENT_FILEDROP;
      e->as.value = e->msg.wParam;
    } break;
    case WM_RBUTTONUP:  {
      e->type = FRAME_EVENT_MOUSERELEASE;
      e->as.key = 'r';
      ReleaseCapture();
    } break;
    case WM_RBUTTONDOWN: {
      e->type = FRAME_EVENT_MOUSEPRESS;
      e->as.key = 'r';
      SetCapture(f->hwnd);
    } break;
    case WM_LBUTTONUP: {
      e->type = FRAME_EVENT_MOUSERELEASE;
      e->as.key = 'l';
      ReleaseCapture();
    } break;
    case WM_LBUTTONDOWN: {
      e->type = FRAME_EVENT_MOUSEPRESS;
      e->as.key = 'l';
      SetCapture(f->hwnd);
    } break;
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP: {
      int was_down = ((e->msg.lParam & (1 << 30)) != 0);
      int is_down = ((e->msg.lParam & (1 << 31)) == 0);

      e->as.key = (char) e->msg.wParam;
      if(was_down) e->type = FRAME_EVENT_KEYRELEASE;
      if(is_down) e->type = FRAME_EVENT_KEYPRESS;
    } break;
    case WM_MOUSEWHEEL: {
      e->type = FRAME_EVENT_MOUSEWHEEL;
      e->as.amount = GET_WHEEL_DELTA_WPARAM(e->msg.wParam) / 120;
    } break;
    }

    if(e->type != FRAME_EVENT_NONE) {
      return 1;
    }    
  }
  
  if(!(f->running & FRAME_FULLSCREEN) &&
     GetClientRect(f->hwnd, &f->rect)) {
    f->width = (f32) (f->rect.right - f->rect.left);
    f->height = (f32) (f->rect.bottom - f->rect.top);
  }

  if(!GetCursorPos(&f->point) ||
     !ScreenToClient(f->hwnd, &f->point)) {
    return 0;
  }

  f->mouse_x = (float) f->point.x;
  f->mouse_y = (float) f->height - f->point.y;
  
  return 0;
}

FRAME_DEF void frame_swap_buffers(Frame *f) {
  SwapBuffers(f->hdc);
}

FRAME_DEF Frame_Error frame_toggle_fullscreen(Frame *f) {
  
  DWORD style = (DWORD) GetWindowLongPtr(f->hwnd, GWL_STYLE);

  if(f->running & FRAME_FULLSCREEN) {

    style &= ~WS_POPUP;
    style |= FRAME_STYLE; // fix this
    if(!(f->running & FRAME_NOT_RESIZABLE)) {
      style |= FRAME_STYLE_RESIZABLE;
    }
    f->running &= ~FRAME_FULLSCREEN;
    
    SetWindowPos(f->hwnd, NULL,
		 f->rect.left,
		 f->rect.top,
		 f->rect.right - f->rect.left,
		 f->rect.bottom - f->rect.top,
		 SWP_FRAMECHANGED);
  } else {    
    if(!GetWindowRect(f->hwnd, &f->rect)) {
      return frame_error_last();
    }

    s32 width = GetSystemMetrics(SM_CXSCREEN);
    s32 height = GetSystemMetrics(SM_CYSCREEN);

    style &= ~FRAME_STYLE;
    if(!(f->running & FRAME_NOT_RESIZABLE)) {
      style &= ~FRAME_STYLE_RESIZABLE;
    }
    style |= WS_POPUP;
    f->running |= FRAME_FULLSCREEN;
    f->width = (f32) width;
    f->height = (f32) height;

    SetWindowPos(f->hwnd, HWND_TOP, 0, 0, width, height, SWP_FRAMECHANGED);
  }

  SetWindowLongPtr(f->hwnd, GWL_STYLE, style);

  return FRAME_ERROR_NONE;
  
}

FRAME_DEF void frame_close(Frame *f) {
  DestroyWindow(f->hwnd);
}

FRAME_DEF int frame_dragged_files_open(Frame_Dragged_Files *f, Frame_Event *event) {
  f->h_drop = (HDROP) event->as.value;
  f->count = DragQueryFileW(f->h_drop, 0xffffffff, NULL, 0);
  if(f->count <= 0) {
    return 0;
  }
  f->index = 0;
  return 1;
}

FRAME_DEF int frame_dragged_files_next(Frame_Dragged_Files *f) {
  if(f->index >= f->count) {
    return 0;
  }
  DragQueryFileW(f->h_drop, f->index++, f->wpath, MAX_PATH);
  WideCharToMultiByte(CP_UTF8, 0, f->wpath, -1, f->path, MAX_PATH, NULL, NULL);
  return 1;
}

FRAME_DEF void frame_dragged_files_close(Frame_Dragged_Files *f) {
  DragFinish(f->h_drop);
}

#else // linux

FRAME_DEF Frame_Error frame_error_last() {
  
  switch(errno) {
  case 0:
    return FRAME_ERROR_NONE;
  default:
    fprintf(stderr, "FRAME_ERROR: Unhandled last_error: %d\n", errno); fflush(stderr);
    exit(1);
  }

}

FRAME_DEF Frame_Error frame_open(Frame *f, s32 width, s32 height, s32 flags) {

  // TODO: handle flags
  (void) flags;

  f->display = XOpenDisplay(NULL);
  if(!f->display) {
    return frame_error_last();
  }

  static int visual_attribs[] = {
    GLX_X_RENDERABLE    , True,
    GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
    GLX_RENDER_TYPE     , GLX_RGBA_BIT,
    GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
    GLX_RED_SIZE        , 8,
    GLX_GREEN_SIZE      , 8,
    GLX_BLUE_SIZE       , 8,
    GLX_ALPHA_SIZE      , 8,
    GLX_DEPTH_SIZE      , 24,
    GLX_STENCIL_SIZE    , 8,
    GLX_DOUBLEBUFFER    , True,
    //GLX_SAMPLE_BUFFERS  , 1,
    //GLX_SAMPLES         , 4,
    None
  };

  s32 fbcount;
  GLXFBConfig *fbc = glXChooseFBConfig(f->display,
				       DefaultScreen(f->display),
				       visual_attribs,
				       &fbcount);
  if(!fbc) {
    return frame_error_last();
  }

  s32 best_fbc_index = -1;
  s32 best_num_samples = -1;
  for(s32 i=0;i<fbcount;i++) {
    XVisualInfo *visual_info = glXGetVisualFromFBConfig(f->display, fbc[i]);
    if(!visual_info) {
      continue;
    }

    s32 samp_buf, samples;
    glXGetFBConfigAttrib(f->display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
    glXGetFBConfigAttrib(f->display, fbc[i], GLX_SAMPLES, &samples);

    if(best_fbc_index < 0 || (samp_buf && samples > best_num_samples)) {
      best_fbc_index = i;
      best_num_samples = samples;
    }
    
    XFree(visual_info);
  }

  GLXFBConfig best_fbc;
  XVisualInfo *visual_info;
  if(best_fbc_index >= 0) {
    best_fbc = fbc[best_fbc_index];
    visual_info = glXGetVisualFromFBConfig(f->display, best_fbc);
  } else {
    visual_info = NULL;
  }
  XFree(fbc);

  if(!visual_info) {
    return frame_error_last();
  }

  Window root_window = RootWindow(f->display, visual_info->screen);

  f->colormap = XCreateColormap(f->display,
				root_window,
				visual_info->visual,
				AllocNone);
  
  XSetWindowAttributes set_window_attributes;
  set_window_attributes.colormap          = f->colormap;
  set_window_attributes.background_pixmap = None;
  set_window_attributes.border_pixel      = 0;
  set_window_attributes.event_mask        = StructureNotifyMask;

#define FRAME_OPENGL_FUNC(return_type, func_name, ...)			\
  do{									\
    __##func_name = (func_name##_t) glXGetProcAddressARB( (unsigned char * ) #func_name ) ; \
    if(! __##func_name ) {						\
      return FRAME_ERROR_CANNOT_LOAD_OPENGL_FUNC;			\
    }									\
  }while(0);
  FRAME_OPENGL_FUNCS
#undef FRAME_OPENGL_FUNC

    f->window = XCreateWindow(f->display,
			      root_window,
			      0,
			      0,
			      width,
			      height,
			      0,
			      visual_info->depth,
			      InputOutput,
			      visual_info->visual,
			      CWBorderPixel | CWColormap | CWEventMask,
			      &set_window_attributes);
  XFree(visual_info);
  if(!f->window) {
    return frame_error_last();
  }

  Atom atom_delete_window = XInternAtom(f->display, "WM_DELETE_WINDOW", False);
  f->atom_delete_window = (s32) atom_delete_window;
  XSetWMProtocols(f->display,
		  f->window,
		  &atom_delete_window,
		  1);

  char *title = " ";
  
  XStoreName(f->display,
	     f->window,
	     title);
  XSelectInput(f->display,
	       f->window,
	       ButtonPressMask   |
	       ButtonReleaseMask |
	       ExposureMask      |
	       PointerMotionMask |
	       EnterWindowMask   |
	       LeaveWindowMask   |
	       PointerMotionMask |
	       KeyPressMask      |
	       KeyReleaseMask    |
	       StructureNotifyMask);
  XMapWindow(f->display, f->window);

  f->context = glXCreateNewContext(f->display,
				   best_fbc,
				   GLX_RGBA_TYPE,
				   0,
				   True);
  XSync(f->display, False);
  glXMakeCurrent(f->display, f->window, f->context);

  f->running = FRAME_RUNNING;
  f->fd = -1;
  
  return FRAME_ERROR_NONE;
}

FRAME_DEF int frame_peek(Frame *f, Frame_Event *e) {

  if(!f->running) {
    return 0;
  }

  if(f->fd < 0) {
    f->fd = ConnectionNumber(f->display);
  }

  while(1) {
    if(!XPending(f->display)) {
      break;
    }

    XNextEvent(f->display, &e->x_event);
    
    e->type = FRAME_EVENT_NONE;

    switch(e->x_event.type) {
    case ClientMessage: {
      if(e->x_event.xclient.data.l[0] == f->atom_delete_window) {
	f->running = 0;
      }
    } break;
    case ButtonRelease:
    case ButtonPress: {
      Frame_Event_Type type;
      if(e->x_event.type == ButtonRelease) {
	type = FRAME_EVENT_MOUSERELEASE;
      } else {
	type = FRAME_EVENT_MOUSEPRESS;
      }
      e->type = type;

      if(e->x_event.xbutton.button == Button1) {
	e->as.key = 'L';
      } else if(e->x_event.xbutton.button == Button2) {
        e->as.key = 'M';
      } else if(e->x_event.xbutton.button == Button3) {
        e->as.key = 'R';
      }

      if(e->x_event.xbutton.button == Button4) { //TO top
        e->type = FRAME_EVENT_MOUSEWHEEL;
        e->as.amount = 1;
      } else if(e->x_event.xbutton.button == Button5) { //TO bottom
        e->type = FRAME_EVENT_MOUSEWHEEL;
        e->as.amount = -1;
      }      
      
    } break;
    case KeyRelease:
    case KeyPress: {
      Frame_Event_Type type;
      if(e->x_event.type == ButtonRelease) {
	type = FRAME_EVENT_KEYRELEASE;
      } else {
	type = FRAME_EVENT_KEYPRESS;
      }
      e->type = type;
      
      e->as.key = XkbKeycodeToKeysym(f->display, e->x_event.xkey.keycode, 0, 1);
    } break;
    }

    if(e->type != FRAME_EVENT_NONE) {
      return 1;
    }    
  }

  Window root;
  s32 x, y;
  u32 w, h;
  u32 bw, depth;
  XGetGeometry(f->display,
	       f->window,
	       &root,
	       &x, &y, &w, &h, &bw, &depth);
  f->width = (f32) w;
  f->height = (f32) h;

  s32 _w, _h;
  XQueryPointer(f->display,
                f->window,
                &root,
                &root,
                &_w,
                &_h,
                &x,
                &y,
                &depth);
  f->mouse_x = (f32) x;
  f->mouse_y = (f32) (f->height - y);
    
  return 0;
}

FRAME_DEF void frame_swap_buffers(Frame *f) {
  glXSwapBuffers(f->display, f->window);
}

FRAME_DEF Frame_Error frame_toggle_fullscreen(Frame *f) {
  // TODO
  (void) f;
  return FRAME_ERROR_NONE;
}

FRAME_DEF void frame_set_mouse_visible(Frame *f, int visible) {
  // TODO
  (void) f;
  (void) visible;
}

FRAME_DEF Frame_Error frame_set_title(Frame *f, char *title) {
  // TODO
  (void) f;
  (void) title;
  return FRAME_ERROR_NONE;
}

FRAME_DEF Frame_Error frame_set_icon(Frame *f, u8 *pixels, u64 pixels_width, u64 pixels_height) {
  // TODO
  (void) f;
  (void) pixels;
  (void) pixels_width;
  (void) pixels_height;
  return FRAME_ERROR_NONE;
}

FRAME_DEF void frame_close(Frame *f) {
  glXMakeCurrent(f->display, 0, 0);
  glXDestroyContext(f->display, f->context);
  XDestroyWindow(f->display, f->window);
  XFreeColormap(f->display, f->colormap);
  XCloseDisplay(f->display);
}

FRAME_DEF int frame_dragged_files_open(Frame_Dragged_Files *files, Frame_Event *event) {
  // TODO
  (void) files;
  (void) event;
  return 0;
}

FRAME_DEF int frame_dragged_files_next(Frame_Dragged_Files *files) {
  // TODO
  (void) files;
  return 0;
}

FRAME_DEF void frame_dragged_files_close(Frame_Dragged_Files *files) {
  // TODO
  (void) files;
}

#endif // _WIN32

#endif // FRAME_IMPLEMENTATION

#undef u8
#undef s32
#undef u32
#undef f32
#undef s64
#undef u64

#endif // FRAME_H
