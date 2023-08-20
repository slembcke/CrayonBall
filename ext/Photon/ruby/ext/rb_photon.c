#include <ruby.h>

#ifdef __APPLE__
#include "OpenGL/gl.h"
#include "OpenGL/glu.h"
#else
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#endif

#include "photon.h"
#include "rb_photon.h"

VALUE rb_cMatrix;
VALUE rb_mNode;
VALUE rb_cColor;
VALUE rb_cBlend;
VALUE rb_cTexture;

static inline matrix_t *
MATRIX(VALUE self)
{
	matrix_t *matrix;
	Data_Get_Struct(self, matrix_t, matrix);
	
	return matrix;
}

static VALUE
matrix_alloc(VALUE klass)
{
	return Data_Wrap_Struct(klass, NULL, free, malloc(sizeof(matrix_t)));
}

static VALUE
matrix_init(int argc, VALUE *argv, VALUE self)
{
	matrix_t *matrix = MATRIX(self);

	if(argc == 0){
		(*matrix) = mIdentity();
	} else if(argc == 16){
		for(int i=0; i<16; i++) matrix->farr[i] = NUM2DBL(argv[i]);
	} else {
		rb_raise(rb_eArgError, "Photon::Matrix#initialize must take either 0 or 16 arguments.");
	}
	
	return self;
}

static VALUE
matrix_toString(VALUE self)
{
	vmfloat_t *arr = MATRIX(self)->farr;
	VALUE str = rb_str_new(NULL, 0);
	
	rb_str_concat(str, rb_str_new2("Matrix = [\n"));
	
	for(int i=0; i<16; i+=4){
		char buff[64];
		snprintf(buff, sizeof(buff), "  [% 6.2f % 6.2f % 6.2f % 6.2f]\n", arr[i+0], arr[i+1], arr[i+2], arr[i+3]);
		
		rb_str_concat(str, rb_str_new2(buff));
	}
	
	rb_str_concat(str, rb_str_new2("]"));
	
	return str;
}


static VALUE
matrix_translate(VALUE self, VALUE x, VALUE y)
{
	matrix_t *matrix = MATRIX(self);
	(*matrix) = mTransform(*matrix, psgTranslationMatrix(NUM2DBL(x), NUM2DBL(y)));
	
	return self;
}

static VALUE
matrix_scale(VALUE self, VALUE x, VALUE y)
{
	matrix_t *matrix = MATRIX(self);
	(*matrix) = mTransform(*matrix, psgScaleMatrix(NUM2DBL(x), NUM2DBL(y)));
	
	return self;
}

static VALUE
matrix_rotate(VALUE self, VALUE radians)
{
	matrix_t *matrix = MATRIX(self);
	(*matrix) = mTransform(*matrix, psgRotateMatrix(NUM2DBL(radians)));
	
	return self;
}

static VALUE
matrix_boneScale(VALUE self, VALUE x0, VALUE y0, VALUE x1, VALUE y1)
{
	matrix_t *matrix = MATRIX(self);
	(*matrix) = mTransform(*matrix, psgBoneScaleMatrix(NUM2DBL(x0), NUM2DBL(y0), NUM2DBL(x1), NUM2DBL(y1)));
	
	return self;
}

static VALUE
matrix_sprite(VALUE self, VALUE x, VALUE y, VALUE sx, VALUE sy, VALUE radians)
{
	matrix_t *matrix = MATRIX(self);
	(*matrix) = mTransform(*matrix, psgSpriteMatrix(NUM2DBL(x), NUM2DBL(y), NUM2DBL(sx), NUM2DBL(sy), NUM2DBL(radians)));
	
	return self;
}

static VALUE
matrix_ortho(VALUE self, VALUE left, VALUE right, VALUE bottom, VALUE top)
{
	matrix_t *matrix = MATRIX(self);
	(*matrix) = mTransform(*matrix, psgOrthoMatrix(NUM2DBL(left), NUM2DBL(right), NUM2DBL(bottom), NUM2DBL(top )));
	
	return self;
}

static VALUE
node_reconstruct(VALUE self, VALUE args)
{
	if(rb_block_given_p())
		rb_ary_push(args, rb_yield(Qnil));
	
	return rb_apply(self, rb_intern("initialize"), args);
}

