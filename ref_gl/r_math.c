/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*
Copyright (C) 1997-2001 Quake2xp Team.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "r_local.h"

const mat3_t mat3_identity = {
	{ 1, 0, 0 },
	{ 0, 1, 0 },
	{ 0, 0, 1 }
};

const mat4_t mat4_identity = {
	{ 1, 0, 0, 0 },
	{ 0, 1, 0, 0 },
	{ 0, 0, 1, 0 },
	{ 0, 0, 0, 1 }
};

/*
===========
Q_sincos

===========
*/

#if defined _WIN32 && defined ASM_X86
__forceinline void Q_sincos (float a, float *s, float *c) {
	__asm {
		fld	a
			mov	ecx, s
			mov	edx, c
			fsincos
			fstp	dword ptr[edx]
			fstp	dword ptr[ecx]
	}
}
#else
__inline void Q_sincos (float a, float *s, float *c) {
	*s = (float)sin (a);
	*c = (float)cos (a);
}
#endif

/*
=================
AnglesToMat3

=================
*/
void AnglesToMat3 (const vec3_t angles, mat3_t m) {
	float     sp, sy, sr, cp, cy, cr;

	Q_sincos (DEG2RAD (angles[PITCH]), &sp, &cp);
	Q_sincos (DEG2RAD (angles[YAW]), &sy, &cy);
	Q_sincos (DEG2RAD (angles[ROLL]), &sr, &cr);

	m[0][0] = cp * cy;
	m[0][1] = cp * sy;
	m[0][2] = -sp;
	m[1][0] = sr * sp * cy - cr * sy;
	m[1][1] = sr * sp * sy + cr * cy;
	m[1][2] = sr * cp;
	m[2][0] = cr * sp * cy + sr * sy;
	m[2][1] = cr * sp * sy - sr * cy;
	m[2][2] = cr * cp;
}

/*
================
Mat3_Identity

================
*/
void Mat3_Identity (mat3_t mat) {
	mat[0][0] = 1;
	mat[0][1] = 0;
	mat[0][2] = 0;
	mat[1][0] = 0;
	mat[1][1] = 1;
	mat[1][2] = 0;
	mat[2][0] = 0;
	mat[2][1] = 0;
	mat[2][2] = 1;
}

/*
================
Mat3_Copy

================
*/
void Mat3_Copy (const mat3_t in, mat3_t out) {
	VectorCopy (in[0], out[0]);
	VectorCopy (in[1], out[1]);
	VectorCopy (in[2], out[2]);
}

/*
================
Mat3_TransposeMultiplyVector

Need this to receive local space coords in one quick operation.
================
*/
void Mat3_TransposeMultiplyVector (const mat3_t m, const vec3_t in, vec3_t out) {
	out[0] = m[0][0] * in[0] + m[0][1] * in[1] + m[0][2] * in[2];
	out[1] = m[1][0] * in[0] + m[1][1] * in[1] + m[1][2] * in[2];
	out[2] = m[2][0] * in[0] + m[2][1] * in[1] + m[2][2] * in[2];
}

/*
================
Mat3_MultiplyVector

Used to transform something to world space.
================
*/
void Mat3_MultiplyVector(const mat3_t m, const vec3_t in, vec3_t out) {
	out[0] = m[0][0] * in[0] + m[1][0] * in[1] + m[2][0] * in[2];
	out[1] = m[0][1] * in[0] + m[1][1] * in[1] + m[2][1] * in[2];
	out[2] = m[0][2] * in[0] + m[1][2] * in[1] + m[2][2] * in[2];
}

qboolean Mat3_Compare(const mat3_t a, const mat3_t b) {
	int		i, j;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			if (a[i][j] != b[i][j])
				return qfalse;
		}
	}

	return qtrue;
}

qboolean Mat3_IsIdentity(const mat3_t mat) {
	return Mat3_Compare(mat, mat3_identity);
}

/*
===========
Mat4_Multiply

===========
*/
void Mat4_Multiply (const mat4_t a, const mat4_t b, mat4_t out) {
#if 0
	int		i, j;

	for (i = 0; i < 4; i++)
	for (j = 0; j < 4; j++)
		out[i][j] = a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j] + a[i][3] * b[3][j];
