//
//  VectorMath.h
//

/*
 * Copyright (c) 2005-2008 Doemoetoer Gulyas
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import <math.h>

#if defined(VMATH_FLOAT_TYPE_IS_DOUBLE)
    #define VMATH_FLOAT_TYPE double
	#undef VMATH_USE_ALTIVEC
#endif

#if defined(VMATH_FLOAT_TYPE_IS_FLOAT)
    #define VMATH_FLOAT_TYPE float
	#ifdef __VEC__
//		#define VMATH_USE_ALTIVEC
	#endif
#endif

#if !defined(VMATH_FLOAT_TYPE)
    #define VMATH_FLOAT_TYPE double
	#define VMATH_FLOAT_TYPE_IS_DOUBLE 1
	#undef VMATH_USE_ALTIVEC
#endif

#if !defined(MIN)
    #define MIN(A,B)	((A) < (B) ? (A) : (B))
#endif

#if !defined(MAX)
    #define MAX(A,B)	((A) > (B) ? (A) : (B))
#endif

#if !defined(ABS)
    #define ABS(A)	((A) < 0 ? (-(A)) : (A))
#endif

#define CLAMP(a, b, c)	(MAX((b),MIN((c),(a))))

typedef VMATH_FLOAT_TYPE vmfloat_t;

typedef struct _vnamed
{
	vmfloat_t x,y,z,w;
} vnamed_t;

typedef union _vt
{
#ifdef VMATH_USE_ALTIVEC
	vector vmfloat_t	vec;
#endif
	vmfloat_t			farr[4];
	vnamed_t		named;
} vector_t;

typedef struct _mnamed
{
	vector_t X, Y, Z, T;
} mnamed_t;

typedef union _mt
{
	vector_t			vtarr[4];
#ifdef VMATH_USE_ALTIVEC
	vector vmfloat_t	varr[4];
#endif
	vmfloat_t			farr[16];
	vmfloat_t			fmat[4][4];
	mnamed_t			named;
} matrix_t;

typedef struct _rt
{
	vector_t	minv;
	vector_t	maxv;
} range3d_t;


#ifdef VMATH_USE_ALTIVEC
static inline vector vmfloat_t vec_zero( void )
{
	return vec_ctf( vec_splat_u32(0), 0);
};
#endif

static inline vector_t vZero( void )
{
#ifdef VMATH_USE_ALTIVEC
	return (vector_t){.vec=vec_zero()};
#else
	return (vector_t){.farr={(vmfloat_t)0.0, (vmfloat_t)0.0, (vmfloat_t)0.0, (vmfloat_t)0.0}};
#endif
};


static inline matrix_t mZero(void)
{
#ifdef VMATH_USE_ALTIVEC
	return (matrix_t){.varr={vec_zero(), vec_zero(), vec_zero(), vec_zero()}};
#else
	return (matrix_t){.vtarr={vZero(), vZero(), vZero(), vZero()}};
#endif
}

static inline matrix_t mIdentity(void)
{
	return (matrix_t){.farr={1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0}};
}


static inline matrix_t mTranspose(matrix_t m)
{
	vmfloat_t* farr = m.farr;
	return (matrix_t){.farr={farr[0], farr[4], farr[8], farr[12], farr[1], farr[5], farr[9], farr[13], farr[2], farr[6], farr[10], farr[14], farr[3], farr[7], farr[11], farr[15]}};
}

static inline matrix_t mCreateFromFloats(vmfloat_t a, vmfloat_t b, vmfloat_t c, vmfloat_t d, vmfloat_t e, vmfloat_t f, vmfloat_t g, vmfloat_t h, vmfloat_t i, vmfloat_t j, vmfloat_t k, vmfloat_t l, vmfloat_t m, vmfloat_t n, vmfloat_t o, vmfloat_t p)
{
	return (matrix_t){.farr={a,b,c,d, e,f,g,h, i,j,k,l, m,n,o,p}};
}

static inline matrix_t mAdjoint(matrix_t m)
{
	vmfloat_t *fmat[4] = { m.farr + 0, m.farr + 4, m.farr + 8, m.farr + 12 };

	vmfloat_t A = fmat[2][2]*fmat[3][3] - fmat[3][2]*fmat[2][3];
	vmfloat_t B = fmat[1][2]*fmat[3][3] - fmat[3][2]*fmat[1][3];
	vmfloat_t C = fmat[1][2]*fmat[2][3] - fmat[2][2]*fmat[1][3];
	vmfloat_t D = fmat[0][2]*fmat[3][3] - fmat[3][2]*fmat[0][3];
	vmfloat_t E = fmat[0][2]*fmat[2][3] - fmat[2][2]*fmat[0][3];
	vmfloat_t F = fmat[0][2]*fmat[1][3] - fmat[1][2]*fmat[0][3];

	vmfloat_t A3 = fmat[2][1]*fmat[3][3] - fmat[3][1]*fmat[2][3];
	vmfloat_t B3 = fmat[1][1]*fmat[3][3] - fmat[3][1]*fmat[1][3];
	vmfloat_t C3 = fmat[1][1]*fmat[2][3] - fmat[2][1]*fmat[1][3];
	vmfloat_t D3 = fmat[0][1]*fmat[3][3] - fmat[3][1]*fmat[0][3];
	vmfloat_t E3 = fmat[0][1]*fmat[2][3] - fmat[2][1]*fmat[0][3];
	vmfloat_t F3 = fmat[0][1]*fmat[1][3] - fmat[1][1]*fmat[0][3];

	vmfloat_t A4 = fmat[2][1]*fmat[3][2] - fmat[3][1]*fmat[2][2];
	vmfloat_t B4 = fmat[1][1]*fmat[3][2] - fmat[3][1]*fmat[1][2];
	vmfloat_t C4 = fmat[1][1]*fmat[2][2] - fmat[2][1]*fmat[1][2];
	vmfloat_t D4 = fmat[0][1]*fmat[3][2] - fmat[3][1]*fmat[0][2];
	vmfloat_t E4 = fmat[0][1]*fmat[2][2] - fmat[2][1]*fmat[0][2];
	vmfloat_t F4 = fmat[0][1]*fmat[1][2] - fmat[1][1]*fmat[0][2];

	vmfloat_t AA = fmat[1][1]*A - fmat[2][1]*B + fmat[3][1]*C;
	vmfloat_t BB = fmat[0][1]*A - fmat[2][1]*D + fmat[3][1]*E;
	vmfloat_t CC = fmat[0][1]*B - fmat[1][1]*D + fmat[3][1]*F;
	vmfloat_t DD = fmat[0][1]*C - fmat[1][1]*E + fmat[2][1]*F;

	vmfloat_t EE = fmat[1][0]*A - fmat[2][0]*B + fmat[3][0]*C;
	vmfloat_t FF = fmat[0][0]*A - fmat[2][0]*D + fmat[3][0]*E;
	vmfloat_t GG = fmat[0][0]*B - fmat[1][0]*D + fmat[3][0]*F;
	vmfloat_t HH = fmat[0][0]*C - fmat[1][0]*E + fmat[2][0]*F;

	vmfloat_t II = fmat[1][0]*A3 - fmat[2][0]*B3 + fmat[3][0]*C3;
	vmfloat_t JJ = fmat[0][0]*A3 - fmat[2][0]*D3 + fmat[3][0]*E3;
	vmfloat_t KK = fmat[0][0]*B3 - fmat[1][0]*D3 + fmat[3][0]*F3;
	vmfloat_t LL = fmat[0][0]*C3 - fmat[1][0]*E3 + fmat[2][0]*F3;

	vmfloat_t MM = fmat[1][0]*A4 - fmat[2][0]*B4 + fmat[3][0]*C4;
	vmfloat_t NN = fmat[0][0]*A4 - fmat[2][0]*D4 + fmat[3][0]*E4;
	vmfloat_t OO = fmat[0][0]*B4 - fmat[1][0]*D4 + fmat[3][0]*F4;
	vmfloat_t PP = fmat[0][0]*C4 - fmat[1][0]*E4 + fmat[2][0]*F4;


	return mCreateFromFloats(AA, -BB, CC, -DD, -EE, FF, -GG, HH, II, -JJ, KK, -LL, -MM, NN, -OO, PP);

};

static inline matrix_t mMulScalar(matrix_t a, vmfloat_t b)
{
#ifdef VMATH_USE_ALTIVEC
	vector vmfloat_t v = {b, b, b, b};
	vector vmfloat_t v0 = vec_zero();
	return (matrix_t){.varr = {vec_madd(a.varr[0], v, v0), vec_madd(a.varr[1], v, v0), vec_madd(a.varr[2], v, v0), vec_madd(a.varr[3], v, v0)}};
#else
	matrix_t m;
	int i;
	for (i=0; i < 16; ++i)
		m.farr[i] = a.farr[i]*b;
	return m;
#endif
}

static inline vmfloat_t mDeterminant(matrix_t m)
{
	vmfloat_t *fmat[4] = { m.farr + 0, m.farr + 4, m.farr + 8, m.farr + 12 };

	vmfloat_t A = fmat[2][2]*fmat[3][3] - fmat[3][2]*fmat[2][3];
	vmfloat_t B = fmat[1][2]*fmat[3][3] - fmat[3][2]*fmat[1][3];
	vmfloat_t C = fmat[1][2]*fmat[2][3] - fmat[2][2]*fmat[1][3];
	vmfloat_t D = fmat[0][2]*fmat[3][3] - fmat[3][2]*fmat[0][3];
	vmfloat_t E = fmat[0][2]*fmat[2][3] - fmat[2][2]*fmat[0][3];
	vmfloat_t F = fmat[0][2]*fmat[1][3] - fmat[1][2]*fmat[0][3];

	vmfloat_t AA = fmat[1][1]*A - fmat[2][1]*B + fmat[3][1]*C;
	vmfloat_t BB = fmat[0][1]*A - fmat[2][1]*D + fmat[3][1]*E;
	vmfloat_t CC = fmat[0][1]*B - fmat[1][1]*D + fmat[3][1]*F;
	vmfloat_t DD = fmat[0][1]*C - fmat[1][1]*E + fmat[2][1]*F;

	vmfloat_t AAA = fmat[0][0]*AA - fmat[1][0]*BB + fmat[2][0]*CC - fmat[3][0]*DD;

	return AAA;

};


static inline matrix_t mInverse(matrix_t m)
{
	matrix_t	mm	= mAdjoint(m);
	vmfloat_t	d	= mDeterminant(m);
    return mMulScalar(mm, 1.0/d);
};

static inline vector_t mTransformVec(matrix_t M, vector_t V)
{
	vector_t	V2;
	V2.farr[0] = V.farr[0]*M.fmat[0][0] + V.farr[1]*M.fmat[1][0] + V.farr[2]*M.fmat[2][0] + V.farr[3]*M.fmat[3][0];
	V2.farr[1] = V.farr[0]*M.fmat[0][1] + V.farr[1]*M.fmat[1][1] + V.farr[2]*M.fmat[2][1] + V.farr[3]*M.fmat[3][1];
	V2.farr[2] = V.farr[0]*M.fmat[0][2] + V.farr[1]*M.fmat[1][2] + V.farr[2]*M.fmat[2][2] + V.farr[3]*M.fmat[3][2];
	V2.farr[3] = V.farr[0]*M.fmat[0][3] + V.farr[1]*M.fmat[1][3] + V.farr[2]*M.fmat[2][3] + V.farr[3]*M.fmat[3][3];

	return V2;
}

static inline vector_t mTransformPos(matrix_t M, vector_t V)
{
	vector_t	V2;
	V2.farr[0] = V.farr[0]*M.fmat[0][0] + V.farr[1]*M.fmat[1][0] + V.farr[2]*M.fmat[2][0] + M.fmat[3][0];
	V2.farr[1] = V.farr[0]*M.fmat[0][1] + V.farr[1]*M.fmat[1][1] + V.farr[2]*M.fmat[2][1] + M.fmat[3][1];
	V2.farr[2] = V.farr[0]*M.fmat[0][2] + V.farr[1]*M.fmat[1][2] + V.farr[2]*M.fmat[2][2] + M.fmat[3][2];
	V2.farr[3] = V.farr[0]*M.fmat[0][3] + V.farr[1]*M.fmat[1][3] + V.farr[2]*M.fmat[2][3] + M.fmat[3][3];

	return V2;
}

static inline vector_t mTransformDir(matrix_t M, vector_t V)
{
	vector_t	V2;
	V2.farr[0] = V.farr[0]*M.fmat[0][0] + V.farr[1]*M.fmat[1][0] + V.farr[2]*M.fmat[2][0];
	V2.farr[1] = V.farr[0]*M.fmat[0][1] + V.farr[1]*M.fmat[1][1] + V.farr[2]*M.fmat[2][1];
	V2.farr[2] = V.farr[0]*M.fmat[0][2] + V.farr[1]*M.fmat[1][2] + V.farr[2]*M.fmat[2][2];
	V2.farr[3] = V.farr[0]*M.fmat[0][3] + V.farr[1]*M.fmat[1][3] + V.farr[2]*M.fmat[2][3];

	return V2;
}

static inline matrix_t mTransform (matrix_t a, matrix_t M) // a transforms M
{
	matrix_t	MM;
	int i;
	for (i = 0; i < 4; i++)
	{
		MM.fmat[i][0] = M.fmat[i][0]*a.fmat[0][0] + M.fmat[i][1]*a.fmat[1][0] + M.fmat[i][2]*a.fmat[2][0] + M.fmat[i][3]*a.fmat[3][0];
		MM.fmat[i][1] = M.fmat[i][0]*a.fmat[0][1] + M.fmat[i][1]*a.fmat[1][1] + M.fmat[i][2]*a.fmat[2][1] + M.fmat[i][3]*a.fmat[3][1];
		MM.fmat[i][2] = M.fmat[i][0]*a.fmat[0][2] + M.fmat[i][1]*a.fmat[1][2] + M.fmat[i][2]*a.fmat[2][2] + M.fmat[i][3]*a.fmat[3][2];
		MM.fmat[i][3] = M.fmat[i][0]*a.fmat[0][3] + M.fmat[i][1]*a.fmat[1][3] + M.fmat[i][2]*a.fmat[2][3] + M.fmat[i][3]*a.fmat[3][3];
	}
	return MM;
}


matrix_t mRotationMatrixHPB(vector_t v);
matrix_t mOrthogonalize(matrix_t m);

static inline matrix_t mTranslationMatrix(vector_t v)
{

	matrix_t m = mIdentity();
	m.fmat[3][0] = v.farr[0];
	m.fmat[3][1] = v.farr[1];
	m.fmat[3][2] = v.farr[2];

	return m;
};

static inline matrix_t mScaleMatrix(vector_t v)
{

	matrix_t m = mIdentity();
	m.fmat[0][0] = v.farr[0];
	m.fmat[1][1] = v.farr[1];
	m.fmat[2][2] = v.farr[2];

	return m;
};

static inline matrix_t mScaleMatrixUniform(vmfloat_t v)
{

	matrix_t m = mIdentity();
	m.fmat[0][0] = v;
	m.fmat[1][1] = v;
	m.fmat[2][2] = v;

	return m;
};


/* this function is no good because its unclear wether it creates a position or direction vector
static inline vector_t CreateVector3D(vmfloat_t a, vmfloat_t b, vmfloat_t c)
{
	vector_t v = {.farr = {a, b, c, 1.0}};
	return v;
};
*/

