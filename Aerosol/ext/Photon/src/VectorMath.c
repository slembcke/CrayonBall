//
//  VectorMath.m
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

#import "VectorMath.h"

matrix_t mOrthogonalize(matrix_t mIn)
{
	matrix_t m = mIn;
//	m.named.X = vSetLength(m.named.X, 1.0);
//	m.named.Y = vSetLength(m.named.Y, 1.0);
//	m.named.Z = vSetLength(m.named.Z, 1.0);
	
	m.named.Z = vCross(m.named.X, m.named.Y);
	m.named.Y = vCross(m.named.Z, m.named.X);
	return m;
}

matrix_t mRotationMatrixHPB(vector_t v)
{
	// row-column format matrix, vectors stored in columns
	vmfloat_t cosa = cosf(v.farr[0]);
	vmfloat_t sina = sin(v.farr[0]);
	matrix_t hM = mIdentity();
	hM.fmat[0][0] = cosa;
	hM.fmat[0][2] = -sina;
	hM.fmat[2][0] = sina;
	hM.fmat[2][2] = cosa;
	cosa = cos(v.farr[1]);
	sina = sin(v.farr[1]);
	matrix_t pM = mIdentity();
	pM.fmat[1][1] = cosa;
	pM.fmat[1][2] = sina;
	pM.fmat[2][1] = -sina;
	pM.fmat[2][2] = cosa;
	cosa = cos(v.farr[2]);
	sina = sin(v.farr[2]);
	matrix_t bM = mIdentity();
	bM.fmat[0][0] = cosa;
	bM.fmat[0][1] = sina;
	bM.fmat[1][0] = -sina;
	bM.fmat[1][1] = cosa;

	return mTransform(hM, mTransform(pM, bM));
};