static VALUE
node_render(VALUE self)
{
	psgNodeRenderRoot(NODE(self));
	
	return Qnil;
}

static VALUE
node_getEnabled(VALUE self)
{
	return NODE(self)->enabled ? Qtrue : Qfalse;
}

static VALUE
node_setEnabled(VALUE self, VALUE value)
{
	NODE(self)->enabled = RTEST(value) ? 1 : 0;
	
	return value;
}

static VALUE
photonNode_construct(VALUE klass, VALUE args)
{
	if(rb_block_given_p())
		rb_ary_push(args, rb_yield(Qnil));
	
	return rb_apply(klass, rb_intern("new"), args);
}

void
rb_photon_makeConstructable(VALUE klass)
{
	rb_define_singleton_method(klass, "construct", photonNode_construct, -2);
}

static inline vector_t *
COLOR(VALUE self)
{
	vector_t *color;
	Data_Get_Struct(self, vector_t, color);
	
	return color;
}

static VALUE
color_alloc(VALUE klass)
{
	return Data_Wrap_Struct(klass, NULL, free, calloc(4, sizeof(GLfloat)));
}

static VALUE
color_init(int argc, VALUE *argv, VALUE self)
{
	vector_t *color = COLOR(self);
	
	VALUE r,g,b,a;
	rb_scan_args(argc, argv, "31", &r, &g, &b, &a);
	
	(*color) = (vector_t){.farr={
		CLAMP(NUM2DBL(r), 0.0f, 1.0f),
		CLAMP(NUM2DBL(g), 0.0f, 1.0f),
		CLAMP(NUM2DBL(b), 0.0f, 1.0f),
		CLAMP((a == Qnil) ? 1.0f : NUM2DBL(a), 0.0f, 1.0f),
	}};
	
	return self;
}

static inline VALUE
new_color(vector_t color)
{
	vector_t *new_color = calloc(1, sizeof(vector_t));
	(*new_color) = color;
	
	return Data_Wrap_Struct(rb_cColor, NULL, free, new_color);
}

static VALUE
color_new_grey(int argc, VALUE *argv, VALUE self)
{
	VALUE v_rb, a_rb;
	rb_scan_args(argc, argv, "11", &v_rb, &a_rb);
	
	vmfloat_t v = CLAMP(NUM2DBL(v_rb), 0.0f, 1.0f);
	vmfloat_t a = CLAMP(NUM2DBL((a_rb == Qnil) ? 1.0f : NUM2DBL(a_rb)), 0.0f, 1.0f);
	
	return new_color((vector_t){.farr={v,v,v,a}});
}

static inline vmfloat_t
frac(vmfloat_t v)
{
	v = fmod(v, 1.0f);
	return v<0.0f ? -v : v;
}

static VALUE
color_new_hsv(int argc, VALUE *argv, VALUE self)
{
	VALUE h_rb, s_rb, v_rb, a_rb;
	rb_scan_args(argc, argv, "31", &h_rb, &s_rb, &v_rb, &a_rb);
	
	vmfloat_t h = frac(NUM2DBL(h_rb));
	vmfloat_t s = CLAMP(NUM2DBL(s_rb), 0.0f, 1.0f);
	vmfloat_t v = CLAMP(NUM2DBL(v_rb), 0.0f, 1.0f);
	vmfloat_t a = (a_rb == Qnil) ? 1.0f : CLAMP(NUM2DBL(a_rb), 0.0f, 1.0f);
	
	if(s == 0.0f)
		return new_color((vector_t){.farr={v,v,v,a}});
	
	vmfloat_t f = frac(h*6.0f);
	
	vmfloat_t p = v*(1.0f - s);
	vmfloat_t q = v*(1.0f - s*f);
	vmfloat_t t = v*(1.0f - s*(1.0f - f));
	
	switch((int)(h*6.0f)){
		case 0:  return new_color((vector_t){.farr={v,t,p,a}});
		case 1:  return new_color((vector_t){.farr={q,v,p,a}});
		case 2:  return new_color((vector_t){.farr={p,v,t,a}});
		case 3:  return new_color((vector_t){.farr={p,q,v,a}});
		case 4:  return new_color((vector_t){.farr={t,p,v,a}});
		default: return new_color((vector_t){.farr={v,p,q,a}});
	}
}

