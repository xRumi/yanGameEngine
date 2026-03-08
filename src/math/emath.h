#pragma once

#include "math_types.h"
#include <math.h>

mat4 mat4_identity();
mat4 mat4_translation(float x, float y, float z);
mat4 mat4_scale(float x, float y, float z);
mat4 mat4_rotation_x(float angle);
mat4 mat4_rotation_y(float angle);
mat4 mat4_rotation_z(float angle);

mat4 mat4_mul(mat4 a, mat4 b);
vec4 mat4_mul_vec4(mat4 m, vec4 v);
mat4 mat4_transpose(mat4 m);
mat4 mat4_inverse(mat4 m);