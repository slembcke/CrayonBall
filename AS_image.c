#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <OpenGL/OpenGL.h>
#else
	#include <GL/gl.h>
	#include <GL/glext.h>
	#include <GL/glu.h>
#endif

#include "ruby.h"

#include "png.h"
//#include "jpeglib.h"

#include "AS.h"

VALUE rbas_image;

#define PNG_SIG_BYTES 8

static void
premultiply(png_structp png_ptr, png_row_infop info, png_bytep data)
{
	int width = info->width;
	int i;
	
	if(info->channels == 4){
		for(i=0; i<width; i++){
			float alpha = data[i*4 + 3]/255.0;
			data[i*4 + 0] *= alpha;
			data[i*4 + 1] *= alpha;
			data[i*4 + 2] *= alpha;
		}
	} else {
		for(i=0; i<width; i++){
			float alpha = data[i*2 + 1]/255.0;
			data[i*4] *= alpha;
		}
	}
}

static VALUE
load_png(char *name, int rgb, int alpha, int premult, int *width, int *height)
{
	FILE *png_file = fopen(name, "rb");
	if(!png_file)
		rb_raise(rb_eRuntimeError, "\"%s\" can't be opened.", name);
	
	unsigned char header[PNG_SIG_BYTES];
	
	fread(header, 1, PNG_SIG_BYTES, png_file);
	if(png_sig_cmp(header, 0, PNG_SIG_BYTES))
		rb_raise(rb_eRuntimeError, "PNG Error: \"%s\" is not a PNG file.", name);
	
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr)
		rb_raise(rb_eRuntimeError, "LibPNG error when loading \"%s\".", name);
	
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr)
		rb_raise(rb_eRuntimeError, "LibPNG error when loading \"%s\".", name);
	
	png_infop end_info = png_create_info_struct(png_ptr);
	if(!end_info)
		rb_raise(rb_eRuntimeError, "LibPNG error when loading \"%s\".", name);
	
	if(setjmp(png_jmpbuf(png_ptr)))
		rb_raise(rb_eRuntimeError, "LibPNG error when loading \"%s\".", name);
	
	png_init_io(png_ptr, png_file);
	png_set_sig_bytes(png_ptr, PNG_SIG_BYTES);
	png_read_info(png_ptr, info_ptr);
	
	*width = png_get_image_width(png_ptr, info_ptr);
	*height = png_get_image_height(png_ptr, info_ptr);
	
	png_uint_32 bit_depth, color_type;
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
	
	// if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
	// 	png_set_gray_1_2_4_to_8(png_ptr);
	
	if (bit_depth == 16)
		png_set_strip_16(png_ptr);
	
	if(rgb)
	{
		if(color_type == PNG_COLOR_TYPE_PALETTE)
			png_set_palette_to_rgb(png_ptr);
		else if(color_type == PNG_COLOR_TYPE_GRAY ||
				color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		{
			png_set_gray_to_rgb(png_ptr);
		}
	}
	else
	{
		if(color_type == PNG_COLOR_TYPE_PALETTE)
			rb_raise(rb_eRuntimeError, "(%s): paletted PNG to grayscale not supported\n", name);
		else if(color_type == PNG_COLOR_TYPE_RGB ||
				color_type == PNG_COLOR_TYPE_RGB_ALPHA)
		{
			png_set_rgb_to_gray_fixed(png_ptr, 1, -1, -1);
		}
	}
	
	if (alpha)
	{
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
			png_set_tRNS_to_alpha(png_ptr);
		else
			png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	}
	else
	{
		if (color_type & PNG_COLOR_MASK_ALPHA)
			png_set_strip_alpha(png_ptr);
	}
	
	if(premult)
		png_set_read_user_transform_fn(png_ptr, &premultiply);
    
	png_read_update_info(png_ptr, info_ptr);
	
	png_uint_32 rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	png_uint_32 numbytes = rowbytes*(*height);
	
	volatile VALUE data = rb_str_new(NULL, 0);
	rb_str_resize(data, numbytes);
	png_byte *pixels = (png_byte *)StringValuePtr(data);
	png_byte **row_ptrs = malloc((*height) * sizeof(png_byte*));
	
	int i;
	for (i=0; i<(*height); i++)
		row_ptrs[i] = pixels + ((*height) - 1 - i)*rowbytes;
	
	png_read_image(png_ptr, row_ptrs);
	
	free(row_ptrs);
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	fclose(png_file);
	
	return data;
}

