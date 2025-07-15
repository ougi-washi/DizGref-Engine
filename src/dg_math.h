// DizGref-Engine - Ougi Washi

#ifndef DG_MATH_H
#define DG_MATH_H

#include "dg_types.h"


typedef struct { f32 m[16]; } dg_mat4;
typedef struct { f32 x, y, z; } dg_vec3;
typedef struct { f32 x, y, z, w; } dg_vec4;
typedef struct { f32 x, y; } dg_vec2;


static dg_mat4 mat4_identity(void) {
    dg_mat4 I = { {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    } };
    return I;
}


// Math (TODO: Change all args to ptrs for performance)
#define PI 3.14159265359
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
f32 vec3_length(dg_vec3 v);
dg_mat4 mat4_perspective(float fov_y, float aspect, float near, float far);
dg_vec3 vec3_sub(dg_vec3 a, dg_vec3 b);
dg_vec3 vec3_norm(dg_vec3 v);
dg_vec3 vec3_cross(dg_vec3 a, dg_vec3 b);
dg_mat4 mat4_look_at(dg_vec3 eye, dg_vec3 center, dg_vec3 up);
dg_mat4 mat4_mul(const dg_mat4 A, const dg_mat4 B);
dg_mat4 mat4_translate(const dg_vec3* v); 
dg_mat4 mat4_rotate_x(dg_mat4 m, f32 angle);
dg_mat4 mat4_rotate_y(dg_mat4 m, f32 angle);
dg_mat4 mat4_rotate_z(dg_mat4 m, f32 angle);
dg_mat4 mat4_scale(const dg_vec3* v);







#endif // DG_MATH_H
