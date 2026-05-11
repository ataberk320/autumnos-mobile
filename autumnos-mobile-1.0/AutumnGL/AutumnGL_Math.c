#include "AutumnGL_Math.h"
mat4 IdentityMatrix() {
	mat4 res;
	memset(res.m, 0, sizeof(float) * 16);
	res.m[0] = 1.0f; res.m[5] = 1.0f; res.m[10] = 1.0f; res.m[15] = 1.0f;
	return res;
}

mat4 ScaleMatrix(mat4 m, float size) {
	m.m[0] *= size;
	m.m[5] *= size;
	m.m[10] *= size;
	return m;
}

mat4 RotateMatrixY(mat4 m, float angle) {
	mat4 rot = IdentityMatrix();
	float s = sinf(angle);
	float c = cosf(angle);
	rot.m[0] = c;	rot.m[2] = -s;
	rot.m[8] = s;	rot.m[10] = c;

	return rot;
}

mat4 TranslateMatrix(mat4 m, float x, float y, float z) {
	m.m[12] = x;
	m.m[13] = y;
	m.m[14] = z;
	return m;
}

mat4 CalculateModelMatrix(float x, float y, float z, float angle, float size) {
	mat4 model = IdentityMatrix();
	model = ScaleMatrix(model, size);
	model = RotateMatrixY(model, angle);
	model = TranslateMatrix(model, x, y, z);
	return model;
}
