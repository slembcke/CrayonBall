#include <SDL2/SDL.h>

#ifdef __APPLE__
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
  #include <OpenGL/OpenGL.h>

  #include <OpenAL/al.h>
  #include <OpenAL/alc.h>
#else
  #include <GL/gl.h>
  #include <GL/glext.h>
  #include <GL/glu.h>

  #include <AL/al.h>
  #include <AL/alc.h>
#endif

#include <ruby.h>

#include "AS.h"

static int isRegistered = 1;
//extern int isRegistered;
VALUE registered;

VALUE rbas_mAS;

static SDL_Window* WINDOW;

int AS_SCREEN_W = -1;
int AS_SCREEN_H = -1;

int AS_NATIVE_W = 0;
int AS_NATIVE_H = 0;

VALUE rbas_swapBuffers(VALUE self)
{
  SDL_GL_SwapWindow(WINDOW);
  return Qnil;
}

VALUE rbas_show_mouse(VALUE self, VALUE show)
{
  SDL_ShowCursor(show ? SDL_ENABLE : SDL_DISABLE);
  return Qnil;
}

VALUE rbas_grab_mouse(VALUE self, VALUE grab)
{
  SDL_SetWindowGrab(WINDOW, grab);
  return Qnil;
}

VALUE rbas_screen_w(VALUE self)
{
  return INT2FIX(AS_SCREEN_W);
}

VALUE rbas_screen_h(VALUE self)
{
  return INT2FIX(AS_SCREEN_H);
}

VALUE rbas_native_w(VALUE self)
{
  return INT2FIX(AS_NATIVE_W);
}

VALUE rbas_native_h(VALUE self)
{
  return INT2FIX(AS_NATIVE_H);
}

static void print_video_info(void)
{
  int r; SDL_GL_GetAttribute(SDL_GL_RED_SIZE,   &r);
  int g; SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &g);
  int b; SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE,  &b);
  int a; SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &a);

  int depth; SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depth);
  int stncl; SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &stncl);
  int dbuff; SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &dbuff);
  int mltsm; SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &mltsm);
  int vbl = SDL_GL_GetSwapInterval();

  int w, h; SDL_GetWindowSizeInPixels(WINDOW, &w, &h);

  printf("Video info:\n");
  printf("\t%dx%d resolution.\n", w, h);
  printf("\tR%d G%d B%d A%d color buffer.\n", r, g, b, a);
  printf("\t%d bit depth buffer.\n", depth);
  printf("\t%d bit stencil buffer.\n", stncl);
  printf("\t%s buffered.\n", dbuff ? "Double" : "Single");
  printf("\t%d multisamples.\n", mltsm);
  printf("\tVBL sync %s.\n", vbl ? "on" : "off");
}

//VALUE rbas_open_screen(VALUE self, VALUE width, VALUE height, VALUE hsh)//(int argc, VALUE *argv, VALUE self)
VALUE rbas_open_screen(int argc, VALUE *argv, VALUE self)
{
  if(WINDOW){
    SDL_GL_DeleteContext(SDL_GL_GetCurrentContext());
    SDL_DestroyWindow(WINDOW);
  }
  
  VALUE width, height, hsh;
  rb_scan_args(argc, argv, "21", &width, &height, &hsh);
	
  if(NIL_P(hsh)) hsh = rb_hash_new();
  Check_Type(hsh, T_HASH);
	
  VALUE depth = rb_hash_aref(hsh, ID2SYM(rb_intern("depth")));
  if(RTEST(depth))
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, NUM2INT(depth));
  else
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
	
  VALUE stencil = rb_hash_aref(hsh, ID2SYM(rb_intern("stencil")));
  if(RTEST(stencil))
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, NUM2INT(stencil));
  else
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);

  VALUE vbl = rb_hash_aref(hsh, ID2SYM(rb_intern("vbl_sync")));
  SDL_GL_SetSwapInterval(RTEST(vbl));

  Uint32 videoflags = SDL_WINDOW_OPENGL;
  VALUE fullscreen = rb_hash_aref(hsh, ID2SYM(rb_intern("fullscreen")));
  if(RTEST(fullscreen)) videoflags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
  
  VALUE name = rb_hash_aref(hsh, ID2SYM(rb_intern("name")));
  const char* title = name != Qnil ? StringValuePtr(name) :  "Aerosol";
  int w = NUM2INT(width), h = NUM2INT(height);
  WINDOW = SDL_CreateWindow(title, 0, 0, w, h, videoflags);
  
  SDL_GLContext* context = SDL_GL_CreateContext(WINDOW);
  SDL_GL_MakeCurrent(WINDOW, context);
  
  print_video_info();
  SDL_GetWindowSizeInPixels(WINDOW, &AS_SCREEN_W, &AS_SCREEN_H);
	
  glMatrixMode(GL_PROJECTION); {
    glLoadIdentity();
    gluOrtho2D(0, AS_SCREEN_W, 0, AS_SCREEN_H);
  } glMatrixMode(GL_MODELVIEW);
		
  return Qnil;
}