#else
	const float	a00 = a[0][0], a01 = a[0][1], a02 = a[0][2], a03 = a[0][3];
	const float	a10 = a[1][0], a11 = a[1][1], a12 = a[1][2], a13 = a[1][3];
	const float	a20 = a[2][0], a21 = a[2][1], a22 = a[2][2], a23 = a[2][3];
	const float	a30 = a[3][0], a31 = a[3][1], a32 = a[3][2], a33 = a[3][3];

	const float	b00 = b[0][0], b01 = b[0][1], b02 = b[0][2], b03 = b[0][3];
	const float	b10 = b[1][0], b11 = b[1][1], b12 = b[1][2], b13 = b[1][3];
	const float	b20 = b[2][0], b21 = b[2][1], b22 = b[2][2], b23 = b[2][3];
	const float	b30 = b[3][0], b31 = b[3][1], b32 = b[3][2], b33 = b[3][3];

	out[0][0] = a00 * b00 + a01 * b10 + a02 * b20 + a03 * b30;
	out[0][1] = a00 * b01 + a01 * b11 + a02 * b21 + a03 * b31;
	out[0][2] = a00 * b02 + a01 * b12 + a02 * b22 + a03 * b32;
	out[0][3] = a00 * b03 + a01 * b13 + a02 * b23 + a03 * b33;

	out[1][0] = a10 * b00 + a11 * b10 + a12 * b20 + a13 * b30;
	out[1][1] = a10 * b01 + a11 * b11 + a12 * b21 + a13 * b31;
	out[1][2] = a10 * b02 + a11 * b12 + a12 * b22 + a13 * b32;
	out[1][3] = a10 * b03 + a11 * b13 + a12 * b23 + a13 * b33;

	out[2][0] = a20 * b00 + a21 * b10 + a22 * b20 + a23 * b30;
	out[2][1] = a20 * b01 + a21 * b11 + a22 * b21 + a23 * b31;
	out[2][2] = a20 * b02 + a21 * b12 + a22 * b22 + a23 * b32;
	out[2][3] = a20 * b03 + a21 * b13 + a22 * b23 + a23 * b33;

	out[3][0] = a30 * b00 + a31 * b10 + a32 * b20 + a33 * b30;
	out[3][1] = a30 * b01 + a31 * b11 + a32 * b21 + a33 * b31;
	out[3][2] = a30 * b02 + a31 * b12 + a32 * b22 + a33 * b32;
	out[3][3] = a30 * b03 + a31 * b13 + a32 * b23 + a33 * b33;
#endif
}

/*
=================
Mat4_Copy

=================
*/
void Mat4_Copy (const mat4_t in, mat4_t out) {
	out[0][0] = in[0][0];
	out[0][1] = in[0][1];
	out[0][2] = in[0][2];
	out[0][3] = in[0][3];
	out[1][0] = in[1][0];
	out[1][1] = in[1][1];
	out[1][2] = in[1][2];
	out[1][3] = in[1][3];
	out[2][0] = in[2][0];
	out[2][1] = in[2][1];
	out[2][2] = in[2][2];
	out[2][3] = in[2][3];
	out[3][0] = in[3][0];
	out[3][1] = in[3][1];
	out[3][2] = in[3][2];
	out[3][3] = in[3][3];
}

/*
=================
Mat4_Transpose

=================
*/
void Mat4_Transpose (const mat4_t in, mat4_t out) {
	out[0][0] = in[0][0];
	out[0][1] = in[1][0];
	out[0][2] = in[2][0];
	out[0][3] = in[3][0];
	out[1][0] = in[0][1];
	out[1][1] = in[1][1];
	out[1][2] = in[2][1];
	out[1][3] = in[3][1];
	out[2][0] = in[0][2];
	out[2][1] = in[1][2];
	out[2][2] = in[2][2];
	out[2][3] = in[3][2];
	out[3][0] = in[0][3];
	out[3][1] = in[1][3];
	out[3][2] = in[2][3];
	out[3][3] = in[3][3];
}

