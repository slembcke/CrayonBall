#ifndef rboal_h
#define rboal_h

static inline unsigned int rb_ary_size(VALUE array)
{
    return RARRAY_LEN(array);//((struct RArray *)array)->len;
}

#endif
