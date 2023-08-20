#define VMATH_FLOAT_TYPE_IS_FLOAT 1
#include "VectorMath.h"

static inline matrix_t
psgTranslationMatrix(vmfloat_t x, vmfloat_t y)
{
	matrix_t m = mIdentity();
	m.fmat[3][0] = x;
	m.fmat[3][1] = y;

	return m;
}

static inline matrix_t
psgScaleMatrix(vmfloat_t x, vmfloat_t y)
{
	matrix_t m = mIdentity();
	m.fmat[0][0] = x;
	m.fmat[1][1] = y;
	m.fmat[2][2] = 1.0f;

	return m;
};

static inline matrix_t
psgRotateMatrix(vmfloat_t radians)
{
	vmfloat_t cos = cosf(radians);
	vmfloat_t sin = sinf(radians);
	
	return (matrix_t){.farr={
		 cos,  sin, 0.0f, 0.0f,
		 -sin,  cos, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	}};
}

static inline matrix_t
psgBoneScaleMatrix(vmfloat_t x0, vmfloat_t y0, vmfloat_t x1, vmfloat_t y1)
{
	vmfloat_t cosr = x1 - x0;
	vmfloat_t sinr = y1 - y0;
	
	return (matrix_t){.farr={
		 cosr, sinr, 0.0f, 0.0f,
		-sinr, cosr, 0.0f, 0.0f,
		 0.0f, 0.0f, 1.0f, 1.0f,
			 x0,   y0, 0.0f, 1.0f,
	}};
}

static inline matrix_t
psgSpriteMatrix(vmfloat_t x, vmfloat_t y, vmfloat_t sx, vmfloat_t sy, vmfloat_t radians)
{
	vmfloat_t cos = cosf(radians);
	vmfloat_t sin = sinf(radians);
	
	return (matrix_t){.farr={
		  cos*sx,  sin*sy, 0.0f, 0.0f,
		 -sin*sx,  cos*sy, 0.0f, 0.0f,
		    0.0f,    0.0f, 1.0f, 1.0f,
			     x,       y, 0.0f, 1.0f,
	}};
}

static inline matrix_t
psgOrthoMatrix(vmfloat_t right, vmfloat_t left, vmfloat_t bottom, vmfloat_t top)
{
	vmfloat_t a = 2.0f/(right - left);
	vmfloat_t b = -(right + left)/(right - left);
	vmfloat_t c = 2.0f/(top - bottom);
	vmfloat_t d = -(top + bottom)/(top - bottom);
	
	return (matrix_t){.farr={
		   a, 0.0f, 0.0f,    b,
		0.0f,    c, 0.0f,    d,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	}};
}