/*
===========
Mat4_TransposeMultiply

===========
*/
void Mat4_TransposeMultiply (const mat4_t a, const mat4_t b, mat4_t out) {

	const float	a00 = a[0][0], a01 = a[0][1], a02 = a[0][2], a03 = a[0][3];
	const float	a10 = a[1][0], a11 = a[1][1], a12 = a[1][2], a13 = a[1][3];
	const float	a20 = a[2][0], a21 = a[2][1], a22 = a[2][2], a23 = a[2][3];
	const float	a30 = a[3][0], a31 = a[3][1], a32 = a[3][2], a33 = a[3][3];

	const float	b00 = b[0][0], b01 = b[0][1], b02 = b[0][2], b03 = b[0][3];
	const float	b10 = b[1][0], b11 = b[1][1], b12 = b[1][2], b13 = b[1][3];
	const float	b20 = b[2][0], b21 = b[2][1], b22 = b[2][2], b23 = b[2][3];
	const float	b30 = b[3][0], b31 = b[3][1], b32 = b[3][2], b33 = b[3][3];

	out[0][0] = a00 * b00 + a10 * b10 + a20 * b20 + a30 * b30;
	out[0][1] = a00 * b01 + a10 * b11 + a20 * b21 + a30 * b31;
	out[0][2] = a00 * b02 + a10 * b12 + a20 * b22 + a30 * b32;
	out[0][3] = a00 * b03 + a10 * b13 + a20 * b23 + a30 * b33;

	out[1][0] = a01 * b00 + a11 * b10 + a21 * b20 + a31 * b30;
	out[1][1] = a01 * b01 + a11 * b11 + a21 * b21 + a31 * b31;
	out[1][2] = a01 * b02 + a11 * b12 + a21 * b22 + a31 * b32;
	out[1][3] = a01 * b03 + a11 * b13 + a21 * b23 + a31 * b33;

	out[2][0] = a02 * b00 + a12 * b10 + a22 * b20 + a32 * b30;
	out[2][1] = a02 * b01 + a12 * b11 + a22 * b21 + a32 * b31;
	out[2][2] = a02 * b02 + a12 * b12 + a22 * b22 + a32 * b32;
	out[2][3] = a02 * b03 + a12 * b13 + a22 * b23 + a32 * b33;

	out[3][0] = a03 * b00 + a13 * b10 + a23 * b20 + a33 * b30;
	out[3][1] = a03 * b01 + a13 * b11 + a23 * b21 + a33 * b31;
	out[3][2] = a03 * b02 + a13 * b12 + a23 * b22 + a33 * b32;
	out[3][3] = a03 * b03 + a13 * b13 + a23 * b23 + a33 * b33;
}

/*
===============
Mat4_MultiplyVector

===============
*/
void Mat4_MultiplyVector (const mat4_t m, const vec3_t in, vec3_t out) {
	float	s;

	s = m[3][0] * in[0] + m[3][1] * in[1] + m[3][2] * in[2] + m[3][3];

	if (s == 0.0)
		VectorClear (out);
	else if (s == 1.0) {
		out[0] = m[0][0] * in[0] + m[0][1] * in[1] + m[0][2] * in[2] + m[0][3];
		out[1] = m[1][0] * in[0] + m[1][1] * in[1] + m[1][2] * in[2] + m[1][3];
		out[2] = m[2][0] * in[0] + m[2][1] * in[1] + m[2][2] * in[2] + m[2][3];
	}
	else {
		s = 1.0 / s;

		out[0] = (m[0][0] * in[0] + m[0][1] * in[1] + m[0][2] * in[2] + m[0][3]) * s;
		out[1] = (m[1][0] * in[0] + m[1][1] * in[1] + m[1][2] * in[2] + m[1][3]) * s;
		out[2] = (m[2][0] * in[0] + m[2][1] * in[1] + m[2][2] * in[2] + m[2][3]) * s;
	}
}

/*
============
Mat4_Invert

104 multiplications, 1 division.
============
*/
#define MATRIX_INVERSE_EPSILON		1e-14
#define MATRIX_EPSILON				1e-6
float Q_fabs (float f);