static VALUE
rbas_loadPNG_RGBA(VALUE self, VALUE filename)
{
	int w, h;
	volatile VALUE data = load_png(StringValuePtr(filename), 1, 1, 0, &w, &h);
	
	return rb_struct_new(rbas_image, data, INT2FIX(w), INT2FIX(h), INT2NUM(GL_RGBA));
}

static VALUE
rbas_loadPNG_RGBA_pre(VALUE self, VALUE filename)
{
	int w, h;
	volatile VALUE data = load_png(StringValuePtr(filename), 1, 1, 1, &w, &h);
	
	return rb_struct_new(rbas_image, data, INT2FIX(w), INT2FIX(h), INT2NUM(GL_RGBA));
}

static VALUE
rbas_loadPNG_RGB(VALUE self, VALUE filename)
{
	int w, h;
	volatile VALUE data = load_png(StringValuePtr(filename), 1, 0, 0, &w, &h);
	
	return rb_struct_new(rbas_image, data, INT2FIX(w), INT2FIX(h), INT2NUM(GL_RGB));
}

static VALUE
rbas_loadPNG_LUM(VALUE self, VALUE filename)
{
	int w, h;
	volatile VALUE data = load_png(StringValuePtr(filename), 0, 0, 0, &w, &h);
	
	return rb_struct_new(rbas_image, data, INT2FIX(w), INT2FIX(h), INT2NUM(GL_LUMINANCE));
}

static VALUE
rbas_loadPNG_LUM_AS_ALPHA(VALUE self, VALUE filename)
{
	int w, h;
	volatile VALUE data = load_png(StringValuePtr(filename), 0, 0, 0, &w, &h);
	
	return rb_struct_new(rbas_image, data, INT2FIX(w), INT2FIX(h), INT2NUM(GL_ALPHA));
}

static VALUE
rbas_loadPNG_LUMA(VALUE self, VALUE filename)
{
	int w, h;
	volatile VALUE data = load_png(StringValuePtr(filename), 0, 1, 0, &w, &h);
	
	return rb_struct_new(rbas_image, data, INT2FIX(w), INT2FIX(h), INT2NUM(GL_LUMINANCE_ALPHA));
}

