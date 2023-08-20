#include <stdlib.h>
#include <math.h>

#include <assert.h>

#ifdef __APPLE__
#include "OpenGL/gl.h"
#include "OpenGL/glu.h"
#else
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#endif

#include "photon.h"

void
psgRenderStateTransition(psgRenderState *curr_state, psgRenderState *target_state)
{
	glColor4fv(target_state->color);
	
	psgBlendState *blend = target_state->blend;
	if(blend != curr_state->blend){
		if(blend->enable){
			glEnable(GL_BLEND);
			glBlendFunc(blend->sfactor, blend->dfactor);
		} else {
			glDisable(GL_BLEND);
		}
		
		curr_state->blend = blend;
	}
	
	psgTextureState *texture = target_state->texture;
	if(texture != curr_state->texture){
		GLenum curr_target = curr_state->texture->tex_target;
		
		if(texture->tex_id){
			if(texture->tex_target != curr_target){
				if(curr_target) glDisable(curr_target);
				glEnable(texture->tex_target);
			}
			
			glBindTexture(texture->tex_target, texture->tex_id);
		} else {
			glDisable(curr_target);
		}
		
		curr_state->texture = texture;
	}
}

void
psgNodeInit(psgNode *node, psgNodeFunc *nodeFunc, psgNode *child)
{
	node->enabled = 1;
	node->nodeFunc = nodeFunc;
	node->child = child;
}

void
psgNodeRenderRoot(psgNode *node)
{
	GLfloat color[] = {1.0f, 1.0f, 1.0f, 1.0f};
	psgBlendState blend = {0, GL_ONE, GL_ZERO};
	psgTextureState texture = {0, 0};
	
	psgRenderState curr_state = {
		color,
		&blend,
		&texture,
		NULL,
	};
	
	// Set state
	glColor4fv(color);

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_RECTANGLE);
	
	{
		psgRenderState target_state = curr_state;
		psgRenderNode(node, &curr_state, &target_state);
	}
}


static void
render_stateChangeNode(psgRenderStateNode *node, psgRenderState *curr_state, psgRenderState *target_state)
{
	psgRenderState *node_state = &node->state;
	
	psgRenderState set_state = *target_state;
	if(node_state->color) set_state.color = node_state->color;
	if(node_state->blend) set_state.blend = node_state->blend;
	if(node_state->texture) set_state.texture = node_state->texture;
	
	psgRenderNode(node->node.child, curr_state, &set_state);
}

psgRenderStateNode *
psgRenderStateNodeAlloc()
{
	return calloc(1, sizeof(psgRenderStateNode));
}

psgNode *
psgRenderStateNodeInit(psgRenderStateNode *node, psgRenderState *state, psgNode *child)
{
	psgNodeInit((psgNode *)node, (psgNodeFunc *)render_stateChangeNode, child);
	
	node->state = *state;
	
	return (psgNode *)node;
}

psgNode *
psgRenderStateNodeNew(psgRenderState *state, psgNode *child)
{
	return psgRenderStateNodeInit(psgRenderStateNodeAlloc(), state, child);
}

static void
render_matrixNode(psgMatrixNode *node, psgRenderState *curr_state, psgRenderState *target_state)
{
	glMatrixMode(node->mode);
	glPushMatrix();
	{
		glMultMatrixf(node->matrix->farr);
		
		psgRenderNode(node->node.child, curr_state, target_state);
	}; 
	glMatrixMode(node->mode);
	glPopMatrix();
}

psgMatrixNode *
psgMatrixNodeAlloc()
{
	return calloc(1, sizeof(psgMatrixNode));
}

psgNode *
psgMatrixNodeInit(psgMatrixNode *node, GLenum mode, matrix_t *matrix, psgNode *child)
{
	psgNodeInit((psgNode *)node, (psgNodeFunc *)render_matrixNode, child);
	
	node->mode = mode;
	node->matrix = matrix;
	
	return (psgNode *)node;
}

psgNode *
psgMatrixNodeNew(GLenum mode, matrix_t *matrix, psgNode *child)
{
	return psgMatrixNodeInit(psgMatrixNodeAlloc(), mode, matrix, child);
}

static void
render_arrayNode(psgArrayNode *node, psgRenderState *curr_state, psgRenderState *target_state)
{
	for(int i=0, count=node->count; i<count; i++)
		psgRenderNode(node->array[i], curr_state, target_state);
}

psgArrayNode *
psgArrayNodeAlloc()
{
	return calloc(1, sizeof(psgArrayNode));
}

psgNode *
psgArrayNodeInit(psgArrayNode *node, psgNode **array, int count)
{
	psgNodeInit((psgNode *)node, (psgNodeFunc *)render_arrayNode, NULL);
	
	node->array = array;
	node->count = count;
	
	return (psgNode *)node;
}

psgNode *
psgArrayNodeNew(psgNode **array, int count)
{
	return psgArrayNodeInit(psgArrayNodeAlloc(), array, count);
}

void
psgVARPtrsInit(psgVARPtrs *ptrs, GLfloat *VAR, int num_vertex, int num_color, int num_texcoord)
{
	ptrs->num_vertex = num_vertex;
	ptrs->num_color = num_color;
	ptrs->num_texcoord = num_texcoord;
	
	ptrs->ptr = VAR;
}

void
psgBindVARPtrs(psgVARPtrs *ptrs, psgRenderState *curr_state)
{
	const int num_vertex = ptrs->num_vertex;
	const int num_color = ptrs->num_color;
	const int num_texcoord = ptrs->num_texcoord;
	
	const int color = num_vertex;
	const int texcoord = color + num_color;
	const int stride = (texcoord + num_texcoord)*sizeof(GLfloat);
	
	GLfloat *ptr = ptrs->ptr;
	if(curr_state->VAR != ptr){
		if(num_vertex){
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(num_vertex, GL_FLOAT, stride, ptr);
		} else {
			glDisableClientState(GL_VERTEX_ARRAY);
		}
		
		if(num_color){
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(num_color, GL_FLOAT, stride, ptr + color);
		} else {
			glDisableClientState(GL_COLOR_ARRAY);
		}
		
		if(num_texcoord){
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(num_texcoord, GL_FLOAT, stride, ptr + texcoord);
		} else {
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		
		curr_state->VAR = ptr;
	}
}

static void
render_VARNode(psgVARNode *node, psgRenderState *curr_state, psgRenderState *target_state)
{
	psgRenderStateTransition(curr_state, target_state);
	psgBindVARPtrs(&node->ptrs, curr_state);
	
	glDrawArrays(node->mode, node->first, node->count);
}

psgVARNode *
psgVARNodeAlloc()
{
	return calloc(1, sizeof(psgVARNode));
}

psgNode *
psgVARNodeInit(psgVARNode *node, psgVARPtrs *ptrs, GLenum mode, GLint first, GLsizei count)
{
	psgNodeInit((psgNode *)node, (psgNodeFunc *)render_VARNode, NULL);
	
	node->ptrs = *ptrs;
	node->mode = mode;
	node->first = first;
	node->count = count;
	
	return (psgNode *)node;
}

psgNode *
psgVARNodeNew(psgVARPtrs *ptrs, GLenum mode, GLint first, GLsizei count)
{
	return psgVARNodeInit(psgVARNodeAlloc(), ptrs, mode, first, count);
}
