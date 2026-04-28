#pragma once

#include "math_types.h"
#include <math.h>

vec3 vec3_add(vec3 a, vec3 b);
vec3 vec3_sub(vec3 a, vec3 b);
vec3 vec3_neg(vec3 a);
vec3 vec3_scale(vec3 a, float scale);
vec3 vec3_cross(vec3 a, vec3 b);
vec3 vec3_normalize(vec3 a);
vec4 vec4_from_vec3(vec3 a, float w);
vec3 vec3_from_vec4(vec4 a);

vec3 vec3_min(vec3 a, vec3 b);
vec3 vec3_max(vec3 a, vec3 b);

float vec3_dot(vec3 a, vec3 b);
float vec3_length_sqr(vec3 a);
float vec3_length(vec3 a);

vec3 vec3_lerp(vec3 a, vec3 b, float t);
float scaler_lerp(float a, float b, float t);

mat4 mat4_identity();
mat4 mat4_translation(float x, float y, float z);
mat4 mat4_translation_vec3(vec3 a);
mat4 mat4_scale(float x, float y, float z);
mat4 mat4_scale_vec3(vec3 a);
mat4 mat4_rotation_x(float rad);
mat4 mat4_rotation_y(float rad);
mat4 mat4_rotation_z(float rad);

mat4 mat4_mul(mat4 a, mat4 b);
vec4 mat4_mul_vec4(mat4 m, vec4 v);
vec3 mat4_mul_vec3(mat4 m, vec3 v);
mat4 mat4_transpose(mat4 m);
mat4 mat4_inverse(mat4 m);

mat4 mat4_look(vec3 cameraPos, vec3 cameraDir, vec3 up);
mat4 mat4_look_at(vec3 cameraPos, vec3 cameraTarget, vec3 up);
mat4 mat4_perspective(float fov, float aspect, float near, float far);
mat4 mat4_orthographic_projection(float left, float right, float top, float bottom, float near, float far);
mat4 mat4_view_YXZ(vec3 position, vec3 rotation);