qboolean Mat4_Invert (const mat4_t in, mat4_t out) {
	float	det2_01_01, det2_01_02, det2_01_03;
	float	det2_01_12, det2_01_13, det2_01_23;

	float	det3_201_012, det3_201_013, det3_201_023, det3_201_123;

	float	det2_03_01, det2_03_02, det2_03_03;
	float	det2_03_12, det2_03_13, det2_03_23;
	float	det2_13_01, det2_13_02, det2_13_03;
	float	det2_13_12, det2_13_13, det2_13_23;

	float	det3_203_012, det3_203_013, det3_203_023, det3_203_123;
	float	det3_213_012, det3_213_013, det3_213_023, det3_213_123;
	float	det3_301_012, det3_301_013, det3_301_023, det3_301_123;

	double	det, invDet;

	// 2x2 sub-determinants required to calculate 4x4 determinant
	det2_01_01 = in[0][0] * in[1][1] - in[0][1] * in[1][0];
	det2_01_02 = in[0][0] * in[1][2] - in[0][2] * in[1][0];
	det2_01_03 = in[0][0] * in[1][3] - in[0][3] * in[1][0];
	det2_01_12 = in[0][1] * in[1][2] - in[0][2] * in[1][1];
	det2_01_13 = in[0][1] * in[1][3] - in[0][3] * in[1][1];
	det2_01_23 = in[0][2] * in[1][3] - in[0][3] * in[1][2];

	// 3x3 sub-determinants required to calculate 4x4 determinant
	det3_201_012 = in[2][0] * det2_01_12 - in[2][1] * det2_01_02 + in[2][2] * det2_01_01;
	det3_201_013 = in[2][0] * det2_01_13 - in[2][1] * det2_01_03 + in[2][3] * det2_01_01;
	det3_201_023 = in[2][0] * det2_01_23 - in[2][2] * det2_01_03 + in[2][3] * det2_01_02;
	det3_201_123 = in[2][1] * det2_01_23 - in[2][2] * det2_01_13 + in[2][3] * det2_01_12;

	det = -det3_201_123 * in[3][0] + det3_201_023 * in[3][1] - det3_201_013 * in[3][2] + det3_201_012 * in[3][3];

	if (Q_fabs (det) < MATRIX_INVERSE_EPSILON)
		return qfalse;

	invDet = 1.0f / det;

	// remaining 2x2 sub-determinants
	det2_03_01 = in[0][0] * in[3][1] - in[0][1] * in[3][0];
	det2_03_02 = in[0][0] * in[3][2] - in[0][2] * in[3][0];
	det2_03_03 = in[0][0] * in[3][3] - in[0][3] * in[3][0];
	det2_03_12 = in[0][1] * in[3][2] - in[0][2] * in[3][1];
	det2_03_13 = in[0][1] * in[3][3] - in[0][3] * in[3][1];
	det2_03_23 = in[0][2] * in[3][3] - in[0][3] * in[3][2];

	det2_13_01 = in[1][0] * in[3][1] - in[1][1] * in[3][0];
	det2_13_02 = in[1][0] * in[3][2] - in[1][2] * in[3][0];
	det2_13_03 = in[1][0] * in[3][3] - in[1][3] * in[3][0];
	det2_13_12 = in[1][1] * in[3][2] - in[1][2] * in[3][1];
	det2_13_13 = in[1][1] * in[3][3] - in[1][3] * in[3][1];
	det2_13_23 = in[1][2] * in[3][3] - in[1][3] * in[3][2];

	// remaining 3x3 sub-determinants
	det3_203_012 = in[2][0] * det2_03_12 - in[2][1] * det2_03_02 + in[2][2] * det2_03_01;
	det3_203_013 = in[2][0] * det2_03_13 - in[2][1] * det2_03_03 + in[2][3] * det2_03_01;
	det3_203_023 = in[2][0] * det2_03_23 - in[2][2] * det2_03_03 + in[2][3] * det2_03_02;
	det3_203_123 = in[2][1] * det2_03_23 - in[2][2] * det2_03_13 + in[2][3] * det2_03_12;

	det3_213_012 = in[2][0] * det2_13_12 - in[2][1] * det2_13_02 + in[2][2] * det2_13_01;
	det3_213_013 = in[2][0] * det2_13_13 - in[2][1] * det2_13_03 + in[2][3] * det2_13_01;
	det3_213_023 = in[2][0] * det2_13_23 - in[2][2] * det2_13_03 + in[2][3] * det2_13_02;
	det3_213_123 = in[2][1] * det2_13_23 - in[2][2] * det2_13_13 + in[2][3] * det2_13_12;

	det3_301_012 = in[3][0] * det2_01_12 - in[3][1] * det2_01_02 + in[3][2] * det2_01_01;
	det3_301_013 = in[3][0] * det2_01_13 - in[3][1] * det2_01_03 + in[3][3] * det2_01_01;
	det3_301_023 = in[3][0] * det2_01_23 - in[3][2] * det2_01_03 + in[3][3] * det2_01_02;
	det3_301_123 = in[3][1] * det2_01_23 - in[3][2] * det2_01_13 + in[3][3] * det2_01_12;

	out[0][0] = -det3_213_123 * invDet;
	out[1][0] = det3_213_023 * invDet;
	out[2][0] = -det3_213_013 * invDet;
	out[3][0] = det3_213_012 * invDet;

	out[0][1] = det3_203_123 * invDet;
	out[1][1] = -det3_203_023 * invDet;
	out[2][1] = det3_203_013 * invDet;
	out[3][1] = -det3_203_012 * invDet;

	out[0][2] = det3_301_123 * invDet;
	out[1][2] = -det3_301_023 * invDet;
	out[2][2] = det3_301_013 * invDet;
	out[3][2] = -det3_301_012 * invDet;

	out[0][3] = -det3_201_123 * invDet;
	out[1][3] = det3_201_023 * invDet;
	out[2][3] = -det3_201_013 * invDet;
	out[3][3] = det3_201_012 * invDet;

	return qtrue;
}

