#include "emath.h"

vec3 vec3_add(vec3 a, vec3 b) {
    return (vec3){{
        a.ele[0] + b.ele[0],
        a.ele[1] + b.ele[1],
        a.ele[2] + b.ele[2]
    }};
}
vec3 vec3_sub(vec3 a, vec3 b) {
    return (vec3){{
        a.ele[0] - b.ele[0],
        a.ele[1] - b.ele[1],
        a.ele[2] - b.ele[2]
    }};
}
vec3 vec3_neg(vec3 a) {
    return (vec3){{-a.x, -a.y, -a.z}};
}
vec3 vec3_scale(vec3 a, float scale) {
    return (vec3){{
        a.ele[0] * scale,
        a.ele[1] * scale,
        a.ele[2] * scale
    }};
}
float vec3_dot(vec3 a, vec3 b) {
    return
        a.ele[0] * b.ele[0] +
        a.ele[1] * b.ele[1] +
        a.ele[2] * b.ele[2];
}
vec3 vec3_cross(vec3 a, vec3 b) {
    return (vec3){{
        a.ele[1]*b.ele[2] - a.ele[2]*b.ele[1],
       -a.ele[0]*b.ele[2] + a.ele[2]*b.ele[0],
        a.ele[0]*b.ele[1] - a.ele[1]*b.ele[0]
    }};
}
vec3 vec3_normalize(vec3 a) {
    float length = vec3_length(a);
    vec3 ret = {{
        a.ele[0] / length,
        a.ele[1] / length,
        a.ele[2] / length
    }};
    return ret;
}
float vec3_length_sqr(vec3 a) {
    return
        a.ele[0] * a.ele[0] +
        a.ele[1] * a.ele[1] +
        a.ele[2] * a.ele[2];
}
float vec3_length(vec3 a) {
    return sqrt(vec3_length_sqr(a));
}

vec4 vec4_from_vec3(vec3 a, float w) {
    return (vec4){{
        a.x, a.y, a.z, w
    }};
}
vec3 vec3_from_vec4(vec4 a) {
    return (vec3){{
        a.x, a.y, a.z
    }};
}

mat4 mat4_identity() {
    return (mat4){{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    }};
};

mat4 mat4_translation(float x, float y, float z) {
    return (mat4){{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, z, 1,
    }};
}
mat4 mat4_scale(float x, float y, float z) {
    return (mat4){{
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1,
    }};
}
mat4 mat4_rotation_x(float degree) {
    degree = TO_RADIANS(degree);
    float c = cosf(degree),
        s = sinf(degree);
    mat4 ret = {{
        1, 0, 0, 0,
        0, c, s, 0,
        0,-s, c, 0,
        0, 0, 0, 1,
    }};
    return ret;
}
mat4 mat4_rotation_y(float degree) {
    degree = TO_RADIANS(degree);
    float c = cosf(degree),
        s = sinf(degree);
    mat4 ret = {{
        c, 0,-s, 0,
        0, 1, 0, 0,
        s, 0, c, 0,
        0, 0, 0, 1,
    }};
    return ret;
}
mat4 mat4_rotation_z(float degree) {
    degree = TO_RADIANS(degree);
    float c = cosf(degree),
        s = sinf(degree);
    mat4 ret = {{
        c, s, 0, 0,
       -s, c, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    }};
    return ret;
}
mat4 mat4_mul(mat4 a, mat4 b) {
    mat4 ret = {};
    for (int col = 0; col < 4; col++)
        for (int row = 0; row < 4; row++)
            ret.e[col][row] =
                a.e[0][row] * b.e[col][0] +
                a.e[1][row] * b.e[col][1] +
                a.e[2][row] * b.e[col][2] +
                a.e[3][row] * b.e[col][3];
    return ret;
}
vec4 mat4_mul_vec4(mat4 m, vec4 v) {
    return (vec4){{
        m.ele[0]*v.ele[0] + m.ele[4]*v.ele[1] + m.ele[8]*v.ele[2] + m.ele[12]*v.ele[3],
        m.ele[1]*v.ele[0] + m.ele[5]*v.ele[1] + m.ele[9]*v.ele[2] + m.ele[13]*v.ele[3],
        m.ele[2]*v.ele[0] + m.ele[6]*v.ele[1] + m.ele[10]*v.ele[2] + m.ele[14]*v.ele[3],
        m.ele[3]*v.ele[0] + m.ele[7]*v.ele[1] + m.ele[11]*v.ele[2] + m.ele[15]*v.ele[3]
    }};
};
vec3 mat4_mul_vec3(mat4 m, vec3 v) {
    return (vec3){{
        m.ele[0]*v.ele[0] + m.ele[4]*v.ele[1] + m.ele[8]*v.ele[2],
        m.ele[1]*v.ele[0] + m.ele[5]*v.ele[1] + m.ele[9]*v.ele[2],
        m.ele[2]*v.ele[0] + m.ele[6]*v.ele[1] + m.ele[10]*v.ele[2],
    }};
};
mat4 mat4_transpose(mat4 m) {
    mat4 ret;
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            ret.ele[col * 4 + row] = m.ele[row * 4 + col];
    return ret;
}
mat4 mat4_inverse(mat4 m); // TODO: inverse matrix

mat4 mat4_look(vec3 position, vec3 direction, vec3 up) {
    vec3 front = vec3_normalize(direction),
         right = vec3_normalize(vec3_cross(front, up)),
         newUp = vec3_cross(right, front);
    mat4 ret = {{
        right.x, newUp.x, front.x, 0,
        right.y, newUp.y, front.y, 0,
        right.z, newUp.z, front.z, 0,
        -vec3_dot(right, position), -vec3_dot(newUp, position), -vec3_dot(front, position), 1
    }};
    return ret;
}
mat4 mat4_perspective(float fovy, float aspect, float near, float far) {
    float tanHalfFovy = tanf(TO_RADIANS(fovy) / 2.0f);
    mat4 ret = {};
    ret.e[0][0] = 1.f / (aspect * tanHalfFovy);
    ret.e[1][1] = -1.f / (tanHalfFovy);
    ret.e[2][2] = -far / (far - near);
    ret.e[2][3] = -1.f;
    ret.e[3][2] = -(far * near) / (far - near);
    return ret;
}

mat4 mat4_orthographic_projection(float left, float right, float top, float bottom, float near, float far) {
    mat4 ret = {};
    ret.e[0][0] = 2.f / (right - left);
    ret.e[1][1] = 2.f / (bottom - top);
    ret.e[2][2] = 1.f / (far - near);
    ret.e[3][0] = -(right + left) / (right - left);
    ret.e[3][1] = -(bottom + top) / (bottom - top);
    ret.e[3][2] = -near / (far - near);
    return ret;
}

mat4 mat4_view_YXZ(vec3 position, vec3 rotation) {
    const float c1 = cosf(rotation.x);
    const float s1 = sinf(rotation.x);
    const float c2 = cosf(rotation.y);
    const float s2 = sinf(rotation.y);
    const float c3 = cosf(rotation.z);
    const float s3 = sinf(rotation.z);
    const vec3 u = {{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)}};
    const vec3 v = {{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)}};
    const vec3 w = {{(c2 * s1), (-s2), (c1 * c2)}};
    mat4 ret = {{
        u.x, v.x, w.x, 0,
        u.y, v.y, w.y, 0,
        u.z, v.z, w.z, 0,
        -vec3_dot(u, position), -vec3_dot(v, position), -vec3_dot(w, position), 1
    }};
  return ret;
}