static VALUE
color_set(VALUE self, VALUE other)
{
	(*COLOR(self)) = *COLOR(other);
	
	return other;
}

static VALUE
color_getR(VALUE self)
{
	return rb_float_new(COLOR(self)->farr[0]);
}

static VALUE
color_getG(VALUE self)
{
	return rb_float_new(COLOR(self)->farr[1]);
}

static VALUE
color_getB(VALUE self)
{
	return rb_float_new(COLOR(self)->farr[2]);
}

static VALUE
color_getA(VALUE self)
{
	return rb_float_new(COLOR(self)->farr[3]);
}

static VALUE
color_lerp(VALUE self, VALUE blend, VALUE color)
{
	vector_t a = *COLOR(self);
	vector_t b = *COLOR(color);
	vmfloat_t t = CLAMP(NUM2DBL(blend), 0.0f, 1.0f);
	
	return new_color((vector_t){.farr={
		a.farr[0] + (b.farr[0] - a.farr[0])*t,
		a.farr[1] + (b.farr[1] - a.farr[1])*t,
		a.farr[2] + (b.farr[2] - a.farr[2])*t,
		a.farr[3] + (b.farr[3] - a.farr[3])*t,
	}});
}

static inline psgBlendState *
BLEND(VALUE self)
{
	psgBlendState *blend;
	Data_Get_Struct(self, psgBlendState, blend);
	
	return blend;
}

static VALUE
blend_alloc(VALUE klass)
{
	return Data_Wrap_Struct(klass, NULL, free, calloc(1, sizeof(psgBlendState)));
}

static VALUE
blend_init(VALUE self, VALUE sfactor, VALUE dfactor)
{
	psgBlendState *blend = BLEND(self);
	
	blend->enable = 1;
	blend->sfactor = NUM2UINT(sfactor);
	blend->dfactor = NUM2UINT(dfactor);
		
	return self;
}

static inline psgTextureState *
TEXTURE(VALUE self)
{
	psgTextureState *texture;
	Data_Get_Struct(self, psgTextureState, texture);
	
	return texture;
}

static VALUE
texture_alloc(VALUE klass)
{
	return Data_Wrap_Struct(klass, NULL, free, calloc(1, sizeof(psgTextureState)));
}

static VALUE
texture_init(VALUE self, VALUE tex_target, VALUE tex_id)
{
	psgTextureState *texture = TEXTURE(self);
	
	texture->tex_target = NUM2UINT(tex_target);
	texture->tex_id = NUM2UINT(tex_id);
		
	return self;
}

static VALUE
renderStateNode_alloc(VALUE klass)
{
	return Data_Wrap_Struct(klass, NULL, free, psgRenderStateNodeAlloc());
}

static VALUE
renderStateNode_init(VALUE self, VALUE state_hash, VALUE child)
{
	psgRenderStateNode *node = (psgRenderStateNode *)NODE(self);
	rb_iv_set(self, "@state_hash", state_hash);
	rb_iv_set(self, "@child", child);
	
	rb_hash_freeze(state_hash);

	VALUE color = rb_hash_aref(state_hash, ID2SYM(rb_intern("color")));
	VALUE blend = rb_hash_aref(state_hash, ID2SYM(rb_intern("blend")));
	VALUE texture = rb_hash_aref(state_hash, ID2SYM(rb_intern("texture")));

	psgRenderState state = {
		color == Qnil ? NULL : COLOR(color)->farr,
		blend == Qnil ? NULL : BLEND(blend),
		texture == Qnil ? NULL : TEXTURE(texture),
	};
	
	psgRenderStateNodeInit(node, &state, NODE(child));
	
	return self;
}

static VALUE
matrixNode_alloc(VALUE klass)
{
	return Data_Wrap_Struct(klass, NULL, free, psgMatrixNodeAlloc());
}

static VALUE
matrixNode_init(VALUE self, VALUE mode, VALUE matrix, VALUE child)
{
	psgMatrixNode *node = (psgMatrixNode *)NODE(self);
	rb_iv_set(self, "@matrix", matrix);
	rb_iv_set(self, "@child", child);

	psgMatrixNodeInit(node, NUM2UINT(mode), MATRIX(matrix), NODE(child));
	
	return self;
}

