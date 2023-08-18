/*
 * $Date: 2004/01/27 05:31:34 $
 * $Author: keith $
 */
#if defined(WIN32)
# include "windows.h"
#endif
#if defined(__APPLE_CC__)
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif
#include "rbogl.h"

#ifdef WIN32
typedef void (CALLBACK*(VOIDFUNC))();
#else
typedef void (*VOIDFUNC)();
#endif

#if defined(GLU_VERSION_1_2)
typedef GLUtesselator tesselatorObj;
#else /* GLU_VERSION_1_2 */
typedef GLUtriangulatorObj tesselatorObj;
#endif /* !GLU_VERSION_1_2 */

#ifdef MESA
struct glu_MesaStack {
    int len;
    GLfloat **ptr;
};

static struct glu_MesaStack gms = {0, NULL};
#endif

struct nurbsdata {
    GLUnurbsObj *nobj;
};
struct tessdata {
    tesselatorObj *tobj;
    VALUE t_ref;
};

struct quaddata {
    GLUquadricObj *qobj;
};

static VALUE cNurbs;
static VALUE cTess;
static VALUE cQuad;

#define GetNURBS(obj, ndata) {\
    Data_Get_Struct(obj, struct nurbsdata, ndata);\
    if (ndata->nobj == NULL) rb_raise(rb_eRuntimeError, "Nurbs Object already deleted!");\
}

#define GetTESS(obj, tdata) {\
    Data_Get_Struct(obj, struct tessdata, tdata);\
    if (tdata->tobj == NULL) rb_raise(rb_eRuntimeError, "Triangulator Object already deleted!");\
}

#define GetQUAD(obj, qdata) {\
    Data_Get_Struct(obj, struct quaddata, qdata);\
    if (qdata->qobj == NULL) rb_raise(rb_eRuntimeError, "Quadric Object already deleted!");\
}

static ID callId;
static ID refId;

/*
 * GLU Implementation
 */

/*
 * Nurbs
 */
/* from nurbscrv.c */
static int
get_curve_dim(type)
GLenum type;
{
        switch(type)
        {
                case GL_MAP1_VERTEX_3:                  return 3;
                case GL_MAP1_VERTEX_4:                  return 4;  
                case GL_MAP1_INDEX:                             return 1;
                case GL_MAP1_COLOR_4:                   return 4;
                case GL_MAP1_NORMAL:                    return 3;
                case GL_MAP1_TEXTURE_COORD_1:   return 1;
                case GL_MAP1_TEXTURE_COORD_2:   return 2;
                case GL_MAP1_TEXTURE_COORD_3:   return 3;
                case GL_MAP1_TEXTURE_COORD_4:   return 4;
                default:  abort();  /* TODO: is this OK? */
        }
        return 0; /*never get here*/
}
/* from nurbssrf.c */
static int
get_surface_dim(GLenum type)
{
        switch(type)
        {
                case GL_MAP2_VERTEX_3:                  return 3;
                case GL_MAP2_VERTEX_4:                  return 4;
                case GL_MAP2_INDEX:                             return 1;
                case GL_MAP2_COLOR_4:                   return 4;
                case GL_MAP2_NORMAL:                    return 3;
                case GL_MAP2_TEXTURE_COORD_1:   return 1; 
                case GL_MAP2_TEXTURE_COORD_2:   return 2;
                case GL_MAP2_TEXTURE_COORD_3:   return 3;
                case GL_MAP2_TEXTURE_COORD_4:   return 4;
                default:  abort();  /* TODO: is this OK? */
        }
        return 0; /*never get here*/
}

/*
 * NURBS API
 */
static void
free_nurbs(ndata)
struct nurbsdata *ndata;
{
  if (ndata->nobj) gluDeleteNurbsRenderer(ndata->nobj);
  ndata->nobj = NULL;
}
static VALUE
glu_NewNurbsRenderer(obj)
VALUE obj;
{
    VALUE ret;
    struct nurbsdata *ndata;
    ret = Data_Make_Struct(cNurbs, struct nurbsdata, 0, free_nurbs, ndata);
    ndata->nobj = gluNewNurbsRenderer();
    return ret;
}
static VALUE
glu_DeleteNurbsRenderer(obj, arg1)
VALUE obj, arg1;
{
    struct nurbsdata *ndata;
    GetNURBS(arg1, ndata);
    free_nurbs(ndata);
    return Qnil;
}
static VALUE
glu_NurbsProperty(obj, arg1, arg2, arg3)
VALUE obj, arg1, arg2, arg3;
{
    struct nurbsdata *ndata;
    GLenum property;
    GLfloat value;
    GetNURBS(arg1, ndata);
    property = (GLenum)NUM2INT(arg2);
    value = (GLfloat)NUM2DBL(arg3);
    gluNurbsProperty(ndata->nobj, property, value);
    return Qnil;
}
static VALUE
glu_GetNurbsProperty(obj, arg1, arg2)
VALUE obj, arg1, arg2;
{
    struct nurbsdata *ndata;
    GLenum property;
    GLfloat value;
    GetNURBS(arg1, ndata);
    property = (GLenum)NUM2INT(arg2);
    gluGetNurbsProperty(ndata->nobj, property, &value);
    return rb_float_new(value);
}
static VALUE
glu_BeginCurve(obj, arg1)
VALUE obj, arg1;
{
    struct nurbsdata *ndata;
    GetNURBS(arg1, ndata);
    gluBeginCurve(ndata->nobj);
    return Qnil;
}
static VALUE
glu_EndCurve(obj, arg1)
VALUE obj, arg1;
{
    struct nurbsdata *ndata;
    GetNURBS(arg1, ndata);
    gluEndCurve(ndata->nobj);
    /* hack for Mesa 3.1 */
#ifdef MESA
    for (gms.len;gms.len>0;gms.len--)
	free(gms.ptr[gms.len-1]);
    free(gms.ptr);
    gms.ptr = NULL;
#endif
    return Qnil;
}

static VALUE
glu_BeginSurface(obj, arg1)
VALUE obj, arg1;
{
    struct nurbsdata *ndata;
    GetNURBS(arg1, ndata);
    gluBeginSurface(ndata->nobj);
    return Qnil;
}
static VALUE
glu_EndSurface(obj, arg1)
VALUE obj, arg1;
{
    struct nurbsdata *ndata;
    GetNURBS(arg1, ndata);
    gluEndSurface(ndata->nobj);
    /* hack for Mesa 3.1 */
#ifdef MESA
    for(gms.len; gms.len>0; gms.len--)
	free(gms.ptr[gms.len-1]);
    free(gms.ptr);
    gms.ptr = NULL;
#endif
    return Qnil;
}


static VALUE
glu_BeginTrim(obj, arg1)
VALUE obj, arg1;
{
    struct nurbsdata *ndata;
    GetNURBS(arg1, ndata);
    gluBeginTrim(ndata->nobj);
    return Qnil;
}
static VALUE
glu_EndTrim(obj, arg1)
VALUE obj, arg1;
{
    struct nurbsdata *ndata;
    GetNURBS(arg1, ndata);
    gluEndTrim(ndata->nobj);
    return Qnil;
}