static inline vector_t vCreatePos(vmfloat_t a, vmfloat_t b, vmfloat_t c)
{
	vector_t v = {.farr = {a, b, c, 1.0}};
	return v;
};

static inline vector_t vCreateDir(vmfloat_t a, vmfloat_t b, vmfloat_t c)
{
	vector_t v = {.farr = {a, b, c, 0.0}};
	return v;
};


static inline vector_t CreateHVector3D(vmfloat_t a, vmfloat_t b, vmfloat_t c, vmfloat_t d)
{
	vector_t v = {.farr = {a, b, c, d}};
	return v;
};

static inline vmfloat_t vDot(vector_t a, vector_t b)
{
	vmfloat_t* aptr = a.farr;
	vmfloat_t* bptr = b.farr;
	return aptr[0]*bptr[0] + aptr[1]*bptr[1] + aptr[2]*bptr[2];
}

static inline vector_t vCross(vector_t a, vector_t b)
{
	vmfloat_t* aptr = (vmfloat_t*)&a;
	vmfloat_t* bptr = (vmfloat_t*)&b;
	return (vector_t) {.farr={
		aptr[1]*bptr[2] - aptr[2]*bptr[1],
		aptr[2]*bptr[0] - aptr[0]*bptr[2],
		aptr[0]*bptr[1] - aptr[1]*bptr[0], aptr[3]
		}};
}