static VALUE
rbas_loadPNG_LUMA_pre(VALUE self, VALUE filename)
{
	int w, h;
	volatile VALUE data = load_png(StringValuePtr(filename), 0, 1, 1, &w, &h);
	
	return rb_struct_new(rbas_image, data, INT2FIX(w), INT2FIX(h), INT2NUM(GL_LUMINANCE_ALPHA));
}
/*
struct my_error_mgr {
	struct jpeg_error_mgr pub;
	
	jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr *my_error_ptr;

static void
my_error_exit (j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
//	(*cinfo->err->output_message) (cinfo);
	
	longjmp(myerr->setjmp_buffer, 1);
}

static VALUE
load_jpeg(char *filename, int *width, int *height)
{
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	
	FILE * jpeg_file = fopen(filename, "rb");
	
	if(!jpeg_file)
		rb_raise(rb_eRuntimeError, "\"%s\" can't be opened.", filename);
	
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);
		fclose(jpeg_file);
		rb_raise(rb_eRuntimeError, "JPEG Error(\"%s\")", filename);
	}
	
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, jpeg_file);
	jpeg_read_header(&cinfo, 1);
	jpeg_start_decompress(&cinfo);
	
	(*width) = cinfo.output_width;
	(*height) = cinfo.output_height;
	int components = cinfo.output_components;
	if(components != 3)
		rb_raise(rb_eRuntimeError, "JPEG Error: \"%s\" is not an RGB JPEG file.", filename);
	
	int rowbytes = (*width)*components;
	volatile VALUE data = rb_str_new(NULL, 0);
	rb_str_resize(data, rowbytes*(*height));
	
	JSAMPLE *pixels = (JSAMPLE *)StringValuePtr(data);
	JSAMPROW row = pixels + ((*height) - 1)*rowbytes;
	
	while (cinfo.output_scanline < *height) {
		jpeg_read_scanlines(&cinfo, &row, 1);
		row -= rowbytes;
	}
	
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(jpeg_file);
	
	return data;
}

static VALUE
rbas_loadJPEG_RGB(VALUE self, VALUE filename)
{
	int w, h;
	volatile VALUE data = load_jpeg(StringValuePtr(filename), &w, &h);
	
	return rb_struct_new(rbas_image, data, INT2FIX(w), INT2FIX(h), INT2NUM(GL_RGB));
}
*/
static VALUE
rbas_screen_shot(VALUE self, VALUE filename)
{
	int bytes = AS_SCREEN_W * AS_SCREEN_H * 3;
	GLubyte pixels[bytes];
	glReadPixels(0, 0, AS_SCREEN_W, AS_SCREEN_H, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	
	char *name = StringValuePtr(filename);
	FILE *png_file = fopen(name, "wb");
	if(!png_file)
		rb_raise(rb_eRuntimeError, "\"%s\" could not be opened for writing.", name);
	
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr)
		rb_raise(rb_eRuntimeError, "LibPNG error when saving \"%s\".", name);

	
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr)
		rb_raise(rb_eRuntimeError, "LibPNG error when saving \"%s\".", name);
	
	if(setjmp(png_jmpbuf(png_ptr)))
		rb_raise(rb_eRuntimeError, "LibPNG error when saving \"%s\".", name);

	png_init_io(png_ptr, png_file);
	
	png_set_IHDR(png_ptr, info_ptr, AS_SCREEN_W, AS_SCREEN_H,
	             8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
	             PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	
	png_byte* row_ptrs[AS_SCREEN_H];
	
	int i;
	for (i=0; i<AS_SCREEN_H; i++)
		row_ptrs[i] = pixels + (AS_SCREEN_H - 1 - i)*(AS_SCREEN_W*3);
	
	png_set_rows(png_ptr, info_ptr, row_ptrs);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
	
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(png_file);
	
	return Qnil;
}

static VALUE
rbas_getScreen(VALUE self)
{
	VALUE out_str = rb_str_new(NULL, 0);
	rb_str_resize(out_str, AS_SCREEN_W*AS_SCREEN_H*4);
	
	glReadPixels(0, 0, AS_SCREEN_W, AS_SCREEN_H, GL_RGBA, GL_UNSIGNED_BYTE, (GLubyte *)RSTRING_PTR(out_str));
	
	
	return out_str;
}

void Init_ASimage(void)
{
	rbas_image = rb_eval_string("AS::Image");

	rb_define_module_function(rbas_image, "load_png_rgba", &rbas_loadPNG_RGBA, 1);
	rb_define_module_function(rbas_image, "load_png_rgba_premult", &rbas_loadPNG_RGBA_pre, 1);
	rb_define_module_function(rbas_image, "load_png_rgb", &rbas_loadPNG_RGB, 1);
	rb_define_module_function(rbas_image, "load_png_lum", &rbas_loadPNG_LUM, 1);
	rb_define_module_function(rbas_image, "load_png_lum_as_alpha", &rbas_loadPNG_LUM_AS_ALPHA, 1);
	rb_define_module_function(rbas_image, "load_png_luma", &rbas_loadPNG_LUMA, 1);
	rb_define_module_function(rbas_image, "load_png_luma_premult", &rbas_loadPNG_LUMA_pre, 1);
	
	//rb_define_module_function(rbas_image, "load_jpeg_rgb", &rbas_loadJPEG_RGB, 1);
	
	rb_define_module_function(rbas_image, "screen_shot", &rbas_screen_shot, 1);
	rb_define_module_function(rbas_image, "get_screen", &rbas_getScreen, 0);
}