/*
 * Tesselation API
 */
static VALUE t_current;
#define TESS_DATA   0
#define TESS_BEGIN  1
#define TESS_VERTEX 2
#define TESS_END    3
#define TESS_ERROR  4
#define TESS_EDGE   5
#if defined(GLU_VERSION_1_2)
#  define TESS_OUTDATA     6
#  define TESS_COMBINE     7
#  define TESS_BEGIN_DATA  8
#  define TESS_VERTEX_DATA 9
#  define TESS_END_DATA    10
#  define TESS_ERROR_DATA  11
#  define TESS_EDGE_DATA   12
#  define TESS_COMBINE_DATA    13
#  define TESS_USERDATA   14
#  define REF_LAST    15
#else /* !GLU_VERSION_1_2 */
#  define REF_LAST    6
#endif /* GLU_VERSION_1_2 */
static void
mark_tess(tdata)
struct tessdata* tdata;
{
    if (tdata->tobj)
        rb_gc_mark(tdata->t_ref);
}
static void
free_tess(tdata)
struct tessdata *tdata;
{
    if (tdata->tobj) {
	gluDeleteTess(tdata->tobj);
    }
    tdata->t_ref = Qnil;
    tdata->tobj = NULL;
}
static VALUE
glu_NewTess(obj)
VALUE obj;
{
    VALUE ret;
    struct tessdata *tdata;
    ret = Data_Make_Struct(cTess, struct tessdata, mark_tess, free_tess, tdata);
    tdata->tobj = gluNewTess();
    tdata->t_ref = rb_ary_new2(REF_LAST);
    return ret;
}
static VALUE
glu_DeleteTess(obj, arg1)
VALUE obj, arg1;
{
    struct tessdata *tdata;
    GetTESS(arg1, tdata);
    free_tess(tdata);
    return Qnil;
}
void
t_begin(type)
GLenum type;
{
    VALUE tess;
    struct tessdata *tdata;
    tess = rb_ary_entry(t_current, -1);
    if (tess != Qnil) {
        GetTESS(tess, tdata);
        rb_funcall(rb_ary_entry(tdata->t_ref, TESS_BEGIN), callId, 1, INT2NUM(type));
    }
}
static void
t_edgeFlag(flag)
GLboolean flag;
{
    VALUE tess;
    struct tessdata *tdata;
    tess = rb_ary_entry(t_current, -1);
    if (tess != Qnil) {
        GetTESS(tess, tdata);
        rb_funcall(rb_ary_entry(tdata->t_ref, TESS_EDGE), callId, 1, INT2NUM(flag));
    }
}
static void
t_vertex(data)
void* data;
{
    VALUE tess;
    struct tessdata *tdata;
    tess = rb_ary_entry(t_current, -1);
    if (tess != Qnil) {
        GetTESS(tess, tdata);
        rb_funcall(rb_ary_entry(tdata->t_ref, TESS_VERTEX), callId, 1, data);
    }
}
static void
t_end()
{
    VALUE tess;
    struct tessdata *tdata;
    tess = rb_ary_entry(t_current, -1);
    if (tess != Qnil) {
        GetTESS(tess, tdata);
        rb_funcall(rb_ary_entry(tdata->t_ref, TESS_END), callId, 0);
    }
}
static void
t_error(errorno)
GLenum errorno;
{
    VALUE tess;
    struct tessdata *tdata;
    tess = rb_ary_entry(t_current, -1);
    if (tess != Qnil) {
        GetTESS(tess, tdata);
        rb_funcall(rb_ary_entry(tdata->t_ref, TESS_ERROR), callId, 1, INT2NUM(errorno));
    }
}

