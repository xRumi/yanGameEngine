#include "emath.h"

vec3 vec3_add(vec3 a, vec3 b) {
    vec3 ret = {{
        a.ele[0] + b.ele[0],
        a.ele[1] + b.ele[1],
        a.ele[2] + b.ele[2]
    }};
    return ret;
}
vec3 vec3_sub(vec3 a, vec3 b) {
    vec3 ret = {{
        a.ele[0] - b.ele[0],
        a.ele[1] - b.ele[1],
        a.ele[2] - b.ele[2]
    }};
    return ret;
}
vec3 vec3_scale(vec3 a, float scale) {
    vec3 ret = {{
        a.ele[0] * scale,
        a.ele[1] * scale,
        a.ele[2] * scale
    }};
    return ret;
}
float vec3_dot(vec3 a, vec3 b) {
    float ret =
        a.ele[0] * b.ele[0] +
        a.ele[1] * b.ele[1] +
        a.ele[2] * b.ele[2];
    return ret;
}
vec3 vec3_cross(vec3 a, vec3 b) {
    vec3 ret = {{
        a.ele[1]*b.ele[2] - a.ele[2]*b.ele[1],
       -a.ele[0]*b.ele[2] + a.ele[2]*b.ele[0],
        a.ele[0]*b.ele[1] - a.ele[1]*b.ele[0]
    }};
    return ret;
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
    float x2 = a.ele[0]*a.ele[0],
          y2 = a.ele[1]*a.ele[1],
          z2 = a.ele[2]*a.ele[2];
    return (x2 + y2 + z2);
}
float vec3_length(vec3 a) {
    return sqrt(vec3_length_sqr(a));
}

vec4 vec3_to_vec4(vec3 a, float w) {
    vec4 ret = {{
        a.x, a.y, a.z, w
    }};
    return ret;
}

mat4 mat4_identity() {
    mat4 ret = {{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    }};
    return ret;
};

mat4 mat4_translation(float x, float y, float z) {
    mat4 ret = {{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, z, 1,
    }};
    return ret;
}
mat4 mat4_scale(float x, float y, float z) {
    mat4 ret = {{
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1,
    }};
    return ret;
}
mat4 mat4_rotation_x(float angle) {
    angle = TO_RADIANS(angle);
    float c = cosf(angle),
        s = sinf(angle);
    mat4 ret = {{
        1, 0, 0, 0,
        0, c, s, 0,
        0,-s, c, 0,
        0, 0, 0, 1,
    }};
    return ret;
}
mat4 mat4_rotation_y(float angle) {
    angle = TO_RADIANS(angle);
    float c = cosf(angle),
        s = sinf(angle);
    mat4 ret = {{
        c, 0,-s, 0,
        0, 1, 0, 0,
        s, 0, c, 0,
        0, 0, 0, 1,
    }};
    return ret;
}
mat4 mat4_rotation_z(float angle) {
    angle = TO_RADIANS(angle);
    float c = cosf(angle),
        s = sinf(angle);
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
            ret.ele[col*4 + row] =
                a.ele[0*4 + row] * b.ele[col*4 + 0] +
                a.ele[1*4 + row] * b.ele[col*4 + 1] +
                a.ele[2*4 + row] * b.ele[col*4 + 2] +
                a.ele[3*4 + row] * b.ele[col*4 + 3];
    return ret;
}
vec4 mat4_mul_vec4(mat4 m, vec4 v) {
    vec4 ret = {{
        m.ele[0]*v.ele[0] + m.ele[4]*v.ele[1] + m.ele[8]*v.ele[2] + m.ele[12]*v.ele[3],
        m.ele[1]*v.ele[0] + m.ele[5]*v.ele[1] + m.ele[9]*v.ele[2] + m.ele[13]*v.ele[3],
        m.ele[2]*v.ele[0] + m.ele[6]*v.ele[1] + m.ele[10]*v.ele[2] + m.ele[14]*v.ele[3],
        m.ele[3]*v.ele[0] + m.ele[7]*v.ele[1] + m.ele[11]*v.ele[2] + m.ele[15]*v.ele[3]
    }};
    return ret;
};
mat4 mat4_transpose(mat4 m) {
    mat4 ret;
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            ret.ele[col * 4 + row] = m.ele[row * 4 + col];
    return ret;
}
mat4 mat4_inverse(mat4 m); // TODO: inverse matrix

mat4 mat4_look_at(vec3 cameraPos, vec3 cameraTarget, vec3 up) {
    vec3 front = vec3_normalize(vec3_sub(cameraTarget, cameraPos)),
         right = vec3_normalize(vec3_cross(front, up)),
         newUp = vec3_cross(right, front);
    mat4 ret = {{
        right.x, newUp.x, -front.x, 0,
        right.y, newUp.y, -front.y, 0,
        right.z, newUp.z, -front.z, 0,
        -vec3_dot(right, cameraPos), -vec3_dot(newUp, cameraPos), -vec3_dot(front, cameraPos), 1
    }};
    return ret;
}
mat4 mat4_perspective(float fov, float aspect, float near, float far) {
    float tanHalfFov = tanf(TO_RADIANS(fov) / 2.0f);
    mat4 res = {};
    res.e[0][0] = 1.0f / (aspect * tanHalfFov);
    res.e[1][1] = 1.0f / tanHalfFov;
    res.e[2][2] = far / (far - near);
    res.e[2][3] = 1.0f;
    res.e[3][2] = - (near * far) / (far - near);

    return res;
}
