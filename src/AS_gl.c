#ifdef __APPLE__
	#include "OpenGL/gl.h"
	#include "OpenGL/glu.h"
#else
	#include <GL/gl.h>
	#include <GL/glext.h>
	#include <GL/glu.h>
#endif

#ifndef GL_TEXTURE_RECTANGLE
	#define GL_TEXTURE_RECTANGLE_EXT 0x84F5
#endif

#include <ruby.h>

#include "AS.h"

static VALUE
rbas_clear_screen(VALUE self)
{
	glClear(GL_COLOR_BUFFER_BIT);
	return Qnil;
}

static VALUE
rbas_clear_color(VALUE self, VALUE r, VALUE g, VALUE b, VALUE a)
{
	glClearColor(NUM2DBL(r), NUM2DBL(g), NUM2DBL(b), NUM2DBL(a));
	return Qnil;
}

void Init_ASgl(void){
	rb_define_module_function(rbas_mAS, "clear_screen", rbas_clear_screen, 0);
	rb_define_module_function(rbas_mAS, "clear_color", rbas_clear_color, 4);
}