/*
===============
Mat4_Scale

===============
*/
void Mat4_Scale (mat4_t m, float x, float y, float z) {
	m[0][0] *= x;
	m[0][1] *= x;
	m[0][2] *= x;
	m[0][3] *= x;

	m[1][0] *= y;
	m[1][1] *= y;
	m[1][2] *= y;
	m[1][3] *= y;

	m[2][0] *= z;
	m[2][1] *= z;
	m[2][2] *= z;
	m[2][3] *= z;
}

/*
===============
Mat4_Translate

===============
*/
void Mat4_Translate (mat4_t m, float x, float y, float z) {
	m[3][0] += m[0][0] * x + m[1][0] * y + m[2][0] * z;
	m[3][1] += m[0][1] * x + m[1][1] * y + m[2][1] * z;
	m[3][2] += m[0][2] * x + m[1][2] * y + m[2][2] * z;
	m[3][3] += m[0][3] * x + m[1][3] * y + m[2][3] * z;
}

void Mat4_Identity (mat4_t mat) {
	mat[0][0] = 1.0f;
	mat[0][1] = 0.0f;
	mat[0][2] = 0.0f;
	mat[0][3] = 0.0f;
	mat[1][0] = 0.0f;
	mat[1][1] = 1.0f;
	mat[1][2] = 0.0f;
	mat[1][3] = 0.0f;
	mat[2][0] = 0.0f;
	mat[2][1] = 0.0f;
	mat[2][2] = 1.0f;
	mat[2][3] = 0.0f;
	mat[3][0] = 0.0f;
	mat[3][1] = 0.0f;
	mat[3][2] = 0.0f;
	mat[3][3] = 1.0f;
}

/*
=================
Mat4_SetupTransform

=================
*/
void Mat4_SetupTransform (mat4_t m, const mat3_t rotation, const vec3_t translation) {
	m[0][0] = rotation[0][0];
	m[0][1] = rotation[0][1];
	m[0][2] = rotation[0][2];
	m[0][3] = 0.0f;

	m[1][0] = rotation[1][0];
	m[1][1] = rotation[1][1];
	m[1][2] = rotation[1][2];
	m[1][3] = 0.0f;

	m[2][0] = rotation[2][0];
	m[2][1] = rotation[2][1];
	m[2][2] = rotation[2][2];
	m[2][3] = 0.0f;

	m[3][0] = translation[0];
	m[3][1] = translation[1];
	m[3][2] = translation[2];
	m[3][3] = 1.0f;
}

