// DizGref-Engine - Ougi Washi

#ifndef DG_RENDER_H
#define DG_RENDER_H

#include "dg_types.h"
#include "dg_gl.h"
#include <GLFW/glfw3.h>
#include <time.h>
#include <assert.h>
#include <pthread.h>

#define PI 3.14159265359

#define MAX_BUFFERS 8
#define MAX_UNIFORMS 32
#define MAX_SHADERS 64
#define MAX_MESHES 64
#define MAX_VERTICES 65536
#define MAX_INDICES 65536
#define MAX_PATH_LENGTH 256

static dg_mat4 mat4_identity(void) {
    dg_mat4 I = { {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    } };
    return I;
}

typedef struct {
    dg_vec3 position;
    dg_vec3 normal;
    dg_vec2 uv;
} dg_vertex;

typedef struct {
    GLuint program;
    GLuint vertex_shader;
    GLuint fragment_shader;
    char vertex_path[MAX_PATH_LENGTH];
    char fragment_path[MAX_PATH_LENGTH];
    time_t vertex_mtime;
    time_t fragment_mtime;
    b8 needs_reload;
} dg_shader;

typedef struct {
    dg_vertex* vertices;
    u32* indices;
    u32 vertex_count;
    u32 index_count;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    dg_shader* shader;
    dg_mat4 matrix;  
} dg_mesh;

typedef struct {
    dg_mesh* meshes;
    u32 dg_mesh_count;
} dg_model;

typedef enum {
    dg_uniform_FLOAT,
    dg_uniform_VEC2,
    dg_uniform_VEC3,
    dg_uniform_VEC4,
    dg_uniform_INT,
    dg_uniform_TEXTURE
} dg_uniform_type;

typedef struct {
    char name[64];
    dg_uniform_type type;
    union {
        f32 f;
        dg_vec2 vec2;
        dg_vec3 vec3;
        dg_vec4 vec4;
        i32 i;
        GLuint texture;
    } value;
} dg_uniform;

typedef struct {
    GLuint framebuffer;
    GLuint texture;
    GLuint depth_buffer;
    u32 width;
    u32 height;
} dg_render_buffer;

typedef struct {
    GLFWwindow* window;
    u32 window_width;
    u32 window_height;
    
    dg_render_buffer buffers[MAX_BUFFERS];
    u32 buffer_count;
    
    dg_shader shaders[MAX_SHADERS];
    u32 dg_shader_count;
    
    dg_model* models;
    u32 dg_model_count;
    
    dg_uniform uniforms[MAX_UNIFORMS];
    u32 dg_uniform_count;
    
    f64 time;
    f64 delta_time;
    f64 last_frame_time;
    i32 frame_count;
    
    b8 keys[1024];
    f64 mouse_x, mouse_y;
    f64 mouse_dx, mouse_dy;
    b8 mouse_buttons[8];
    
    GLuint quad_vao;
    GLuint quad_vbo;
} dg_engine;

// Engine functions
b8 dg_engine_init(dg_engine* engine, u32 width, u32 height, const char* title);
void dg_engine_cleanup(dg_engine* engine);
b8 dg_engine_should_close(dg_engine* engine);
void dg_engine_update(dg_engine* engine);
void dg_engine_render_quad(dg_engine* engine);
void dg_engine_render(dg_engine* engine);
void dg_engine_clear();
void dg_engine_swap_buffers(dg_engine* engine);
void dg_engine_poll_events(dg_engine* engine);
void dg_engine_check_exit_keys(dg_engine* engine, i32* keys, i32 key_count);

// Shader functions
dg_shader* dg_shader_load(dg_engine* engine, const char* vertex_path, const char* fragment_path);
b8 dg_shader_reload_if_changed(dg_shader* shader);
void dg_shader_use(dg_engine* engine, dg_shader* shader, const b8 update_uniforms);
void dg_shader_cleanup(dg_shader* shader);
GLuint dg_shader_get_uniform_location(dg_shader* shader, const char* name);

// Mesh functions
void dg_mesh_translate(dg_mesh* mesh, const dg_vec3* v);
void dg_mesh_rotate(dg_mesh* mesh, const dg_vec3* v);
void dg_mesh_scale(dg_mesh* mesh, const dg_vec3* v);

// Model functions
b8 dg_model_load_obj(dg_model* model, const char* path, dg_shader** shaders, const sz dg_shader_count); 
void dg_model_render(dg_engine* engine, dg_model* model);
void dg_model_cleanup(dg_model* model);
void dg_model_translate(dg_model* model, const dg_vec3* v);
void dg_model_rotate(dg_model* model, const dg_vec3* v);
void dg_model_scale(dg_model* model, const dg_vec3* v);

// Buffer functions
b8 dg_render_buffer_create(dg_render_buffer* buffer, u32 width, u32 height);
void dg_render_buffer_bind(dg_render_buffer* buffer);
void dg_render_buffer_unbind(void);
void dg_render_buffer_cleanup(dg_render_buffer* buffer);

// Uniform functions
void dg_uniform_set_float(dg_engine* engine, const char* name, f32 value);
void dg_uniform_set_vec2(dg_engine* engine, const char* name, dg_vec2 value);
void dg_uniform_set_vec3(dg_engine* engine, const char* name, dg_vec3 value);
void dg_uniform_set_vec4(dg_engine* engine, const char* name, dg_vec4 value);
void dg_uniform_set_int(dg_engine* engine, const char* name, i32 value);
void dg_uniform_set_texture(dg_engine* engine, const char* name, GLuint texture);
void dg_uniform_set_buffer_texture(dg_engine* engine, const char* name, dg_render_buffer* buffer);
void dg_uniform_apply(dg_engine* engine, dg_shader* shader); // make sure the shader is in use

// Utility functions
f64 get_time(void);
f64 get_delta_time(const dg_engine* engine);
time_t get_file_mtime(const char* path);
char* load_file(const char* path);
void create_fullscreen_quad(GLuint* vao, GLuint* vbo);

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

#endif // DG_RENDER_H
