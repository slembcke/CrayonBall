typedef struct psgBodyNode {
	psgNode node;
	
	cpBody *body;
} psgBodyNode;

psgBodyNode *psgBodyNodeAlloc();
psgNode *psgBodyNodeInit(psgBodyNode *node, cpBody *body, psgNode *child);
psgNode *psgBodyNodeNew(cpBody *body, psgNode *child);

psgNode *psgBodyPosNodeInit(psgBodyNode *node, cpBody *body, psgNode *child);
psgNode *psgBodyPosNodeNew(cpBody *body, psgNode *child);