#if defined(GLU_VERSION_1_2)
static void
t_begin_data(type, user_data)
GLenum type;
void* user_data;
{
    VALUE tess;
    struct tessdata *tdata;
    tess = rb_ary_entry(t_current, -1);
    if (tess != Qnil) {
        GetTESS(tess, tdata);
        rb_funcall(rb_ary_entry(tdata->t_ref, TESS_BEGIN_DATA), callId, 2, INT2NUM(type), user_data);
    }
}
static void
t_edgeFlag_data(flag, user_data)
GLboolean flag;
void* user_data;
{
    VALUE tess;
    struct tessdata *tdata;
    tess = rb_ary_entry(t_current, -1);
    if (tess != Qnil) {
        GetTESS(tess, tdata);
        rb_funcall(rb_ary_entry(tdata->t_ref, TESS_EDGE_DATA), callId, 2, INT2NUM(flag), user_data);
    }
}
static void
t_vertex_data(data, user_data)
void* data;
void* user_data;
{
    VALUE tess;
    struct tessdata *tdata;
    tess = rb_ary_entry(t_current, -1);
    if (tess != Qnil) {
        GetTESS(tess, tdata);
        rb_funcall(rb_ary_entry(tdata->t_ref, TESS_VERTEX_DATA), callId, 2, data, user_data);
    }
}
static void
t_end_data(user_data)
void* user_data;
{
    VALUE tess;
    struct tessdata *tdata;
    tess = rb_ary_entry(t_current, -1);
    if (tess != Qnil) {
        GetTESS(tess, tdata);
        rb_funcall(rb_ary_entry(tdata->t_ref, TESS_END_DATA), callId, 1, user_data);
    }
}
static void
t_error_data(errorno, user_data)
GLenum errorno;
void* user_data;
{
    VALUE tess;
    struct tessdata *tdata;
    tess = rb_ary_entry(t_current, -1);
    if (tess != Qnil) {
        GetTESS(tess, tdata);
        rb_funcall(rb_ary_entry(tdata->t_ref, TESS_ERROR_DATA), callId, 2, INT2NUM(errorno), user_data);
    }
}
static void
t_combine(coords, vertex_data, weight, outData)
GLdouble coords[3];
void* vertex_data[4];
GLfloat weight[4];
void** outData;
{
    VALUE tess;
    struct tessdata *tdata;
    VALUE rb_coord, rb_vertex_data, rb_weight;
    int i;
    tess = rb_ary_entry(t_current, -1);
    if (tess != Qnil) {
        GetTESS(tess, tdata);
        rb_coord = rb_ary_new2(3);
        for (i = 0; i < 3; i++)
            rb_ary_store(rb_coord, i, rb_float_new(coords[i]));
        rb_vertex_data = rb_ary_new2(4);
        for (i = 0; i < 4; i++)
            rb_ary_store(rb_vertex_data, i, (VALUE)vertex_data[i]);
        rb_weight = rb_ary_new2(4);
        for (i = 0; i < 4; i++)
            rb_ary_store(rb_weight, i, rb_float_new(weight[i]));
        *outData = (void*)rb_funcall(rb_ary_entry(tdata->t_ref, TESS_COMBINE), callId, 3, rb_coord, rb_vertex_data, rb_weight);
        rb_ary_push(rb_ary_entry(tdata->t_ref, TESS_OUTDATA), (VALUE)*outData);
    }
}
static void
t_combine_data(coords, vertex_data, weight, outData, user_data)
GLdouble coords[3];
void* vertex_data[4];
GLfloat weight[4];
void** outData;
void* user_data;
{
    VALUE tess;
    struct tessdata *tdata;
    VALUE rb_coord, rb_vertex_data, rb_weight;
    int i;
    tess = rb_ary_entry(t_current, -1);
    if (tess != Qnil) {
        GetTESS(tess, tdata);
        rb_coord = rb_ary_new2(3);
        for (i = 0; i < 3; i++)
            rb_ary_store(rb_coord, i, rb_float_new(coords[i]));
        rb_vertex_data = rb_ary_new2(4);
        for (i = 0; i < 4; i++)
            rb_ary_store(rb_vertex_data, i, (VALUE)vertex_data[i]);
        rb_weight = rb_ary_new2(4);
        for (i = 0; i < 4; i++)
            rb_ary_store(rb_weight, i, rb_float_new(weight[i]));
        *outData = (void*)rb_funcall(rb_ary_entry(tdata->t_ref, TESS_COMBINE_DATA), callId, 4, rb_coord, rb_vertex_data, rb_weight, (VALUE)user_data);
        rb_ary_push(rb_ary_entry(tdata->t_ref, TESS_OUTDATA), (VALUE)*outData);
    }
}
static VALUE
glu_TessProperty(obj, arg1, arg2, arg3)
VALUE obj, arg1, arg2, arg3;
{
    struct tessdata* tdata;
    GLenum property;
    GLdouble value;
    GetTESS(arg1, tdata);
    property = (GLenum)NUM2INT(arg2);
    value = (GLdouble)NUM2DBL(arg3);
    gluTessProperty(tdata->tobj, property, value);
    return Qnil;
}
static VALUE
glu_GetTessProperty(obj, arg1, arg2)
VALUE obj, arg1, arg2;
{
    struct tessdata* tdata;
    GLenum property;
    GLdouble value;
    GetTESS(arg1, tdata);
    property = (GLenum)NUM2INT(arg2);
    gluGetTessProperty(tdata->tobj, property, &value);
    return rb_float_new(value);
}
static VALUE
glu_TessNormal(obj, arg1, arg2, arg3, arg4)
VALUE obj, arg1, arg2, arg3, arg4;
{
    struct tessdata* tdata;
    GLdouble x, y, z;
    GetTESS(arg1, tdata);
    x = (GLdouble)NUM2DBL(arg2);
    y = (GLdouble)NUM2DBL(arg3);
    z = (GLdouble)NUM2DBL(arg4);
    gluTessNormal(tdata->tobj, x, y, z);
    return Qnil;
}
static VALUE
glu_TessBeginPolygon(obj, arg1, arg2)
VALUE obj, arg1, arg2;
{
    struct tessdata* tdata;
    GetTESS(arg1, tdata);
    rb_ary_store(tdata->t_ref, TESS_USERDATA, arg2);
    rb_ary_store(tdata->t_ref, TESS_OUTDATA, rb_ary_new());
    rb_ary_store(tdata->t_ref, TESS_DATA, rb_ary_new());
    rb_ary_push(t_current, arg1);
    gluTessBeginPolygon(tdata->tobj, (void*)arg2);
    return Qnil;
}
static VALUE
glu_TessEndPolygon(obj, arg1)
VALUE obj, arg1;
{
    struct tessdata* tdata;
    GetTESS(arg1, tdata);
    gluTessEndPolygon(tdata->tobj);
    rb_ary_store(tdata->t_ref, TESS_USERDATA, Qnil);
    rb_ary_store(tdata->t_ref, TESS_OUTDATA, Qnil);
    rb_ary_store(tdata->t_ref, TESS_DATA, Qnil);
    rb_ary_pop(t_current);
    return Qnil;
}
static VALUE
glu_TessBeginContour(obj, arg1)
VALUE obj, arg1;
{
    struct tessdata* tdata;
    GetTESS(arg1, tdata);
    gluTessBeginContour(tdata->tobj);
    return Qnil;
}
static VALUE
glu_TessEndContour(obj, arg1)
VALUE obj,arg1;
{
    struct tessdata* tdata;
    GetTESS(arg1, tdata);
    gluTessEndContour(tdata->tobj);
    return Qnil;
}
#endif /* GLU_VERSION_1_2 */