/*
=================
Mat4_SetOrientation

'rotation' is transposed.
=================
*/
void Mat4_SetOrientation (mat4_t m, const mat3_t rotation, const vec3_t translation) {
	m[0][0] = rotation[0][0];
	m[0][1] = rotation[1][0];
	m[0][2] = rotation[2][0];
	m[0][3] = translation[0];

	m[1][0] = rotation[0][1];
	m[1][1] = rotation[1][1];
	m[1][2] = rotation[2][1];
	m[1][3] = translation[1];

	m[2][0] = rotation[0][2];
	m[2][1] = rotation[1][2];
	m[2][2] = rotation[2][2];
	m[2][3] = translation[2];

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
}

/*
===============
Mat4_Rotate

===============
*/
#define	EQUAL_EPSILON		0.000001f
void Mat4_Rotate (mat4_t m, float angle, float x, float y, float z) {
	vec4_t	mx, my, mz;
	vec3_t	rx, ry, rz;
	float	len, rad, s, c, i;
	float	xx, yy, zz, xy, yz, zx, xs, ys, zs;

	len = sqrt (x*x + y*y + z*z);
	if (len < EQUAL_EPSILON)
		return;

	len = 1.0 / len;

	x *= len;
	y *= len;
	z *= len;

	rad = DEG2RAD (angle);
	s = sin (rad);
	c = cos (rad);

	i = 1.0 - c;

	xx = (x * x) * i;
	yy = (y * y) * i;
	zz = (z * z) * i;
	xy = (x * y) * i;
	yz = (y * z) * i;
	zx = (z * x) * i;

	xs = x * s;
	ys = y * s;
	zs = z * s;

	mx[0] = m[0][0];
	mx[1] = m[0][1];
	mx[2] = m[0][2];
	mx[3] = m[0][3];
	my[0] = m[1][0];
	my[1] = m[1][1];
	my[2] = m[1][2];
	my[3] = m[1][3];
	mz[0] = m[2][0];
	mz[1] = m[2][1];
	mz[2] = m[2][2];
	mz[3] = m[2][3];

	rx[0] = xx + c;
	rx[1] = xy + zs;
	rx[2] = zx - ys;
	ry[0] = xy - zs;
	ry[1] = yy + c;
	ry[2] = yz + xs;
	rz[0] = zx + ys;
	rz[1] = yz - xs;
	rz[2] = zz + c;

	m[0][0] = mx[0] * rx[0] + my[0] * rx[1] + mz[0] * rx[2];
	m[0][1] = mx[1] * rx[0] + my[1] * rx[1] + mz[1] * rx[2];
	m[0][2] = mx[2] * rx[0] + my[2] * rx[1] + mz[2] * rx[2];
	m[0][3] = mx[3] * rx[0] + my[3] * rx[1] + mz[3] * rx[2];
	m[1][0] = mx[0] * ry[0] + my[0] * ry[1] + mz[0] * ry[2];
	m[1][1] = mx[1] * ry[0] + my[1] * ry[1] + mz[1] * ry[2];
	m[1][2] = mx[2] * ry[0] + my[2] * ry[1] + mz[2] * ry[2];
	m[1][3] = mx[3] * ry[0] + my[3] * ry[1] + mz[3] * ry[2];
	m[2][0] = mx[0] * rz[0] + my[0] * rz[1] + mz[0] * rz[2];
	m[2][1] = mx[1] * rz[0] + my[1] * rz[1] + mz[1] * rz[2];
	m[2][2] = mx[2] * rz[0] + my[2] * rz[1] + mz[2] * rz[2];
	m[2][3] = mx[3] * rz[0] + my[3] * rz[1] + mz[3] * rz[2];
}

