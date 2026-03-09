#pragma once

#include "math_types.h"
#include <math.h>

vec3 vec3_add(vec3 a, vec3 b);
vec3 vec3_sub(vec3 a, vec3 b);
vec3 vec3_scale(vec3 a, float scale);
float vec3_dot(vec3 a, vec3 b);
vec3 vec3_cross(vec3 a, vec3 b);
vec3 vec3_normalize(vec3 a);
float vec3_length_sqr(vec3 a);
float vec3_length(vec3 a);

vec4 vec3_to_vec4(vec3 a, float w);

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

mat4 mat4_look_at(vec3 cameraPos, vec3 cameraTarget, vec3 up);
mat4 mat4_perspective(float fov, float aspect, float near, float far);