static VALUE
glu_TessCallback(obj, arg1, arg2, arg3)
VALUE obj, arg1, arg2, arg3;
{
    struct tessdata* tdata;
    GLenum type;
    ID id;
    GetTESS(arg1, tdata);
    type = (GLenum)NUM2INT(arg2);
    id = rb_intern("[]=");
    if (!rb_obj_is_kind_of(arg3,rb_cProc) && NIL_P(arg3))
	 rb_raise(rb_eTypeError, "GLU.TessCallback needs Proc Object:%s",rb_class2name(CLASS_OF(arg3)));
	
    switch (type) {
#if defined(GLU_VERSION_1_2)
	case GLU_TESS_BEGIN:
#else
	case GLU_BEGIN:
#endif
            rb_ary_store(tdata->t_ref, TESS_BEGIN, arg3);
            if (NIL_P(arg3))
                gluTessCallback(tdata->tobj, type, NULL);
            else
                gluTessCallback(tdata->tobj, type, (VOIDFUNC)(t_begin));
	    break;
#if defined(GLU_VERSION_1_2)
	case GLU_TESS_EDGE_FLAG:
#else
	case GLU_EDGE_FLAG:
#endif
            rb_ary_store(tdata->t_ref, TESS_EDGE, arg3);
            if (NIL_P(arg3))
                gluTessCallback(tdata->tobj, type, NULL);
            else
                gluTessCallback(tdata->tobj, type, (VOIDFUNC)t_edgeFlag);
	    break;
#if defined(GLU_VERSION_1_2)
	case GLU_TESS_VERTEX:
#else
	case GLU_VERTEX:
#endif
            rb_ary_store(tdata->t_ref, TESS_VERTEX, arg3);
            if (NIL_P(arg3))
                gluTessCallback(tdata->tobj, type, NULL);
            else
                gluTessCallback(tdata->tobj, type, (VOIDFUNC)t_vertex);
	    break;
#if defined(GLU_VERSION_1_2)
	case GLU_TESS_END:
#else
	case GLU_END:
#endif
            rb_ary_store(tdata->t_ref, TESS_END, arg3);
            if (NIL_P(arg3))
                gluTessCallback(tdata->tobj, type, NULL);
            else
                gluTessCallback(tdata->tobj, type, (VOIDFUNC)t_end);
	    break;
#if defined(GLU_VERSION_1_2)
	case GLU_TESS_ERROR:
#else
	case GLU_ERROR:
#endif
            rb_ary_store(tdata->t_ref, TESS_ERROR, arg3);
            if (NIL_P(arg3))
                gluTessCallback(tdata->tobj, type, NULL);
            else
                gluTessCallback(tdata->tobj, type, (VOIDFUNC)t_error);
	    break;
#if defined(GLU_VERSION_1_2)
        case GLU_TESS_COMBINE:
            rb_ary_store(tdata->t_ref, TESS_COMBINE, arg3);
            if (NIL_P(arg3))
                gluTessCallback(tdata->tobj, type, NULL);
            else
                gluTessCallback(tdata->tobj, type, (VOIDFUNC)(t_combine));
            break;
        case GLU_TESS_BEGIN_DATA:
            rb_ary_store(tdata->t_ref, TESS_BEGIN_DATA, arg3);
            if (NIL_P(arg3))
                gluTessCallback(tdata->tobj, type, NULL);
            else
                gluTessCallback(tdata->tobj, type, (VOIDFUNC)(t_begin_data));
            break;
        case GLU_TESS_VERTEX_DATA:
            rb_ary_store(tdata->t_ref, TESS_VERTEX_DATA, arg3);
            if (NIL_P(arg3))
                gluTessCallback(tdata->tobj, type, NULL);
            else
                gluTessCallback(tdata->tobj, type, (VOIDFUNC)(t_vertex_data));
            break;
        case GLU_TESS_END_DATA:
            rb_ary_store(tdata->t_ref, TESS_END_DATA, arg3);
            if (NIL_P(arg3))
                gluTessCallback(tdata->tobj, type, NULL);
            else
                gluTessCallback(tdata->tobj, type, (VOIDFUNC)(t_end_data));
            break;
        case GLU_TESS_ERROR_DATA:
            rb_ary_store(tdata->t_ref, TESS_ERROR_DATA, arg3);
            if (NIL_P(arg3))
                gluTessCallback(tdata->tobj, type, NULL);
            else
                gluTessCallback(tdata->tobj, type, (VOIDFUNC)(t_error_data));
            break;
        case GLU_TESS_EDGE_FLAG_DATA:
            rb_ary_store(tdata->t_ref, TESS_EDGE_DATA, arg3);
            if (NIL_P(arg3))
                gluTessCallback(tdata->tobj, type, NULL);
            else
                gluTessCallback(tdata->tobj, type, (VOIDFUNC)(t_edgeFlag_data));
            break;
        case GLU_TESS_COMBINE_DATA:
            rb_ary_store(tdata->t_ref, TESS_COMBINE_DATA, arg3);
            if (NIL_P(arg3))
                gluTessCallback(tdata->tobj, type, NULL);
            else
                gluTessCallback(tdata->tobj, type, (VOIDFUNC)(t_combine_data));
            break;
#endif /* GLU_VERSION_1_2 */
    }
    return Qnil;
}
static VALUE
glu_BeginPolygon(obj, arg1)
VALUE obj, arg1;
{
    struct tessdata* tdata;
    GetTESS(arg1, tdata);
    rb_ary_store(tdata->t_ref, TESS_DATA, rb_ary_new());
    rb_ary_push(t_current, arg1);
    gluBeginPolygon(tdata->tobj);
    return Qnil;
}
static VALUE
glu_TessVertex(obj, arg1, arg2, arg3)
VALUE obj, arg1, arg2, arg3;
{
    struct tessdata* tdata;
    GLdouble v[3];
   GetTESS(arg1, tdata);
    rb_ary_push(rb_ary_entry(tdata->t_ref, TESS_DATA), arg3);
    ary2cdbl(arg2, v, 3);
    gluTessVertex(tdata->tobj, v,(void *)arg3);
    return Qnil;
}
static VALUE
glu_NextContour(obj, arg1, arg2)
VALUE obj, arg1, arg2;
{
    struct tessdata* tdata;
    GLenum type;
    GetTESS(arg1, tdata);
    type = (GLenum)NUM2INT(arg2);
    gluNextContour(tdata->tobj, type);
    return Qnil;
}
static VALUE
glu_EndPolygon(obj, arg1)
VALUE obj, arg1;
{
    struct tessdata* tdata;
    GetTESS(arg1, tdata);
    gluEndPolygon(tdata->tobj);
    rb_ary_store(tdata->t_ref, TESS_DATA, Qnil);
    rb_ary_pop(t_current);
    return Qnil;
}

/*
 * Quadric API
 */
