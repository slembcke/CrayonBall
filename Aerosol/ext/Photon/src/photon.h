#include "photon_matrix.h"

typedef struct psgBlendState {
	int enable;
	GLenum sfactor;
	GLenum dfactor;
} psgBlendState;

typedef struct psgTextureState {
	GLenum tex_target;
	GLuint tex_id;
} psgTextureState;

typedef struct psgRenderState {
	GLfloat *color;
	psgBlendState *blend;
	psgTextureState *texture;
	GLfloat *VAR;
} psgRenderState;

struct psgNode;

void psgRenderStateTransition(psgRenderState *curr_state, psgRenderState *target_state);
typedef void (psgNodeFunc)(struct psgNode *node, psgRenderState *curr_state, psgRenderState *target_state);

typedef struct psgNode {
	psgNodeFunc *nodeFunc;
	
	int enabled;
	struct psgNode *child;
} psgNode;

static inline void
psgRenderNode(psgNode *node, psgRenderState *curr_state, psgRenderState *target_state)
{
//	assert(node);
	
	if(node->enabled){
		node->nodeFunc(node, curr_state, target_state);
	}
}

void psgNodeInit(psgNode *node, psgNodeFunc *nodeFunc, psgNode *child);
void psgNodeRenderRoot(psgNode *node);

typedef struct psgRenderStateNode {
	psgNode node;
	
	psgRenderState state;
} psgRenderStateNode;

psgRenderStateNode *psgRenderStateNodeAlloc();
psgNode *psgRenderStateNodeInit(psgRenderStateNode *node, psgRenderState *state, psgNode *child);
psgNode *psgRenderStateNodeNew(psgRenderState *state, psgNode *child);

typedef struct psgMatrixNode {
	psgNode node;
	
	GLenum mode;
	matrix_t *matrix;
} psgMatrixNode;

psgMatrixNode *psgMatrixNodeAlloc();
psgNode *psgMatrixNodeInit(psgMatrixNode *node, GLenum mode, matrix_t *matrix, psgNode *child);
psgNode *psgMatrixNodeNew(GLenum mode, matrix_t *matrix, psgNode *child);

typedef struct psgArrayNode {
	psgNode node;
	
	psgNode **array;
	int count;
} psgArrayNode;

psgArrayNode *psgArrayNodeAlloc();
psgNode *psgArrayNodeInit(psgArrayNode *node, psgNode **array, int count);
psgNode *psgArrayNodeNew(psgNode **array, int count);

typedef struct psgVARPtrs {
	GLint num_vertex;
	GLint num_color;
	GLint num_texcoord;
	
	GLfloat *ptr;
} psgVARPtrs;

void psgVARPtrsInit(psgVARPtrs *ptrs, GLfloat *VAR, int num_vertex, int num_color, int num_texcoord);
void psgBindVARPtrs(psgVARPtrs *ptrs, psgRenderState *curr_state);

typedef struct psgVARNode {
	psgNode node;

	psgVARPtrs ptrs;
	GLenum mode;
	GLint first;
	GLsizei count;
} psgVARNode;

psgVARNode *psgVARNodeAlloc();
psgNode *psgVARNodeInit(psgVARNode *node, psgVARPtrs *ptrs, GLenum mode, GLint first, GLsizei count);
psgNode *psgVARNodeNew(psgVARPtrs *ptrs, GLenum mode, GLint first, GLsizei count);

//typedef struct psgFontNode {
//	psgRenderStateNode node;
//
//	psgVARPtrs VARPtrs;
//	unsigned int *indexes;
//	GLfloat *advances;
//} psgFontNode;
//
//psgFontNode *psgFontNodeAlloc();
//psgNode * psgFontNodeInit(psgFontNode *node, psgVARPtrs *VARPtrs, unsigned int *indexes, GLfloat *advances);
//psgNode * psgFontNodeNew(psgVARPtrs *VARPtrs, unsigned int *indexes, GLfloat *advances);
