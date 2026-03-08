#include "emath.h"

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
    float c = cosf(angle),
        s = sinf(angle);
    // c -s (row major)
    // s  c
    mat4 ret = {{
        1, 0, 0, 0,
        0, c, s, 0,
        0,-s, c, 0,
        0, 0, 0, 1,
    }};
    return ret;
}
mat4 mat4_rotation_y(float angle) {
    float c = cosf(angle),
        s = sinf(angle);
    //  c s (row major)
    // -s c
    mat4 ret = {{
        c, 0,-s, 0,
        0, 1, 0, 0,
        s, 0, c, 0,
        0, 0, 0, 1,
    }};
    return ret;
}
mat4 mat4_rotation_z(float angle) {
    float c = cosf(angle),
        s = sinf(angle);
    // c -s (row major)
    // s  c
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
mat4 mat4_inverse(mat4 m);