static void
free_quad(qdata)
struct quaddata *qdata;
{
  if (qdata->qobj) gluDeleteQuadric(qdata->qobj);
  qdata->qobj = NULL;
}
static VALUE
glu_NewQuadric(obj)
VALUE obj;
{
    VALUE ret;
    struct quaddata *qdata;
    ret = Data_Make_Struct(cQuad, struct quaddata, 0, free_quad, qdata);
    qdata->qobj = gluNewQuadric();
    return ret;
}
static VALUE
glu_DeleteQuadric(obj, arg1)
VALUE obj, arg1;
{
    struct quaddata *qdata;
    GetQUAD(arg1, qdata);
    free_quad(qdata);
    return Qnil;
}
static VALUE
glu_QuadricNormals(obj, arg1, arg2)
VALUE obj, arg1, arg2;
{
    struct quaddata* qdata;
    GLenum normals;
    GetQUAD(arg1, qdata);
    normals = (GLenum)NUM2INT(arg2);
    gluQuadricNormals(qdata->qobj, normals);
    return Qnil;
}
static VALUE
glu_QuadricTexture(obj, arg1, arg2)
VALUE obj, arg1, arg2;
{
    struct quaddata* qdata;
    GLboolean textureCoords;
    GetQUAD(arg1, qdata);
    textureCoords = (GLboolean)NUM2INT(arg2);
    gluQuadricTexture(qdata->qobj, textureCoords);
    return Qnil;
}
static VALUE
glu_QuadricOrientation(obj, arg1, arg2)
VALUE obj, arg1, arg2;
{
    struct quaddata* qdata;
    GLenum orientation;
    GetQUAD(arg1, qdata);
    orientation = (GLenum)NUM2INT(arg2);
    gluQuadricOrientation(qdata->qobj, orientation);
    return Qnil;
}
static VALUE
glu_QuadricDrawStyle(obj, arg1, arg2)
VALUE obj, arg1, arg2;
{
    struct quaddata* qdata;
    GLenum drawStyle;
    GetQUAD(arg1, qdata);
    drawStyle = (GLenum)NUM2INT(arg2);
    gluQuadricDrawStyle(qdata->qobj, drawStyle);
    return Qnil;
}
static VALUE
glu_Cylinder(obj, arg1, arg2, arg3, arg4, arg5, arg6)
VALUE obj, arg1, arg2, arg3, arg4, arg5, arg6;
{
    struct quaddata* qdata;
    GLdouble baseRadius;
    GLdouble topRadius;
    GLdouble height;
    GLint slices;
    GLint stacks;

    GetQUAD(arg1, qdata);
    baseRadius = (GLdouble)NUM2DBL(arg2);
    topRadius = (GLdouble)NUM2DBL(arg3);
    height = (GLdouble)NUM2DBL(arg4);
    slices = (GLint)NUM2INT(arg5);
    stacks = (GLint)NUM2INT(arg6);

    gluCylinder(qdata->qobj, baseRadius, topRadius, height, slices, stacks);
    return Qnil;
}
static VALUE
glu_Disk(obj, arg1, arg2, arg3, arg4, arg5)
VALUE obj, arg1, arg2, arg3, arg4, arg5;
{
    struct quaddata* qdata;
    GLdouble innerRadius;
    GLdouble outerRadius;
    GLint slices;
    GLint loops;

    GetQUAD(arg1, qdata);
    innerRadius = (GLdouble)NUM2DBL(arg2);
    outerRadius = (GLdouble)NUM2DBL(arg3);
    slices = (GLint)NUM2INT(arg4);
    loops = (GLint)NUM2INT(arg5);

    gluDisk(qdata->qobj, innerRadius, outerRadius, slices, loops);
    return Qnil;
}
static VALUE
glu_PartialDisk(obj, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
VALUE obj, arg1, arg2, arg3, arg4, arg5, arg6, arg7;
{
    struct quaddata* qdata;
    GLdouble innerRadius;
    GLdouble outerRadius;
    GLint slices;
    GLint loops;
    GLdouble startAngle;
    GLdouble sweepAngle;

    GetQUAD(arg1, qdata);
    innerRadius = (GLdouble)NUM2DBL(arg2);
    outerRadius = (GLdouble)NUM2DBL(arg3);
    slices = (GLint)NUM2INT(arg4);
    loops = (GLint)NUM2INT(arg5);
    startAngle = (GLdouble)NUM2DBL(arg6);
    sweepAngle = (GLdouble)NUM2DBL(arg7);

    gluPartialDisk(qdata->qobj, innerRadius, outerRadius, slices, loops, startAngle, sweepAngle);
    return Qnil;
}
static VALUE
glu_Sphere(obj, arg1, arg2, arg3, arg4)
VALUE obj, arg1, arg2, arg3, arg4;
{
    struct quaddata* qdata;
    GLdouble radius;
    GLint slices;
    GLint stacks;

    GetQUAD(arg1, qdata);
    radius = (GLdouble)NUM2DBL(arg2);
    slices = (GLint)NUM2INT(arg3);
    stacks = (GLint)NUM2INT(arg4);

    gluSphere(qdata->qobj, radius, slices, stacks);
    return Qnil;
}
static VALUE
glu_LookAt(obj,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9)
VALUE obj,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9;
{
    GLdouble eyex;
    GLdouble eyey;
    GLdouble eyez;
    GLdouble centerx;
    GLdouble centery;
    GLdouble centerz;
    GLdouble upx;
    GLdouble upy;
    GLdouble upz;
    eyex = (GLdouble)NUM2DBL(arg1);
    eyey = (GLdouble)NUM2DBL(arg2);
    eyez = (GLdouble)NUM2DBL(arg3);
    centerx = (GLdouble)NUM2DBL(arg4);
    centery = (GLdouble)NUM2DBL(arg5);
    centerz = (GLdouble)NUM2DBL(arg6);
    upx = (GLdouble)NUM2DBL(arg7);
    upy = (GLdouble)NUM2DBL(arg8);
    upz = (GLdouble)NUM2DBL(arg9);
    gluLookAt( eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz );
    return Qnil;
}
static VALUE
glu_Ortho2D(obj,arg1,arg2,arg3,arg4)
VALUE obj,arg1,arg2,arg3,arg4;
{
    GLdouble left;
    GLdouble right;
    GLdouble bottom;
    GLdouble top;
    left = (GLdouble)NUM2DBL(arg1);
    right = (GLdouble)NUM2DBL(arg2);
    bottom = (GLdouble)NUM2DBL(arg3);
    top = (GLdouble)NUM2DBL(arg4);
    gluOrtho2D(left,right,bottom,top);
    return Qnil;
}
static VALUE
glu_Perspective(obj,arg1,arg2,arg3,arg4)
VALUE obj,arg1,arg2,arg3,arg4;
{
    GLdouble fovy;
    GLdouble aspect;
    GLdouble zNear;
    GLdouble zFar;
    fovy = (GLdouble)NUM2DBL(arg1);
    aspect = (GLdouble)NUM2DBL(arg2);
    zNear = (GLdouble)NUM2DBL(arg3);
    zFar = (GLdouble)NUM2DBL(arg4);
    gluPerspective(fovy,aspect,zNear,zFar);
    return Qnil;
}
static VALUE
glu_PickMatrix(argc,argv,obj)
int argc;
VALUE* argv;
VALUE obj;
{
    GLdouble x;
    GLdouble y;
    GLdouble width;
    GLdouble height;
    GLint viewport[4];

    VALUE args[5];

    switch (rb_scan_args(argc, argv, "23", &args[0], &args[1], &args[2], &args[3], &args[4])) {
        case 2:
	    width = 5.0f;
	    height = 5.0f;
	    glGetIntegerv(GL_VIEWPORT, viewport);
	    break;
	case 4:
	    width = (GLdouble)NUM2DBL(args[2]);
	    height = (GLdouble)NUM2DBL(args[3]);
	    glGetIntegerv(GL_VIEWPORT, viewport);
	    break;
        case 5:
	    width = (GLdouble)NUM2DBL(args[2]);
	    height = (GLdouble)NUM2DBL(args[3]);
	    ary2cint(args[4], (int *)viewport, 4);
	    break;
        default:
	    rb_raise(rb_eArgError, "GLU.PickMatrix args len:%d",argc);
    }
    x = (GLdouble)NUM2DBL(args[0]);
    y = (GLdouble)NUM2DBL(args[1]);
    gluPickMatrix(x, y, width, height, viewport);
    return Qnil;
}

static VALUE
glu_Project(argc,argv,obj)
int argc;
VALUE* argv;
VALUE obj;
{
    GLdouble ox;
    GLdouble oy;
    GLdouble oz;
    GLdouble mdl_mtx[4*4];
    GLdouble prj_mtx[4*4];
    GLint vport[4];
    GLdouble wx;
    GLdouble wy;
    GLdouble wz;

    VALUE args[6];
    VALUE ret;
    
    switch (rb_scan_args(argc, argv, "33", &args[0], &args[1], &args[2], &args[3], &args[4], &args[5])) {
	case 3:
	    glGetDoublev(GL_MODELVIEW_MATRIX, mdl_mtx);
	    glGetDoublev(GL_PROJECTION_MATRIX, prj_mtx);
	    glGetIntegerv(GL_VIEWPORT, vport);
	    break;
	case 6:
	    ary2cmat4x4(args[3], mdl_mtx);
	    ary2cmat4x4(args[4], prj_mtx);
	    ary2cint(args[5], (int *)vport, 4);
	    break;
	default:
	    rb_raise(rb_eArgError, "GLU.Project args len:%d",argc);
    }
    ox = (GLdouble)NUM2DBL(args[0]);
    oy = (GLdouble)NUM2DBL(args[1]);
    oz = (GLdouble)NUM2DBL(args[2]);

    if (gluProject(ox, oy, oz, mdl_mtx, prj_mtx, vport, &wx, &wy, &wz)
	== GL_TRUE) {
	ret = rb_ary_new3(3, rb_float_new(wx), rb_float_new(wy), rb_float_new(wz));
	return ret;
    }
    else
	return Qnil;
}
static VALUE
glu_UnProject(argc,argv,obj)
int argc;
VALUE* argv;
VALUE obj;
{
    GLdouble wx;
    GLdouble wy;
    GLdouble wz;
    GLdouble mdl_mtx[4*4];
    GLdouble prj_mtx[4*4];
    GLint vport[4];
    GLdouble ox;
    GLdouble oy;
    GLdouble oz;

    VALUE args[6];
    VALUE ret;

    switch (rb_scan_args(argc, argv, "33", &args[0], &args[1], &args[2], &args[3], &args[4], &args[5])) {
	case 3:
	    glGetDoublev(GL_MODELVIEW_MATRIX, mdl_mtx);
	    glGetDoublev(GL_PROJECTION_MATRIX, prj_mtx);
	    glGetIntegerv(GL_VIEWPORT, vport);
	    break;
	case 6:
	    ary2cmat4x4(args[3], mdl_mtx);
	    ary2cmat4x4(args[4], prj_mtx);
	    ary2cint(args[5], (int *)vport, 4);
	    break;
	default:
	    rb_raise(rb_eArgError, "GLU.UnProject args len:%d",argc);
    }
    wx = (GLdouble)NUM2DBL(args[0]);
    wy = (GLdouble)NUM2DBL(args[1]);
    wz = (GLdouble)NUM2DBL(args[2]);

    if (gluUnProject(wx, wy, wz, mdl_mtx, prj_mtx, vport, &ox, &oy, &oz)
	== GL_TRUE) {
        ret = rb_ary_new3(3, rb_float_new(ox), rb_float_new(oy), rb_float_new(oz));
	return ret;
    }
    else
	return Qnil;
}

static VALUE
glu_Build2DMipmaps(obj, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
VALUE obj, arg1, arg2, arg3, arg4, arg5, arg6, arg7;
{
    GLenum target;
    GLint components;
    GLint width;
    GLint height;
    GLenum format;
    GLenum type;
    void* data;

    int type_size;
    int format_size;
    int size;

    target = (GLenum)NUM2INT(arg1);
    components = (GLint)NUM2INT(arg2);
    width = (GLint)NUM2INT(arg3);
    height = (GLint)NUM2INT(arg4);
    format = (GLenum)NUM2INT(arg5);
    type = (GLenum)NUM2INT(arg6);
    if (TYPE(arg7) == T_STRING) {
        type_size = gltype_size(type) / 8;
        format_size = glformat_size(format);
        if (type_size == -1 || format_size == -1)
            return Qnil;
        size = type_size*format_size*height*width;
        if (RSTRING_LEN(arg7) < size)
            rb_raise(rb_eArgError, "string length:%d",RSTRING_LEN(arg7));
        data = RSTRING_PTR(arg7);
    } else 
        rb_raise(rb_eTypeError, "type mismatch:%s",rb_class2name(arg7));
    return INT2NUM(gluBuild2DMipmaps(target, components, width, height, format, type, data));
}


static VALUE
glu_ErrorString(obj, arg1)
VALUE obj, arg1;
{
    GLenum errorCode;
    GLubyte* error;
    errorCode = (GLenum)NUM2INT(arg1);
    error = (GLubyte*)gluErrorString(errorCode);
    if (error)
        return rb_str_new2((char *)error);
    else
        return Qnil;
}
#if defined(GLU_VERSION_1_1)
static VALUE
glu_GetString(obj, arg1)
VALUE obj, arg1;
{
    GLenum name;
    GLubyte* str;
    name = (GLenum)NUM2INT(arg1);
    str = (GLubyte*)gluGetString(name);
    if (str)
        return rb_str_new2((char *)str);
    else
        return Qnil;
}
#endif

static VALUE mGLU;

void
InitializeGLU()
{
    callId = rb_intern("call");
    refId = rb_intern("[]");
    mGLU = rb_define_module("GLU");
    rb_define_module_function(mGLU, "NewNurbsRenderer", glu_NewNurbsRenderer, 0);
    rb_define_module_function(mGLU, "DeleteNurbsRenderer", glu_DeleteNurbsRenderer, 1);
    rb_define_module_function(mGLU, "NurbsProperty", glu_NurbsProperty, 3);
    rb_define_module_function(mGLU, "GetNurbsProperty", glu_GetNurbsProperty, 2);
    rb_define_module_function(mGLU, "BeginCurve", glu_BeginCurve, 1);
    rb_define_module_function(mGLU, "EndCurve", glu_EndCurve, 1);
    rb_define_module_function(mGLU, "BeginSurface", glu_BeginSurface, 1);
    rb_define_module_function(mGLU, "EndSurface", glu_EndSurface, 1);
    rb_define_module_function(mGLU, "BeginTrim", glu_BeginTrim, 1);
    rb_define_module_function(mGLU, "EndTrim", glu_EndTrim, 1);
    rb_define_module_function(mGLU, "NewTess", glu_NewTess, 0);
    rb_define_module_function(mGLU, "DeleteTess", glu_DeleteTess, 1);
    rb_define_module_function(mGLU, "TessCallback", glu_TessCallback, 3);
    rb_define_module_function(mGLU, "BeginPolygon", glu_BeginPolygon, 1);
    rb_define_module_function(mGLU, "TessVertex", glu_TessVertex, 3);
    rb_define_module_function(mGLU, "NextContour", glu_NextContour, 2);
    rb_define_module_function(mGLU, "EndPolygon", glu_EndPolygon, 1);
#if defined(GLU_VERSION_1_2)
    rb_define_module_function(mGLU, "TessBeginPolygon", glu_TessBeginPolygon, 2);
    rb_define_module_function(mGLU, "TessBeginContour",  glu_TessBeginContour, 1);
    rb_define_module_function(mGLU, "TessEndContour", glu_TessEndContour, 1);
    rb_define_module_function(mGLU, "TessEndPolygon", glu_TessEndPolygon, 1);
    rb_define_module_function(mGLU, "TessProperty", glu_TessProperty, 3);
    rb_define_module_function(mGLU, "TessNormal", glu_TessNormal, 4);
    rb_define_module_function(mGLU, "GetTessProperty", glu_GetTessProperty, 2);
#endif /* GLU_VERSION_1_2 */
    rb_define_module_function(mGLU, "NewQuadric", glu_NewQuadric, 0);
    rb_define_module_function(mGLU, "DeleteQuadric", glu_DeleteQuadric, 1);
    rb_define_module_function(mGLU, "QuadricNormals", glu_QuadricNormals, 2);
    rb_define_module_function(mGLU, "QuadricTexture", glu_QuadricTexture, 2);
    rb_define_module_function(mGLU, "QuadricOrientation", glu_QuadricOrientation, 2);
    rb_define_module_function(mGLU, "QuadricDrawStyle", glu_QuadricDrawStyle, 2);
    rb_define_module_function(mGLU, "Cylinder", glu_Cylinder, 6);
    rb_define_module_function(mGLU, "Disk", glu_Disk, 5);
    rb_define_module_function(mGLU, "PartialDisk", glu_PartialDisk, 7);
    rb_define_module_function(mGLU, "Sphere", glu_Sphere, 4);

    rb_define_module_function(mGLU, "LookAt", glu_LookAt, 9);
    rb_define_module_function(mGLU, "Ortho2D", glu_Ortho2D, 4);
    rb_define_module_function(mGLU, "Perspective", glu_Perspective, 4);
    rb_define_module_function(mGLU, "PickMatrix", glu_PickMatrix, -1);
    rb_define_module_function(mGLU, "Project", glu_Project, -1);
    rb_define_module_function(mGLU, "UnProject", glu_UnProject, -1);
    rb_define_module_function(mGLU, "Build2DMipmaps", glu_Build2DMipmaps, 7);
    rb_define_module_function(mGLU, "ErrorString", glu_ErrorString, 1);
#if defined(GLU_VERSION_1_1)
    rb_define_module_function(mGLU, "GetString", glu_GetString, 1);
#endif

    rb_define_const(mGLU, "SMOOTH", INT2NUM(GLU_SMOOTH));
    rb_define_const(mGLU, "FLAT", INT2NUM(GLU_FLAT));
    rb_define_const(mGLU, "NONE", INT2NUM(GLU_NONE));
    rb_define_const(mGLU, "POINT", INT2NUM(GLU_POINT));
    rb_define_const(mGLU, "LINE", INT2NUM(GLU_LINE));
    rb_define_const(mGLU, "FILL", INT2NUM(GLU_FILL));
    rb_define_const(mGLU, "SILHOUETTE", INT2NUM(GLU_SILHOUETTE));
    rb_define_const(mGLU, "OUTSIDE", INT2NUM(GLU_OUTSIDE));
    rb_define_const(mGLU, "INSIDE", INT2NUM(GLU_INSIDE));
#if defined(GLU_VERSION_1_2)
    rb_define_const(mGLU, "TESS_BEGIN", INT2NUM(GLU_TESS_BEGIN));
    rb_define_const(mGLU, "TESS_VERTEX", INT2NUM(GLU_TESS_VERTEX));
    rb_define_const(mGLU, "TESS_END", INT2NUM(GLU_TESS_END));
    rb_define_const(mGLU, "TESS_ERROR", INT2NUM(GLU_TESS_ERROR));
    rb_define_const(mGLU, "TESS_EDGE_FLAG", INT2NUM(GLU_TESS_EDGE_FLAG));
    rb_define_const(mGLU, "TESS_COMBINE", INT2NUM(GLU_TESS_COMBINE));

    rb_define_const(mGLU, "TESS_BEGIN_DATA", INT2NUM(GLU_TESS_BEGIN_DATA));
    rb_define_const(mGLU, "TESS_VERTEX_DATA", INT2NUM(GLU_TESS_VERTEX_DATA));
    rb_define_const(mGLU, "TESS_END_DATA", INT2NUM(GLU_TESS_END_DATA));
    rb_define_const(mGLU, "TESS_ERROR_DATA", INT2NUM(GLU_TESS_ERROR_DATA));
    rb_define_const(mGLU, "TESS_EDGE_FLAG_DATA", INT2NUM(GLU_TESS_EDGE_FLAG_DATA));
    rb_define_const(mGLU, "TESS_COMBINE_DATA", INT2NUM(GLU_TESS_COMBINE_DATA));

    /* Winding rules */
    rb_define_const(mGLU, "TESS_WINDING_ODD", INT2NUM(GLU_TESS_WINDING_ODD));
    rb_define_const(mGLU, "TESS_WINDING_NONZERO", INT2NUM(GLU_TESS_WINDING_NONZERO));
    rb_define_const(mGLU, "TESS_WINDING_POSITIVE", INT2NUM(GLU_TESS_WINDING_POSITIVE));
    rb_define_const(mGLU, "TESS_WINDING_NEGATIVE", INT2NUM(GLU_TESS_WINDING_NEGATIVE));
    rb_define_const(mGLU, "TESS_WINDING_ABS_GEQ_TWO", INT2NUM(GLU_TESS_WINDING_ABS_GEQ_TWO));

    /* Tessellation properties */
    rb_define_const(mGLU, "TESS_WINDING_RULE", INT2NUM(GLU_TESS_WINDING_RULE));
    rb_define_const(mGLU, "TESS_BOUNDARY_ONLY", INT2NUM(GLU_TESS_BOUNDARY_ONLY));
    rb_define_const(mGLU, "TESS_TOLERANCE", INT2NUM(GLU_TESS_TOLERANCE));
#endif /* GLU_VERSION_1_2 */

    rb_define_const(mGLU, "BEGIN", INT2NUM(GLU_BEGIN));
    rb_define_const(mGLU, "VERTEX", INT2NUM(GLU_VERTEX));
    rb_define_const(mGLU, "END", INT2NUM(GLU_END));
    rb_define_const(mGLU, "ERROR", INT2NUM(GLU_ERROR));
    rb_define_const(mGLU, "EDGE_FLAG", INT2NUM(GLU_EDGE_FLAG));
    rb_define_const(mGLU, "CW", INT2NUM(GLU_CW));
    rb_define_const(mGLU, "CCW", INT2NUM(GLU_CCW));
    rb_define_const(mGLU, "INTERIOR", INT2NUM(GLU_INTERIOR));
    rb_define_const(mGLU, "EXTERIOR", INT2NUM(GLU_EXTERIOR));
    rb_define_const(mGLU, "UNKNOWN", INT2NUM(GLU_UNKNOWN));
    rb_define_const(mGLU, "TESS_ERROR1", INT2NUM(GLU_TESS_ERROR1));
    rb_define_const(mGLU, "TESS_ERROR2", INT2NUM(GLU_TESS_ERROR2));
    rb_define_const(mGLU, "TESS_ERROR3", INT2NUM(GLU_TESS_ERROR3));
    rb_define_const(mGLU, "TESS_ERROR4", INT2NUM(GLU_TESS_ERROR4));
    rb_define_const(mGLU, "TESS_ERROR5", INT2NUM(GLU_TESS_ERROR5));
    rb_define_const(mGLU, "TESS_ERROR6", INT2NUM(GLU_TESS_ERROR6));
    rb_define_const(mGLU, "TESS_ERROR7", INT2NUM(GLU_TESS_ERROR7));
    rb_define_const(mGLU, "TESS_ERROR8", INT2NUM(GLU_TESS_ERROR8));
#if defined(TESS_ERROR9)
    rb_define_const(mGLU, "TESS_ERROR9", INT2NUM(GLU_TESS_ERROR9));
#endif
    rb_define_const(mGLU, "AUTO_LOAD_MATRIX", INT2NUM(GLU_AUTO_LOAD_MATRIX));
    rb_define_const(mGLU, "CULLING", INT2NUM(GLU_CULLING));
    rb_define_const(mGLU, "SAMPLING_TOLERANCE", INT2NUM(GLU_SAMPLING_TOLERANCE));
    rb_define_const(mGLU, "DISPLAY_MODE", INT2NUM(GLU_DISPLAY_MODE));
    rb_define_const(mGLU, "SAMPLING_METHOD", INT2NUM(GLU_SAMPLING_METHOD));
    rb_define_const(mGLU, "U_STEP", INT2NUM(GLU_U_STEP));
    rb_define_const(mGLU, "V_STEP", INT2NUM(GLU_V_STEP));
    rb_define_const(mGLU, "PATH_LENGTH", INT2NUM(GLU_PATH_LENGTH));
    rb_define_const(mGLU, "PARAMETRIC_ERROR", INT2NUM(GLU_PARAMETRIC_ERROR));
    rb_define_const(mGLU, "DOMAIN_DISTANCE", INT2NUM(GLU_DOMAIN_DISTANCE));
    rb_define_const(mGLU, "MAP1_TRIM_2", INT2NUM(GLU_MAP1_TRIM_2));
    rb_define_const(mGLU, "MAP1_TRIM_3", INT2NUM(GLU_MAP1_TRIM_3));
    rb_define_const(mGLU, "OUTLINE_POLYGON", INT2NUM(GLU_OUTLINE_POLYGON));
    rb_define_const(mGLU, "OUTLINE_PATCH", INT2NUM(GLU_OUTLINE_PATCH));
    rb_define_const(mGLU, "NURBS_ERROR1", INT2NUM(GLU_NURBS_ERROR1));
    rb_define_const(mGLU, "NURBS_ERROR2", INT2NUM(GLU_NURBS_ERROR2));
    rb_define_const(mGLU, "NURBS_ERROR3", INT2NUM(GLU_NURBS_ERROR3));
    rb_define_const(mGLU, "NURBS_ERROR4", INT2NUM(GLU_NURBS_ERROR4));
    rb_define_const(mGLU, "NURBS_ERROR5", INT2NUM(GLU_NURBS_ERROR5));
    rb_define_const(mGLU, "NURBS_ERROR6", INT2NUM(GLU_NURBS_ERROR6));
    rb_define_const(mGLU, "NURBS_ERROR7", INT2NUM(GLU_NURBS_ERROR7));
    rb_define_const(mGLU, "NURBS_ERROR8", INT2NUM(GLU_NURBS_ERROR8));
    rb_define_const(mGLU, "NURBS_ERROR9", INT2NUM(GLU_NURBS_ERROR9));
    rb_define_const(mGLU, "NURBS_ERROR10", INT2NUM(GLU_NURBS_ERROR10));
    rb_define_const(mGLU, "NURBS_ERROR11", INT2NUM(GLU_NURBS_ERROR11));
    rb_define_const(mGLU, "NURBS_ERROR12", INT2NUM(GLU_NURBS_ERROR12));
    rb_define_const(mGLU, "NURBS_ERROR13", INT2NUM(GLU_NURBS_ERROR13));
    rb_define_const(mGLU, "NURBS_ERROR14", INT2NUM(GLU_NURBS_ERROR14));
    rb_define_const(mGLU, "NURBS_ERROR15", INT2NUM(GLU_NURBS_ERROR15));
    rb_define_const(mGLU, "NURBS_ERROR16", INT2NUM(GLU_NURBS_ERROR16));
    rb_define_const(mGLU, "NURBS_ERROR17", INT2NUM(GLU_NURBS_ERROR17));
    rb_define_const(mGLU, "NURBS_ERROR18", INT2NUM(GLU_NURBS_ERROR18));
    rb_define_const(mGLU, "NURBS_ERROR19", INT2NUM(GLU_NURBS_ERROR19));
    rb_define_const(mGLU, "NURBS_ERROR20", INT2NUM(GLU_NURBS_ERROR20));
    rb_define_const(mGLU, "NURBS_ERROR21", INT2NUM(GLU_NURBS_ERROR21));
    rb_define_const(mGLU, "NURBS_ERROR22", INT2NUM(GLU_NURBS_ERROR22));
    rb_define_const(mGLU, "NURBS_ERROR23", INT2NUM(GLU_NURBS_ERROR23));
    rb_define_const(mGLU, "NURBS_ERROR24", INT2NUM(GLU_NURBS_ERROR24));
    rb_define_const(mGLU, "NURBS_ERROR25", INT2NUM(GLU_NURBS_ERROR25));
    rb_define_const(mGLU, "NURBS_ERROR26", INT2NUM(GLU_NURBS_ERROR26));
    rb_define_const(mGLU, "NURBS_ERROR27", INT2NUM(GLU_NURBS_ERROR27));
    rb_define_const(mGLU, "NURBS_ERROR28", INT2NUM(GLU_NURBS_ERROR28));
    rb_define_const(mGLU, "NURBS_ERROR29", INT2NUM(GLU_NURBS_ERROR29));
    rb_define_const(mGLU, "NURBS_ERROR30", INT2NUM(GLU_NURBS_ERROR30));
    rb_define_const(mGLU, "NURBS_ERROR31", INT2NUM(GLU_NURBS_ERROR31));
    rb_define_const(mGLU, "NURBS_ERROR32", INT2NUM(GLU_NURBS_ERROR32));
    rb_define_const(mGLU, "NURBS_ERROR33", INT2NUM(GLU_NURBS_ERROR33));
    rb_define_const(mGLU, "NURBS_ERROR34", INT2NUM(GLU_NURBS_ERROR34));
    rb_define_const(mGLU, "NURBS_ERROR35", INT2NUM(GLU_NURBS_ERROR35));
    rb_define_const(mGLU, "NURBS_ERROR36", INT2NUM(GLU_NURBS_ERROR36));
    rb_define_const(mGLU, "NURBS_ERROR37", INT2NUM(GLU_NURBS_ERROR37));
    rb_define_const(mGLU, "INVALID_ENUM", INT2NUM(GLU_INVALID_ENUM));
    rb_define_const(mGLU, "INVALID_VALUE", INT2NUM(GLU_INVALID_VALUE));
    rb_define_const(mGLU, "OUT_OF_MEMORY", INT2NUM(GLU_OUT_OF_MEMORY));
#ifdef GLU_INCOMPATIBLE_GL_VERSION
    rb_define_const(mGLU, "INCOMPATIBLE_GL_VERSION", INT2NUM(GLU_INCOMPATIBLE_GL_VERSION));
#endif
    rb_define_const(mGLU, "VERSION", INT2NUM(GLU_VERSION));
    rb_define_const(mGLU, "EXTENSIONS", INT2NUM(GLU_EXTENSIONS));

    cNurbs = rb_define_class("Nurbs", rb_cObject);
    cTess = rb_define_class("Tess", rb_cObject);
    cQuad = rb_define_class("Quadric", rb_cObject);

    rb_global_variable(&t_current);
    t_current = rb_ary_new();
}