static inline vector_t vSubRaw(vector_t a, vector_t b)
{
#ifdef VMATH_USE_ALTIVEC
	return (vector_t){.vec=vec_sub(a.vec, b.vec)};
#else
	return (vector_t){.farr={a.farr[0] - b.farr[0],a.farr[1] - b.farr[1],a.farr[2] - b.farr[2],a.farr[3] - b.farr[3]}};
#endif
}

static inline vector_t vAddRaw(vector_t a, vector_t b)
{
#ifdef VMATH_USE_ALTIVEC
	return (vector_t){.vec=vec_add(a.vec, b.vec)};
#else
	return (vector_t){.farr={a.farr[0] + b.farr[0],a.farr[1] + b.farr[1],a.farr[2] + b.farr[2],a.farr[3] + b.farr[3]}};
#endif
}

static inline vector_t vAdd3D(vector_t a, vector_t b)
{
	if ((a.farr[3] == (vmfloat_t)0.0) && (a.farr[3] == (vmfloat_t)0.0))
		return (vector_t){.farr={a.farr[0] + b.farr[0], a.farr[1] + b.farr[1], a.farr[2] + b.farr[2], 0.0}};
	else if (a.farr[3] == (vmfloat_t)0.0)
	{
		vmfloat_t bw = 1.0/b.farr[3];
		return (vector_t){.farr={a.farr[0] + b.farr[0]*bw, a.farr[1] + b.farr[1]*bw, a.farr[2] + b.farr[2]*bw, 0.0}};
	}
	else if (b.farr[3] == (vmfloat_t)0.0)
	{
		vmfloat_t aw = 1.0/a.farr[3];
		return (vector_t){.farr={a.farr[0]*aw + b.farr[0], a.farr[1]*aw + b.farr[1], a.farr[2]*aw + b.farr[2], 0.0}};
	}
	else
	{
		vmfloat_t aw = 1.0/a.farr[3];
		vmfloat_t bw = 1.0/b.farr[3];
		return (vector_t){.farr={a.farr[0]*aw + b.farr[0]*bw, a.farr[1]*aw + b.farr[1]*bw, a.farr[2]*aw + b.farr[2]*bw, 1.0}};
	}
}