static VALUE
VARNode_alloc(VALUE klass)
{
	return Data_Wrap_Struct(klass, NULL, free, psgVARNodeAlloc());
}

static VALUE
VARNode_init(VALUE self, VALUE VAR, VALUE mode, VALUE first, VALUE count)
{
	psgVARNode *node = (psgVARNode *)NODE(self);
	rb_iv_set(self, "VAR", VAR);

	int num_vertex = NUM2INT(rb_funcall(VAR, rb_intern("num_vertex"), 0));	
	int num_color = NUM2INT(rb_funcall(VAR, rb_intern("num_color"), 0));	
	int num_texcoord = NUM2INT(rb_funcall(VAR, rb_intern("num_texcoord"), 0));
	
	VALUE packed = rb_funcall(VAR, rb_intern("pack"), 0);
	GLfloat *ptr = (GLfloat *)StringValuePtr(packed);	

	psgVARPtrs ptrs;
	psgVARPtrsInit(&ptrs, ptr, num_vertex, num_color, num_texcoord);
	
	psgVARNodeInit(node, &ptrs, NUM2UINT(mode), NUM2UINT(first), NUM2UINT(count));
	
	return self;
}


typedef struct fontNode {
	psgRenderStateNode node;
	
	psgTextureState *texture;
	psgVARPtrs VARPtrs;
	unsigned int *indexes;
	GLfloat *advances;
	GLfloat lineSkip;
	
	VALUE str;
} fontNode;

static void
render_fontNode(fontNode *node, psgRenderState *curr_state, psgRenderState *target_state)
{
	psgRenderState set_state = *target_state;
	set_state.texture = node->texture;
	
	psgRenderStateTransition(curr_state, &set_state);
	psgBindVARPtrs(&node->VARPtrs, curr_state);
	
	char *str = StringValuePtr(node->str);
	int len = strlen(str);
	
	glMatrixMode(GL_MODELVIEW);
	// Second push is for tracking newlines
	glPushMatrix(); glPushMatrix(); {			
		for(int i=0; i<len; i++){
			char c = str[i];
			
			if(c == ' '){
				glTranslatef(node->advances[c], 0.0f, 0.0f);
				continue;
			} else if(c == '\n'){
				glPopMatrix();
				glTranslatef(0.0f, -node->lineSkip, 0.0f);
				glPushMatrix();
				continue;
			}
			
			glDrawArrays(GL_TRIANGLE_STRIP, node->indexes[c], 4);
			glTranslatef(node->advances[c], 0.0f, 0.0f);
		}
	} glPopMatrix(); glPopMatrix();
}

static VALUE
FontNode_alloc(VALUE klass)
{
	return Data_Wrap_Struct(klass, NULL, free, calloc(1, sizeof(fontNode)));
}

static VALUE
FontNode_init(VALUE self, VALUE font, VALUE str)
{
	fontNode *node = (fontNode *)NODE(self);
	
	VALUE VAR = rb_funcall(font, rb_intern("VAR"), 0);

	rb_iv_set(self, "@str", str);
	rb_iv_set(self, "VAR", VAR);

	int num_vertex = NUM2INT(rb_funcall(VAR, rb_intern("num_vertex"), 0));	
	int num_color = NUM2INT(rb_funcall(VAR, rb_intern("num_color"), 0));	
	int num_texcoord = NUM2INT(rb_funcall(VAR, rb_intern("num_texcoord"), 0));
	
	VALUE packed = rb_funcall(VAR, rb_intern("pack"), 0);
	GLfloat *ptr = (GLfloat *)StringValuePtr(packed);	

	psgVARPtrs VARPtrs;
	psgVARPtrsInit(&VARPtrs, ptr, num_vertex, num_color, num_texcoord);
	
	VALUE indexes = rb_funcall(font, rb_intern("packed_indexes"), 0);
	VALUE advances = rb_funcall(font, rb_intern("packed_advances"), 0);
	//fontNodeInit(node, &ptrs, (unsigned int *)StringValuePtr(indexes), (GLfloat *)StringValuePtr(advances));
	
	
	psgNodeInit((psgNode *)node, (psgNodeFunc *)render_fontNode, NULL);
	
	node->texture = TEXTURE(rb_funcall(font, rb_intern("tex"), 0));
	node->VARPtrs = VARPtrs;
	node->indexes = (unsigned int *)StringValuePtr(indexes);
	node->advances = (GLfloat *)StringValuePtr(advances);
	node->lineSkip = NUM2DBL(rb_funcall(font, rb_intern("line_skip"), 0));
	node->str = str;

	return self;
}

