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

#include "chipmunk.h"

#include "photon.h"
#include "photon_chipmunk.h"

static void
render_bodyNode(psgBodyNode *node, psgRenderState *curr_state, psgRenderState *target_state)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	{
		cpBody *body = node->body;
		cpVect pos = body->p;
		cpVect rot = body->rot;
		
		matrix_t matrix = (matrix_t){.farr={
			 rot.x, rot.y, 0.0f, 0.0f,
			-rot.y, rot.x, 0.0f, 0.0f,
			  0.0f,  0.0f, 1.0f, 1.0f,
			 pos.x, pos.y, 0.0f, 1.0f,
		}};

		glMultMatrixf(matrix.farr);
		
		psgRenderNode(node->node.child, curr_state, target_state);
	};
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

psgBodyNode *
psgBodyNodeAlloc()
{
	return calloc(1, sizeof(psgBodyNode));
}

psgNode *
psgBodyNodeInit(psgBodyNode *node, cpBody *body, psgNode *child)
{
	psgNodeInit((psgNode *)node, (psgNodeFunc *)render_bodyNode, child);
	
	node->body = body;
	
	return (psgNode *)node;
}

psgNode *
psgBodyNodeNew(cpBody *body, psgNode *child)
{
	return psgBodyNodeInit(psgBodyNodeAlloc(), body, child);
}

static void
render_bodyPosNode(psgBodyNode *node, psgRenderState *curr_state, psgRenderState *target_state)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	{
		cpBody *body = node->body;
		cpVect pos = body->p;
		
		matrix_t matrix = (matrix_t){.farr={
				1.0f,  0.0f, 0.0f, 0.0f,
				0.0f,  1.0f, 0.0f, 0.0f,
			  0.0f,  0.0f, 1.0f, 1.0f,
			 pos.x, pos.y, 0.0f, 1.0f,
		}};

		glMultMatrixf(matrix.farr);
		
		psgRenderNode(node->node.child, curr_state, target_state);
	};
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

psgNode *
psgBodyPosNodeInit(psgBodyNode *node, cpBody *body, psgNode *child)
{
	psgNodeInit((psgNode *)node, (psgNodeFunc *)render_bodyPosNode, child);
	
	node->body = body;
	
	return (psgNode *)node;
}

psgNode *
psgBodyPosNodeNew(cpBody *body, psgNode *child)
{
	return psgBodyPosNodeInit(psgBodyNodeAlloc(), body, child);
}
