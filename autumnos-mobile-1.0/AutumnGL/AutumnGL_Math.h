#ifndef MATH_H
#define MATH_H

#include <GLES3/gl3.h>
#include <math.h>
#include <string.h>
typedef struct {
	float m[16];
} mat4;


mat4 IdentityMatrix();
mat4 ScaleMatrix(mat4 m, float size);
mat4 RotateMatrixY(mat4 m, float angle);
mat4 TranslateMatrix(mat4 m, float x, float y, float z);
mat4 CalculateModelMatrix(float x, float y, float z, float angle, float size);

#endif
