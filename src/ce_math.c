// Capsian-Engine - Ougi Washi

#include "ce_math.h"
#include <math.h>

f32 vec3_length(ce_vec3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

ce_mat4 mat4_perspective(float fov_y, float aspect, float near, float far) {
    float f = 1.0f / tanf(fov_y * 0.5f);
    ce_mat4 P = { {
        f/aspect, 0, 0,                            0,
        0,        f, 0,                            0,
        0,        0, (far+near)/(near-far),       -1,
        0,        0, (2*far*near)/(near-far),      0
    } };
    return P;
}

ce_vec3 vec3_sub(ce_vec3 a, ce_vec3 b){
    return (ce_vec3){ a.x-b.x, a.y-b.y, a.z-b.z };
}

ce_vec3 vec3_norm(ce_vec3 v){
    const f32 len = vec3_length(v);
    return (ce_vec3){ v.x/len, v.y/len, v.z/len };
}

ce_vec3 vec3_cross(ce_vec3 a, ce_vec3 b){
    return (ce_vec3){
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    };
}

// eye = camera pos, center = look at point, up = world up (e.g. {0,1,0})
ce_mat4 mat4_look_at(ce_vec3 eye, ce_vec3 center, ce_vec3 up) {
    ce_vec3 f = vec3_norm(vec3_sub(center, eye));
    ce_vec3 s = vec3_norm(vec3_cross(f, up));
    ce_vec3 u = vec3_cross(s, f);

    ce_mat4 M = mat4_identity();
    // rotation
    M.m[0] =  s.x; M.m[1] =  u.x; M.m[2] = -f.x;
    M.m[4] =  s.y; M.m[5] =  u.y; M.m[6] = -f.y;
    M.m[8] =  s.z; M.m[9] =  u.z; M.m[10]= -f.z;
    // translation
    M.m[12] = - (s.x*eye.x + s.y*eye.y + s.z*eye.z);
    M.m[13] = - (u.x*eye.x + u.y*eye.y + u.z*eye.z);
    M.m[14] =   (f.x*eye.x + f.y*eye.y + f.z*eye.z);
    return M;
}

ce_mat4 mat4_mul(const ce_mat4 A, const ce_mat4 B) {
    ce_mat4 R;
    for(int row=0; row<4; row++){
        for(int col=0; col<4; col++){
            float sum = 0.0f;
            for(int k=0; k<4; k++){
                sum += A.m[row + 4*k] * B.m[k + 4*col];
            }
            R.m[row + 4*col] = sum;
        }
    }
    return R;
}

ce_mat4 mat4_translate(const ce_vec3* v) {
    ce_mat4 T = mat4_identity();
    T.m[12] = v->x;
    T.m[13] = v->y;
    T.m[14] = v->z;
    return T;
}

ce_mat4 mat4_rotate_x(ce_mat4 m, f32 angle) {
    ce_mat4 R = mat4_identity();
    R.m[5] =  cos(angle);
    R.m[6] = -sin(angle);
    R.m[9] =  sin(angle);
    R.m[10] = cos(angle);
    return mat4_mul(m, R);
}

ce_mat4 mat4_rotate_y(ce_mat4 m, f32 angle) {
    ce_mat4 R = mat4_identity();
    R.m[0] =  cos(angle);
    R.m[2] =  sin(angle);
    R.m[8] = -sin(angle);
    R.m[10] = cos(angle);
    return mat4_mul(m, R);
}

ce_mat4 mat4_rotate_z(ce_mat4 m, f32 angle) {
    ce_mat4 R = mat4_identity();
    R.m[0] =  cos(angle);
    R.m[1] = -sin(angle);
    R.m[4] =  sin(angle);
    R.m[5] =  cos(angle);
    return mat4_mul(m, R);
}

ce_mat4 mat4_scale(const ce_vec3* v) {
    ce_mat4 S = mat4_identity();
    S.m[0] = v->x;
    S.m[5] = v->y;
    S.m[10] = v->z;
    return S;
}