static inline vector_t vSub3D(vector_t a, vector_t b)
{
	if ((a.farr[3] == (vmfloat_t)0.0) && (a.farr[3] == (vmfloat_t)0.0))
		return (vector_t){.farr={a.farr[0] - b.farr[0], a.farr[1] - b.farr[1], a.farr[2] - b.farr[2], 0.0}};
	else if (a.farr[3] == (vmfloat_t)0.0)
	{
		vmfloat_t bw = 1.0/b.farr[3];
		return (vector_t){.farr={a.farr[0] - b.farr[0]*bw, a.farr[1] - b.farr[1]*bw, a.farr[2] - b.farr[2]*bw, 0.0}};
	}
	else if (b.farr[3] == (vmfloat_t)0.0)
	{
		vmfloat_t aw = 1.0/a.farr[3];
		return (vector_t){.farr={a.farr[0]*aw - b.farr[0], a.farr[1]*aw - b.farr[1], a.farr[2]*aw - b.farr[2], 0.0}};
	}
	else
	{
		vmfloat_t aw = 1.0/a.farr[3];
		vmfloat_t bw = 1.0/b.farr[3];
		return (vector_t){.farr={a.farr[0]*aw - b.farr[0]*bw, a.farr[1]*aw - b.farr[1]*bw, a.farr[2]*aw - b.farr[2]*bw, 1.0}};
	}
}