typedef struct arrayNode {
	psgNode node;
	
	VALUE children;
} arrayNode;

static void
render_arrayNode(arrayNode *node, psgRenderState *curr_state, psgRenderState *target_state)
{
	VALUE children = node->children;
	VALUE *arr = RARRAY_PTR(children);
	
	for(int i=0, count=RARRAY_LEN(children); i<count; i++){
		psgNode *child = NODE(arr[i]);
		
		psgRenderNode(child, curr_state, target_state);
	}
}

static VALUE
arrayNode_alloc(VALUE klass)
{
	return Data_Wrap_Struct(klass, NULL, free, calloc(1, sizeof(arrayNode)));
}

static VALUE
arrayNode_init(VALUE self, VALUE children)
{
	arrayNode *node = (arrayNode *)NODE(self);
	rb_iv_set(self, "@children", children);
	
	psgNodeInit((psgNode *)node, (psgNodeFunc *)render_arrayNode, NULL);
	node->children = children;
		
	return self;
}

typedef struct mapNode {
	psgNode node;
	
	VALUE array;
	VALUE block;
} mapNode;

static void
render_mapNode(mapNode *node, psgRenderState *curr_state, psgRenderState *target_state)
{
	VALUE array = node->array;
	VALUE *arr = RARRAY_PTR(array);
	VALUE block = node->block;
	
	ID id_call = rb_intern("call");
	for(int i=0, count=RARRAY_LEN(array); i<count; i++){
		psgNode *mapped_node = NODE(rb_funcall(block, id_call, 1, arr[i]));
		psgRenderNode(mapped_node, curr_state, target_state);
	}
}

static VALUE
mapNode_alloc(VALUE klass)
{
	return Data_Wrap_Struct(klass, NULL, free, calloc(1, sizeof(mapNode)));
}

static VALUE
mapNode_init(int argc, VALUE *argv, VALUE self)
{
	VALUE array, block;
	rb_scan_args(argc, argv, "1&", &array, &block);
	rb_iv_set(self, "@array", array);
	rb_iv_set(self, "block", block);
	
	mapNode *node = (mapNode *)NODE(self);
	psgNodeInit((psgNode *)node, (psgNodeFunc *)render_mapNode, NULL);
	node->array = array;
	node->block = block; //TODO check that block is not nil
		
	return self;
}

extern void Init_PhotonChipmunkEXT(void);

