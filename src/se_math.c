// Syphax-Engine - Ougi Washi

#include "se_math.h"
#include <math.h>

f32 vec3_length(se_vec3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

se_mat4 mat4_perspective(float fov_y, float aspect, float near, float far) {
    float f = 1.0f / tanf(fov_y * 0.5f);
    se_mat4 P = { {
        f/aspect, 0, 0,                            0,
        0,        f, 0,                            0,
        0,        0, (far+near)/(near-far),       -1,
        0,        0, (2*far*near)/(near-far),      0
    } };
    return P;
}

se_vec3 vec3_sub(se_vec3 a, se_vec3 b){
    return (se_vec3){ a.x-b.x, a.y-b.y, a.z-b.z };
}

se_vec3 vec3_norm(se_vec3 v){
    const f32 len = vec3_length(v);
    return (se_vec3){ v.x/len, v.y/len, v.z/len };
}

se_vec3 vec3_cross(se_vec3 a, se_vec3 b){
    return (se_vec3){
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    };
}

// eye = camera pos, center = look at point, up = world up (e.g. {0,1,0})
se_mat4 mat4_look_at(se_vec3 eye, se_vec3 center, se_vec3 up) {
    se_vec3 f = vec3_norm(vec3_sub(center, eye));
    se_vec3 s = vec3_norm(vec3_cross(f, up));
    se_vec3 u = vec3_cross(s, f);

    se_mat4 M = mat4_identity();
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

se_mat4 mat4_mul(const se_mat4 A, const se_mat4 B) {
    se_mat4 R;
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

se_mat4 mat4_translate(const se_vec3* v) {
    se_mat4 T = mat4_identity();
    T.m[12] = v->x;
    T.m[13] = v->y;
    T.m[14] = v->z;
    return T;
}

se_mat4 mat4_rotate_x(se_mat4 m, f32 angle) {
    se_mat4 R = mat4_identity();
    R.m[5] =  cos(angle);
    R.m[6] = -sin(angle);
    R.m[9] =  sin(angle);
    R.m[10] = cos(angle);
    return mat4_mul(m, R);
}

se_mat4 mat4_rotate_y(se_mat4 m, f32 angle) {
    se_mat4 R = mat4_identity();
    R.m[0] =  cos(angle);
    R.m[2] =  sin(angle);
    R.m[8] = -sin(angle);
    R.m[10] = cos(angle);
    return mat4_mul(m, R);
}

se_mat4 mat4_rotate_z(se_mat4 m, f32 angle) {
    se_mat4 R = mat4_identity();
    R.m[0] =  cos(angle);
    R.m[1] = -sin(angle);
    R.m[4] =  sin(angle);
    R.m[5] =  cos(angle);
    return mat4_mul(m, R);
}

se_mat4 mat4_scale(const se_vec3* v) {
    se_mat4 S = mat4_identity();
    S.m[0] = v->x;
    S.m[5] = v->y;
    S.m[10] = v->z;
    return S;
}