static inline vector_t vMin(vector_t a, vector_t b)
{
#ifdef VMATH_USE_ALTIVEC
	return (vector_t){.vec=vec_min(a.vec, b.vec)};
#else
	return (vector_t){.farr={MIN(a.farr[0],b.farr[0]),MIN(a.farr[1],b.farr[1]),MIN(a.farr[2],b.farr[2]),MIN(a.farr[3],b.farr[3])}};
#endif
}

static inline vector_t vMax(vector_t a, vector_t b)
{
#ifdef VMATH_USE_ALTIVEC
	return (vector_t){.vec=vec_max(a.vec, b.vec)};
#else
	return (vector_t){.farr={MAX(a.farr[0],b.farr[0]),MAX(a.farr[1],b.farr[1]),MAX(a.farr[2],b.farr[2]),MAX(a.farr[3],b.farr[3])}};
#endif
}

static inline vector_t vMulScalar3D(vector_t a, vmfloat_t b)
{
#ifdef VMATH_USE_ALTIVEC
	vector_t v = {.farr={b, b, b, 1.0}};
	return (vector_t){.vec = vec_madd(a.vec, v.vec, vec_zero())};
#else
	vector_t v;
	v.farr[0] = a.farr[0]*b;
	v.farr[1] = a.farr[1]*b;
	v.farr[2] = a.farr[2]*b;
	v.farr[3] = a.farr[3];
	return v;
#endif
}