/*
==================
Mat4_AffineInvert

==================
*/
void Mat4_AffineInvert(const mat4_t in, mat4_t out) {
	out[0][0] = in[0][0];
	out[0][1] = in[1][0];
	out[0][2] = in[2][0];
	out[0][3] = 0.0f;
	out[1][0] = in[0][1];
	out[1][1] = in[1][1];
	out[1][2] = in[2][1];
	out[1][3] = 0.0f;
	out[2][0] = in[0][2];
	out[2][1] = in[1][2];
	out[2][2] = in[2][2];
	out[2][3] = 0.0f;
	out[3][0] = -(in[0][0] * in[3][0] + in[0][1] * in[3][1] + in[0][2] * in[3][2]);
	out[3][1] = -(in[1][0] * in[3][0] + in[1][1] * in[3][1] + in[1][2] * in[3][2]);
	out[3][2] = -(in[2][0] * in[3][0] + in[2][1] * in[3][1] + in[2][2] * in[3][2]);
	out[3][3] = 1.0f;
}

void Mat3_Set(mat3_t mat, vec3_t x, vec3_t y, vec3_t z) {
	mat[0][0] = x[0];
	mat[0][1] = x[1];
	mat[0][2] = x[2];

	mat[1][0] = y[0];
	mat[1][1] = y[1];
	mat[1][2] = y[2];
	
	mat[2][0] = z[0];
	mat[2][1] = z[1];
	mat[2][2] = z[2];
}

void Mat4_Set(mat4_t mat, vec4_t x, vec4_t y, vec4_t z, vec4_t w) {
	mat[0][0] = x[0];
	mat[0][1] = x[1];
	mat[0][2] = x[2];
	mat[0][3] = x[3];

	mat[1][0] = y[0];
	mat[1][1] = y[1];
	mat[1][2] = y[2];
	mat[1][3] = y[3];

	mat[2][0] = z[0];
	mat[2][1] = z[1];
	mat[2][2] = z[2];
	mat[2][3] = z[3];

	mat[3][0] = w[0];
	mat[3][1] = w[1];
	mat[3][2] = w[2];
	mat[3][3] = w[3];
}

/*
===============
SetPlaneType

===============
*/
void SetPlaneType (cplane_t *plane) {
	if (plane->normal[0] == 1.0)
		plane->type = PLANE_X;
	else if (plane->normal[1] == 1.0)
		plane->type = PLANE_Y;
	else if (plane->normal[2] == 1.0)
		plane->type = PLANE_Z;
	else
		plane->type = PLANE_NON_AXIAL;
}

/*
===============
SetPlaneSignBits

For fast box on planeside test.
===============
*/
void SetPlaneSignBits (cplane_t *plane) {
	plane->signbits = 0;

	if (plane->normal[0] < 0)
		plane->signbits |= 1;
	if (plane->normal[1] < 0)
		plane->signbits |= 2;
	if (plane->normal[2] < 0)
		plane->signbits |= 4;
}

void AddBoundsToBounds(const vec3_t mins1, const vec3_t maxs1, vec3_t mins2, vec3_t maxs2) {
	int		i;

	for (i = 0; i<3; i++) {
		if (mins1[i] < mins2[i])
			mins2[i] = mins1[i];
		if (maxs1[i] > maxs2[i])
			maxs2[i] = maxs1[i];
	}
}

/*
=================
NormalToLatLong
=================
*/
void NormalToLatLong(const vec3_t normal, byte bytes[2]) {

	int		lat, lng;

	if (normal[0] == 0 && normal[1] == 0) {
		if (normal[2] > 0) {
			// Lattitude = 0, Longitude = 0
			bytes[0] = 0;
			bytes[1] = 0;
		}
		else {
			// Lattitude = 0, Longitude = 128
			bytes[0] = 128;
			bytes[1] = 0;
		}
	}
	else {
		lat = RAD2DEG(atan2(normal[1], normal[0])) * (255.0 / 360.0);
		lng = RAD2DEG(acos(normal[2])) * (255.0 / 360.0);

		bytes[0] = lng & 0xFF;
		bytes[1] = lat & 0xFF;
	}
}