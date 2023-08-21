/*
 * $Date: 2004/01/27 05:31:35 $
 * $Author: keith $
 */
#if defined (WIN32)
# include "windows.h"
#endif

#if defined(__APPLE_CC__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include "rbogl.h"

void InitializeGL(void);
void InitializeGLU(void);

#ifdef _NO_NUM2DBL_
extern double
num2double(val)
    VALUE val;
{
    struct RFloat* flt;
    if (NIL_P(val)) return 0;
    flt = RFLOAT(f_float(0, val));
    return flt->value;
}
#endif

extern int
ary2cint(arg, cary, maxlen)
VALUE arg;
int cary[];
int maxlen;
{
    int i;
    VALUE ary = (rb_Array(arg));
    if (maxlen < 1)
	maxlen = RARRAY_LEN(ary);
    else
	maxlen = maxlen < RARRAY_LEN(ary) ? maxlen : RARRAY_LEN(ary);
    for (i=0; i < maxlen; i++)
	cary[i] = NUM2INT(rb_ary_entry((VALUE)ary,i));
    return i;
}

extern int
ary2cflt(arg, cary, maxlen)
VALUE arg;
float cary[];
int maxlen;
{
    int i;
    VALUE ary = (rb_Array(arg));
    if (maxlen < 1)
	maxlen = RARRAY_LEN(ary);
    else
	maxlen = maxlen < RARRAY_LEN(ary) ? maxlen : RARRAY_LEN(ary);
    for (i=0; i < maxlen; i++)
	cary[i] = (float)NUM2DBL(rb_ary_entry((VALUE)ary,i));
    return i;
}

extern int
ary2cdbl(arg, cary, maxlen)
VALUE arg;
double cary[];
int maxlen;
{
    int i;
    VALUE ary = (rb_Array(arg));
    if (maxlen < 1)
	maxlen = RARRAY_LEN(ary);
    else
	maxlen = maxlen < RARRAY_LEN(ary) ? maxlen : RARRAY_LEN(ary);
    for (i=0; i < maxlen; i++)
	cary[i] = NUM2DBL(rb_ary_entry((VALUE)ary,i));
    return i;
}

extern void
mary2ary(src, ary)
VALUE src;
VALUE ary;
{
    int i;
    VALUE tmp_ary = (rb_Array(src));
    for (i = 0; i < RARRAY_LEN(tmp_ary); i++) {
	if (TYPE(RARRAY_PTR(tmp_ary)[i]) == T_ARRAY)
	    mary2ary((VALUE)tmp_ary, ary);
        else
	    rb_ary_push(ary, RARRAY_PTR(tmp_ary)[i]);
    }
}

extern void
ary2cmat4x4(ary, cary)
VALUE ary;
double cary[];
{
    int i,j;
    memset(cary, 0x0, sizeof(double[4*4]));
    VALUE ary_c = (rb_Array(ary));
    if (TYPE(RARRAY_PTR(ary_c)[0]) != T_ARRAY)
	ary2cdbl((VALUE)ary_c, cary, 16);
    else {
	for (i = 0; i < RARRAY_LEN(ary_c) && i < 4; i++) {
	    VALUE ary_r = (rb_Array(RARRAY_PTR(ary_c)[i]));
	    for(j = 0; j < RARRAY_LEN(ary_r) && j < 4; j++)
		cary[i*4+j] = (GLdouble)NUM2DBL(RARRAY_PTR(ary_r)[j]);
	}
    }
}

/*Need to find proper size for glReadPixels array*/
int glformat_size(GLenum format) {
	switch(format) {
	case GL_COLOR_INDEX:
	case GL_RED:
	case GL_GREEN:
	case GL_BLUE:
	case GL_ALPHA:
	case GL_STENCIL_INDEX:
	case GL_DEPTH_COMPONENT:
	case GL_LUMINANCE:
		return 1;

	case GL_LUMINANCE_ALPHA:
		return 2;

	case GL_RGB:
#ifdef GL_BGR_EXT
	case GL_BGR_EXT:
#endif
		return 3;

	case GL_RGBA:
#ifdef GL_BGRA_EXT
	case GL_BGRA_EXT:
#endif
#ifdef GL_ABGR_EXT
	case GL_ABGR_EXT:
#endif
		return 4;
        case 1:
        case 2:
        case 3:
        case 4:
                return format;
	default:
		return -1;
	}
}

int gltype_size(GLenum type) {
	switch(type) {
	case GL_BYTE:
	case GL_UNSIGNED_BYTE:
		return 8;

	case GL_SHORT:
	case GL_UNSIGNED_SHORT:
		return 16;

	case GL_INT:
	case GL_UNSIGNED_INT:
	case GL_FLOAT:
		return 32;

	case GL_BITMAP:
		return 1;
	
	default:
		return -1;
	}
}

VALUE
allocate_buffer_with_string(size)
int size;
{
//    NEWOBJ(buffer, struct RString);
//    OBJSETUP(buffer, rb_cString, T_STRING);
//    buffer->ptr = (char*)xmalloc(sizeof(char)*(size+1));
//    buffer->len = size;
//    buffer->ptr[buffer->len] = '\0';
//    return (VALUE)buffer;
	volatile VALUE data = rb_str_new(NULL, 0);
	rb_str_resize(data, size);
	return data;
}

void
Init_opengl()
{
    InitializeGL();
    InitializeGLU();
}