static inline vector_t vProjectAOnB(vector_t a, vector_t b)
{
	vmfloat_t aDotB = vDot(a, b);
	vmfloat_t factor = aDotB/vDot(b,b);
	return vMulScalar3D(b, factor);
}

static inline vmfloat_t vLength(vector_t av)
{
	vmfloat_t* a = av.farr;
	return sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
}

static inline vector_t vSetLength(vector_t a, vmfloat_t b)
{
	vmfloat_t r = b/vLength(a);
#ifdef VMATH_USE_ALTIVEC
	vector vmfloat_t v = {r, r, r, 0.0};
	return (vector_t){.vec=vec_madd(a.vec, v, vec_zero())};
#else
	return vMulScalar3D(a,r);
#endif
}

static inline vector_t vAverage(vector_t a, vector_t b)
{
	return vMulScalar3D(vAdd3D(a, b), 0.5);
}

static inline vector_t vNormal(vector_t a, vector_t b, vector_t c)
{
	 return vCross(vSub3D(b, a), vSub3D(c, a));
};





static inline vector_t	vMinVector(vector_t a, vector_t b)
{
	return (vector_t){.farr={MIN(a.farr[0], b.farr[0]), MIN(a.farr[1], b.farr[1]), MIN(a.farr[2], b.farr[2]), MIN(a.farr[3], b.farr[3])}};
};

static inline vector_t	vMaxVector(vector_t a, vector_t b)
{
	return (vector_t){.farr={MAX(a.farr[0], b.farr[0]), MAX(a.farr[1], b.farr[1]), MAX(a.farr[2], b.farr[2]), MAX(a.farr[3], b.farr[3])}};
};

