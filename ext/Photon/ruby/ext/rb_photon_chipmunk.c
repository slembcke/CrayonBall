#include <ruby.h>

#ifdef __APPLE__
#include "OpenGL/gl.h"
#include "OpenGL/glu.h"
#else
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#endif

static VALUE c_cpSpace;
static VALUE c_cpBody;

#include "chipmunk.h"
#include "rb_chipmunk.h"

#include "photon.h"
#include "photon_chipmunk.h"
#include "rb_photon.h"

static VALUE
bodyNode_alloc(VALUE klass)
{
	return Data_Wrap_Struct(klass, NULL, free, psgBodyNodeAlloc());
}

static VALUE
bodyNode_init(VALUE self, VALUE body, VALUE child)
{
	psgBodyNode *node = (psgBodyNode *)NODE(self);
	rb_iv_set(self, "@body", body);
	rb_iv_set(self, "@child", child);

	psgBodyNodeInit(node, BODY(body), NODE(child));
	
	return self;
}

static VALUE
bodyPosNode_init(VALUE self, VALUE body, VALUE child)
{
	psgBodyNode *node = (psgBodyNode *)NODE(self);
	rb_iv_set(self, "@body", body);
	rb_iv_set(self, "@child", child);

	psgBodyPosNodeInit(node, BODY(body), NODE(child));
	
	return self;
}

typedef struct spaceNode {
	psgNode node;
	
	cpSpace *space;
} spaceNode;

extern void drawSpace(cpSpace *space);

static void
render_spaceNode(spaceNode *node, psgRenderState *curr_state, psgRenderState *target_state)
{
	psgBlendState blend = {0, GL_ONE, GL_ZERO};
	psgTextureState texture = {0, 0};

	psgRenderState set_state = *target_state;
	set_state.blend = &blend;
	set_state.texture = &texture;
	set_state.VAR = NULL;
	
	psgRenderStateTransition(curr_state, &set_state);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	drawSpace(node->space);
}

static VALUE
spaceNode_alloc(VALUE klass)
{
	return Data_Wrap_Struct(klass, NULL, free, calloc(1, sizeof(spaceNode)));
}

static VALUE
spaceNode_init(VALUE self, VALUE space)
{
	spaceNode *node = (spaceNode *)NODE(self);
	rb_iv_set(self, "@space", space);

	psgNodeInit((psgNode *)node, (psgNodeFunc *)render_spaceNode, NULL);
	node->space = SPACE(space);
	
	return self;
}

void Init_PhotonChipmunkEXT(void)
{
	c_cpSpace = rb_eval_string("CP::Space");
	c_cpBody = rb_eval_string("CP::Body");
	
	VALUE rb_cBodyNode = rb_define_class_under(rb_mNode, "Body", rb_cObject);
	rb_include_module(rb_cBodyNode, rb_mNode);
	rb_define_alloc_func(rb_cBodyNode, bodyNode_alloc);
	rb_define_attr(rb_cBodyNode, "body", 1, 0);
	rb_define_method(rb_cBodyNode, "initialize", bodyNode_init, 2);
	rb_photon_makeConstructable(rb_cBodyNode);
	
	VALUE rb_cBodyPosNode = rb_define_class_under(rb_mNode, "BodyPos", rb_cBodyNode);
	rb_include_module(rb_cBodyPosNode, rb_mNode);
	rb_define_alloc_func(rb_cBodyPosNode, bodyNode_alloc);
	rb_define_attr(rb_cBodyPosNode, "body", 1, 0);
	rb_define_method(rb_cBodyPosNode, "initialize", bodyPosNode_init, 2);
	rb_photon_makeConstructable(rb_cBodyPosNode);
	
	VALUE rb_cSpaceNode = rb_define_class_under(rb_mNode, "Space", rb_cObject);
	rb_include_module(rb_cSpaceNode, rb_mNode);
	rb_define_alloc_func(rb_cSpaceNode, spaceNode_alloc);
	rb_define_attr(rb_cSpaceNode, "space", 1, 0);
	rb_define_method(rb_cSpaceNode, "initialize", spaceNode_init, 1);
	
}