void Init_photonEXT(void)
{
	VALUE rb_mPhoton = rb_define_module("Photon");
	
	rb_cMatrix = rb_define_class_under(rb_mPhoton, "Matrix", rb_cObject);
	rb_define_alloc_func(rb_cMatrix, matrix_alloc);
	rb_define_method(rb_cMatrix, "initialize", matrix_init, -1);
	rb_define_method(rb_cMatrix, "reset!", matrix_init, -1);
	rb_define_method(rb_cMatrix, "to_s", matrix_toString, 0);
	rb_define_method(rb_cMatrix, "translate!", matrix_translate, 2);
	rb_define_method(rb_cMatrix, "scale!", matrix_scale, 2);
	rb_define_method(rb_cMatrix, "rotate!", matrix_rotate, 1);
	rb_define_method(rb_cMatrix, "bone_scale!", matrix_boneScale, 4);
	rb_define_method(rb_cMatrix, "sprite!", matrix_sprite, 5);
	rb_define_method(rb_cMatrix, "ortho!", matrix_ortho, 4);
	
	rb_mNode = rb_define_module_under(rb_mPhoton, "Node");
	rb_define_attr(rb_mNode, "child", 1, 0);
	rb_define_method(rb_mNode, "reconstruct", node_reconstruct, -2);
//	rb_define_method(rb_mNode, "child=", node_setChild, 1);
	rb_define_method(rb_mNode, "enabled", node_getEnabled, 0);
	rb_define_method(rb_mNode, "enabled=", node_setEnabled, 1);
	rb_define_method(rb_mNode, "render", node_render, 0);
	
	rb_cColor = rb_define_class_under(rb_mPhoton, "Color", rb_cObject);
	rb_define_alloc_func(rb_cColor, color_alloc);
	rb_define_method(rb_cColor, "initialize", color_init, -1);
	rb_define_singleton_method(rb_cColor, "new_grey", color_new_grey, -1);
	rb_define_singleton_method(rb_cColor, "new_hsv", color_new_hsv, -1);
	rb_define_method(rb_cColor, "color=", color_set, 1);
	rb_define_method(rb_cColor, "r", color_getR, 0);
	rb_define_method(rb_cColor, "g", color_getG, 0);
	rb_define_method(rb_cColor, "b", color_getB, 0);
	rb_define_method(rb_cColor, "a", color_getA, 0);
	rb_define_method(rb_cColor, "lerp", color_lerp, 2);
	
	rb_cBlend = rb_define_class_under(rb_mPhoton, "Blend", rb_cObject);
	rb_define_alloc_func(rb_cBlend, blend_alloc);
	rb_define_method(rb_cBlend, "initialize", blend_init, 2);

	rb_cTexture = rb_define_class_under(rb_mPhoton, "Texture", rb_cObject);
	rb_define_alloc_func(rb_cTexture, texture_alloc);
	rb_define_method(rb_cTexture, "set_data", texture_init, 2);

	VALUE rb_cRenderStateNode = rb_define_class_under(rb_mNode, "RenderState", rb_cObject);
	rb_include_module(rb_cRenderStateNode, rb_mNode);
	rb_define_alloc_func(rb_cRenderStateNode, renderStateNode_alloc);
	rb_define_attr(rb_cRenderStateNode, "state_hash", 1, 0);
	rb_define_method(rb_cRenderStateNode, "initialize", renderStateNode_init, 2);
	rb_photon_makeConstructable(rb_cRenderStateNode);
	
	VALUE rb_cMatrixNode = rb_define_class_under(rb_mNode, "Matrix", rb_cObject);
	rb_include_module(rb_cMatrixNode, rb_mNode);
	rb_define_alloc_func(rb_cMatrixNode, matrixNode_alloc);
	rb_define_attr(rb_cMatrixNode, "matrix", 1, 0);
	rb_define_method(rb_cMatrixNode, "initialize", matrixNode_init, 3);
	rb_photon_makeConstructable(rb_cMatrixNode);

	VALUE rb_cVARNode = rb_define_class_under(rb_mNode, "VAR", rb_cObject);
	rb_include_module(rb_cVARNode, rb_mNode);
	rb_define_alloc_func(rb_cVARNode, VARNode_alloc);
	rb_define_method(rb_cVARNode, "initialize", VARNode_init, 4);

	VALUE rb_cFontNode = rb_define_class_under(rb_mNode, "Text", rb_cObject);
	rb_include_module(rb_cFontNode, rb_mNode);
	rb_define_alloc_func(rb_cFontNode, FontNode_alloc);
	rb_define_method(rb_cFontNode, "initialize", FontNode_init, 2);
	rb_define_attr(rb_mNode, "str", 1, 0);

	VALUE rb_cArrayNode = rb_define_class_under(rb_mNode, "Array", rb_cObject);
	rb_include_module(rb_cArrayNode, rb_mNode);
	rb_define_alloc_func(rb_cArrayNode, arrayNode_alloc);
	rb_define_attr(rb_mNode, "children", 1, 0);
	rb_define_method(rb_cArrayNode, "initialize", arrayNode_init, 1);
	rb_photon_makeConstructable(rb_cArrayNode);
	
	VALUE rb_cMapNode = rb_define_class_under(rb_mNode, "Map", rb_cObject);
	rb_include_module(rb_cMapNode, rb_mNode);
	rb_define_alloc_func(rb_cMapNode, mapNode_alloc);
	rb_define_attr(rb_mNode, "array", 1, 0);
	rb_define_method(rb_cMapNode, "initialize", mapNode_init, -1);
	
	Init_PhotonChipmunkEXT();
}
