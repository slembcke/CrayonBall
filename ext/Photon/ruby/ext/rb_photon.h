extern VALUE rb_mNode;

static inline psgNode *
NODE(VALUE self)
{
	psgNode *node;
	Data_Get_Struct(self, psgNode, node);
	
	return node;
}

void rb_photon_makeConstructable(VALUE klass);