static inline range3d_t rEmptyRange(void)
{
	vector_t zvec = vZero();
	return (range3d_t){.minv=zvec, .maxv=zvec};
}

static inline range3d_t rCreateFromMinMax(vector_t min, vector_t max)
{
	return (range3d_t){.minv = min, .maxv = max};
}

static inline range3d_t rCreateFromVectors(vector_t a, vector_t b)
{
	vector_t min = vMinVector(a, b);
	vector_t max = vMaxVector(a, b);
	return (range3d_t){.minv = min, .maxv = max};
}


static inline int rIsEmptyRange(range3d_t r)
{
	return (r.minv.farr[0] >= r.maxv.farr[0]) || (r.minv.farr[1] >= r.maxv.farr[1]) || (r.minv.farr[2] >= r.maxv.farr[2]);
};

static inline int rRangesIntersect(range3d_t a, range3d_t b)
{
	if (rIsEmptyRange(a) || rIsEmptyRange(b))
		return 0;
	return ((a.minv.farr[0] < b.maxv.farr[0]) && (b.minv.farr[0] < a.maxv.farr[0]))
		&& ((a.minv.farr[1] < b.maxv.farr[1]) && (b.minv.farr[1] < a.maxv.farr[1]))
		&& ((a.minv.farr[2] < b.maxv.farr[2]) && (b.minv.farr[2] < a.maxv.farr[2]));

};

static inline range3d_t rIntersectRanges(range3d_t a, range3d_t b)
{
	vector_t min = vMaxVector(a.minv, b.minv);
	vector_t max = vMinVector(a.maxv, b.maxv);
	if ((max.farr[0] <= min.farr[0]) || (max.farr[1] <= min.farr[1]) || (max.farr[2] <= min.farr[2]))
		return rEmptyRange();
	return (range3d_t){.minv = min, .maxv = max};
};

static inline range3d_t rUnionRange(range3d_t a, range3d_t b)
{
	vector_t min = vMinVector(a.minv, b.minv);
	vector_t max = vMaxVector(a.maxv, b.maxv);
	return (range3d_t){.minv = min, .maxv = max};
};

static inline int rRangeContainsPoint(range3d_t r, vector_t P)
{
	return ((P.farr[0] >= r.minv.farr[0]) && (P.farr[1] >= r.minv.farr[1]) && (P.farr[2] >= r.minv.farr[2]) && (P.farr[0] <= r.maxv.farr[0]) && (P.farr[1] <= r.maxv.farr[1]) && (P.farr[2] <= r.maxv.farr[2]));
};

static inline range3d_t mTransformRange(matrix_t m, range3d_t r)
{
	vector_t rv = vSub3D(r.maxv, r.minv);
	matrix_t mr = mTranslationMatrix(r.minv);
	mr.fmat[0][0] = rv.farr[0];
	mr.fmat[1][1] = rv.farr[1];
	mr.fmat[2][2] = rv.farr[2];
	
	mr = mTransform(m,mr);
	
	vector_t a = mr.vtarr[3];
	vector_t b = vZero();
	b.farr[0] = MAX(MAX(mr.fmat[0][0], mr.fmat[1][0]), mr.fmat[2][0]);
	b.farr[1] = MAX(MAX(mr.fmat[0][1], mr.fmat[1][1]), mr.fmat[2][1]);
	b.farr[2] = MAX(MAX(mr.fmat[0][2], mr.fmat[1][2]), mr.fmat[2][2]);
	vector_t c = vZero();
	c.farr[0] = MIN(MIN(mr.fmat[0][0], mr.fmat[1][0]), mr.fmat[2][0]);
	c.farr[1] = MIN(MIN(mr.fmat[0][1], mr.fmat[1][1]), mr.fmat[2][1]);
	c.farr[2] = MIN(MIN(mr.fmat[0][2], mr.fmat[1][2]), mr.fmat[2][2]);
	b = vAdd3D(a,b);
	c = vAdd3D(a,c);
	return rCreateFromVectors(c,b);
};

