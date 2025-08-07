// Capsian-Engine - Ougi Washi

#ifndef CE_RENDER_H
#define CE_RENDER_H

#include "ce_gl.h"
#include "ce_math.h"
#include <GLFW/glfw3.h>
#include <time.h>
#include <assert.h>
#include <pthread.h>

#define PI 3.14159265359

#define MAX_BUFFERS 8
#define MAX_UNIFORMS 32
#define MAX_TEXTURES 128
#define MAX_SHADERS 64
#define MAX_MESHES 64
#define MAX_VERTICES 65536
#define MAX_INDICES 65536
#define MAX_PATH_LENGTH 256

typedef struct {
    ce_vec3 position;
    ce_vec3 normal;
    ce_vec2 uv;
} ce_vertex;

typedef struct {
    GLuint program;
    GLuint vertex_shader;
    GLuint fragment_shader;
    char vertex_path[MAX_PATH_LENGTH];
    char fragment_path[MAX_PATH_LENGTH];
    time_t vertex_mtime;
    time_t fragment_mtime;
    b8 needs_reload;
} ce_shader;

typedef struct ce_texture {
    char path[MAX_PATH_LENGTH];
    GLuint id;
    i32 width;
    i32 height;
    i32 channels;
} ce_texture;

typedef struct {
    ce_vertex* vertices;
    u32* indices;
    u32 vertex_count;
    u32 index_count;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    ce_shader* shader;
    ce_mat4 matrix;  
} ce_mesh;

typedef struct {
    ce_mesh* meshes;
    u32 ce_mesh_count;
} ce_model;

typedef enum {
    ce_uniform_FLOAT,
    ce_uniform_VEC2,
    ce_uniform_VEC3,
    ce_uniform_VEC4,
    ce_uniform_INT,
    ce_uniform_TEXTURE
} ce_uniform_type;

typedef struct {
    char name[64];
    ce_uniform_type type;
    union {
        f32 f;
        ce_vec2 vec2;
        ce_vec3 vec3;
        ce_vec4 vec4;
        i32 i;
        GLuint texture;
    } value;
} ce_uniform;

typedef struct {
    GLuint framebuffer;
    GLuint texture;
    GLuint prev_framebuffer;
    GLuint prev_texture;
    GLuint depth_buffer;
    u32 width;
    u32 height;
} ce_render_buffer;

typedef struct {
    GLFWwindow* window;
    u32 window_width;
    u32 window_height;
    
    ce_render_buffer buffers[MAX_BUFFERS];
    u32 buffer_count;

    ce_texture textures[MAX_TEXTURES];
    u32 texture_count;
    
    ce_shader shaders[MAX_SHADERS];
    u32 ce_shader_count;
    
    ce_model* models;
    u32 ce_model_count;
    
    ce_uniform uniforms[MAX_UNIFORMS];
    u32 ce_uniform_count;
    
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
} ce_engine;

// Engine functions
extern b8 ce_engine_init(ce_engine* engine, u32 width, u32 height, const char* title);
extern void ce_engine_cleanup(ce_engine* engine);
extern b8 ce_engine_should_close(ce_engine* engine);
extern void ce_engine_update(ce_engine* engine);
extern void ce_engine_render_quad(ce_engine* engine);
extern void ce_engine_render(ce_engine* engine);
extern void ce_engine_clear();
extern void ce_engine_swap_buffers(ce_engine* engine);
extern void ce_engine_poll_events(ce_engine* engine);
extern void ce_engine_check_exit_keys(ce_engine* engine, i32* keys, i32 key_count);
extern b8 ce_engine_is_key_down(ce_engine* engine, i32 key);
extern void ce_enigne_set_fps(const f64 fps);
extern void ce_engine_sleep(const f64 seconds);

// Texture functions
typedef enum { CE_REPEAT, CE_CLAMP } ce_texture_wrap;
extern ce_texture* ce_texture_load(ce_engine* engine, const char* path, const ce_texture_wrap wrap);
extern void ce_texture_cleanup(ce_texture* texture);

// Shader functions
extern ce_shader* ce_shader_load(ce_engine* engine, const char* vertex_path, const char* fragment_path);
extern b8 ce_shader_reload_if_changed(ce_shader* shader);
extern void ce_shader_use(ce_engine* engine, ce_shader* shader, const b8 update_uniforms);
extern void ce_shader_cleanup(ce_shader* shader);
extern GLuint ce_shader_get_uniform_location(ce_shader* shader, const char* name);

// Mesh functions
extern void ce_mesh_translate(ce_mesh* mesh, const ce_vec3* v);
extern void ce_mesh_rotate(ce_mesh* mesh, const ce_vec3* v);
extern void ce_mesh_scale(ce_mesh* mesh, const ce_vec3* v);

// Model functions
extern b8 ce_model_load_obj(ce_model* model, const char* path, ce_shader** shaders, const sz ce_shader_count); 
extern void ce_model_render(ce_engine* engine, ce_model* model);
extern void ce_model_cleanup(ce_model* model);
extern void ce_model_translate(ce_model* model, const ce_vec3* v);
extern void ce_model_rotate(ce_model* model, const ce_vec3* v);
extern void ce_model_scale(ce_model* model, const ce_vec3* v);

// Buffer functions
extern b8 ce_render_buffer_create(ce_render_buffer* buffer, u32 width, u32 height);
extern void ce_render_buffer_bind(ce_render_buffer* buffer);
extern void ce_render_buffer_unbind(ce_render_buffer* buf);
extern void ce_render_buffer_cleanup(ce_render_buffer* buffer);

// Uniform functions
extern void ce_uniform_set_float(ce_engine* engine, const char* name, f32 value);
extern void ce_uniform_set_vec2(ce_engine* engine, const char* name, ce_vec2 value);
extern void ce_uniform_set_vec3(ce_engine* engine, const char* name, ce_vec3 value);
extern void ce_uniform_set_vec4(ce_engine* engine, const char* name, ce_vec4 value);
extern void ce_uniform_set_int(ce_engine* engine, const char* name, i32 value);
extern void ce_uniform_set_texture(ce_engine* engine, const char* name, GLuint texture);
extern void ce_uniform_set_buffer_texture(ce_engine* engine, const char* name, ce_render_buffer* buffer);
extern void ce_uniform_apply(ce_engine* engine, ce_shader* shader); // make sure the shader is in use

// Utility functions
extern f64 get_time(void);
extern f64 get_delta_time(const ce_engine* engine);
extern time_t get_file_mtime(const char* path);
extern char* load_file(const char* path);
extern void create_fullscreen_quad(GLuint* vao, GLuint* vbo);

#endif // CE_RENDER_H