//Referenced from AS_init.c
extern ALCcontext *AL_context;
extern ALCdevice *AL_device;

VALUE rbas_exit(VALUE self)
{
  //	printf("\n\tCalling SDL_Quit ... ");
  SDL_Quit();
  //	printf("Successful.\n");
  //	printf("Successful.\nGetting ALC context and device ... ");
  ALCcontext *context = alcGetCurrentContext();
  if(!context) printf("Null context\n");
  ALCdevice *device = alcGetContextsDevice(context);
  if(!device) printf("Null device\n");
  //	printf("Successful.\nStopping context ... ");
  //	printf("\tStopping context ... ");
  if(alcMakeContextCurrent(NULL) != AL_TRUE) printf("alcMakeContextCurrent() returned false\n");
  //	printf("Successful.\n\tDestroying context ... ");
  alcDestroyContext(AL_context);
  //	printf("Successful\n\tClosing device ... ");
  alcCloseDevice(AL_device);
  //	printf("Successful.\n");
	
  return Qnil;
}

static VALUE
rbas_set_gamma(VALUE self, VALUE red, VALUE green, VALUE blue)
{
  abort();
}

/**
 * Initialize the ruby code parts of aerosol. Does not include the
 * library functions that aerosol uses- they are initialized inside the main
 * function. This function is intended to be called from within an rb_protect
 * as to provide some error handling if something goes wrong in the files
 * rb_require'd inside this code.
 *
 * At the end of this method, the game code is run inside as_start.rb, which
 * has additional error handling specifically for the game code.
 */
VALUE Start_Aerosol(VALUE self)
{
  rb_define_module_function(rbas_mAS, "swap_buffers", rbas_swapBuffers, 0);
	
  rb_define_module_function(rbas_mAS, "show_mouse=", rbas_show_mouse, 1);
  rb_define_module_function(rbas_mAS, "grab_mouse=", rbas_grab_mouse, 1);
	
  rb_define_module_function(rbas_mAS, "open_screen", rbas_open_screen, -1);
  rb_define_singleton_method(rbas_mAS, "screen_w", rbas_screen_w, 0);
  rb_define_singleton_method(rbas_mAS, "screen_h", rbas_screen_h, 0);
  rb_define_singleton_method(rbas_mAS, "native_w", rbas_native_w, 0);
  rb_define_singleton_method(rbas_mAS, "native_h", rbas_native_h, 0);
  rb_define_module_function(rbas_mAS, "set_gamma", rbas_set_gamma, 3);
	
  rb_define_module_function(rbas_mAS, "exit", rbas_exit, 0);

	registered = isRegistered ? Qtrue : Qfalse;
	rb_define_readonly_variable("registered", &registered);
	
  // This ruby code loads all of the ruby code for aerosol.
  rb_funcall(rb_cObject, rb_intern("require"), 1, rb_str_new2("src/load.rb"));
//  rb_require("ruby/load.rb");
  // Register c functions for the different modules
  Init_ASmusic();
  Init_ASgl();
  Init_ASimage();
	Init_ASevents();
	
  printf("Aerosol loaded, starting game code.\n");
  rb_funcall(rb_cObject, rb_intern("require"), 1, rb_str_new2("ruby/execute.rb"));
//  rb_require("ruby/execute.rb");
	
  return Qnil;
}
