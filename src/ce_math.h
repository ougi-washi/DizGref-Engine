// Capsian-Engine - Ougi Washi

#ifndef CE_MATH_H
#define CE_MATH_H

#include "ce_types.h"


typedef struct { f32 m[16]; } ce_mat4;
typedef struct { f32 x, y, z; } ce_vec3;
typedef struct { f32 x, y, z, w; } ce_vec4;
typedef struct { f32 x, y; } ce_vec2;


static ce_mat4 mat4_identity(void) {
    ce_mat4 I = { {
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
f32 vec3_length(ce_vec3 v);
ce_mat4 mat4_perspective(float fov_y, float aspect, float near, float far);
ce_vec3 vec3_sub(ce_vec3 a, ce_vec3 b);
ce_vec3 vec3_norm(ce_vec3 v);
ce_vec3 vec3_cross(ce_vec3 a, ce_vec3 b);
ce_mat4 mat4_look_at(ce_vec3 eye, ce_vec3 center, ce_vec3 up);
ce_mat4 mat4_mul(const ce_mat4 A, const ce_mat4 B);
ce_mat4 mat4_translate(const ce_vec3* v); 
ce_mat4 mat4_rotate_x(ce_mat4 m, f32 angle);
ce_mat4 mat4_rotate_y(ce_mat4 m, f32 angle);
ce_mat4 mat4_rotate_z(ce_mat4 m, f32 angle);
ce_mat4 mat4_scale(const ce_vec3* v);

#endif // CE_MATH_H
