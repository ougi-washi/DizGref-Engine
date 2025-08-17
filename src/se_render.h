// Syphax-render_handle - Ougi Washi

#ifndef SE_RENDER_H
#define SE_RENDER_H

#include "se_math.h"
#include "se_array.h"
#include <GLFW/glfw3.h>
#include <time.h>
#include <assert.h>
#include <pthread.h>

#define SE_MAX_RENDER_BUFFERS 16
#define SE_MAX_FRAMEBUFFERS 16
#define SE_MAX_UNIFORMS 32
#define SE_MAX_TEXTURES 128
#define SE_MAX_SHADERS 64
#define SE_MAX_MESHES 64
#define SE_MAX_MODELS 1024
#define SE_MAX_VERTICES 65536
#define SE_MAX_INDICES 65536
#define SE_MAX_NAME_LENGTH 64
#define SE_MAX_PATH_LENGTH 256
#define SE_MAX_CAMERAS 32 
#define SE_MAX_2D_OBJECTS 1024

#define SE_OBJECT_2D_VERTEX_SHADER_PATH "shaders/object_2d_vertex.glsl"

typedef struct {
    se_vec3 position;
    se_vec3 normal;
    se_vec2 uv;
} se_vertex;

typedef enum {
    SE_UNIFORM_FLOAT,
    SE_UNIFORM_VEC2,
    SE_UNIFORM_VEC3,
    SE_UNIFORM_VEC4,
    SE_UNIFORM_INT,
    SE_UNIFORM_TEXTURE
} se_uniform_type;

typedef struct {
    char name[SE_MAX_NAME_LENGTH];
    se_uniform_type type;
    union {
        f32 f;
        se_vec2 vec2;
        se_vec3 vec3;
        se_vec4 vec4;
        i32 i;
        GLuint texture;
    } value;
} se_uniform;
SE_DEFINE_ARRAY(se_uniform, se_uniforms, SE_MAX_UNIFORMS);

typedef struct {
    GLuint program;
    GLuint vertex_shader;
    GLuint fragment_shader;
    c8 vertex_path[SE_MAX_PATH_LENGTH];
    c8 fragment_path[SE_MAX_PATH_LENGTH];
    time_t vertex_mtime;
    time_t fragment_mtime;
    se_uniforms uniforms;
    b8 needs_reload;
} se_shader;
SE_DEFINE_ARRAY(se_shader, se_shaders, SE_MAX_SHADERS);
typedef se_shader* se_shader_ptr;
SE_DEFINE_ARRAY(se_shader_ptr, se_shaders_ptr, SE_MAX_SHADERS);

typedef struct se_texture {
    char path[SE_MAX_PATH_LENGTH];
    GLuint id;
    i32 width;
    i32 height;
    i32 channels;
} se_texture;
SE_DEFINE_ARRAY(se_texture, se_textures, SE_MAX_TEXTURES);
typedef se_texture* se_texture_ptr;
SE_DEFINE_ARRAY(se_texture_ptr, se_textures_ptr, SE_MAX_TEXTURES);

typedef struct {
    se_vertex* vertices;
    u32* indices;
    u32 vertex_count;
    u32 index_count;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    se_shader* shader;
    se_mat4 matrix;
} se_mesh;
SE_DEFINE_ARRAY(se_mesh, se_meshes, SE_MAX_MESHES);

typedef struct {
    se_meshes meshes;
} se_model;
SE_DEFINE_ARRAY(se_model, se_models, SE_MAX_MODELS);
typedef se_model* se_model_ptr;
SE_DEFINE_ARRAY(se_model_ptr, se_models_ptr, SE_MAX_MODELS);

typedef struct {
    se_vec3 position;
    se_vec3 target;
    se_vec3 up;
    se_vec3 right;
    f32 fov;
    f32 near;
    f32 far;
    f32 aspect;
} se_camera;
SE_DEFINE_ARRAY(se_camera, se_cameras, SE_MAX_CAMERAS);
typedef se_camera* se_camera_ptr;
SE_DEFINE_ARRAY(se_camera_ptr, se_cameras_ptr, SE_MAX_CAMERAS);

typedef struct {
    GLuint framebuffer;
    GLuint texture;
    GLuint depth_buffer;
    se_vec2 size;
} se_framebuffer;
SE_DEFINE_ARRAY(se_framebuffer, se_framebuffers, SE_MAX_FRAMEBUFFERS);
typedef se_framebuffer* se_framebuffer_ptr;
SE_DEFINE_ARRAY(se_framebuffer_ptr, se_framebuffers_ptr, SE_MAX_FRAMEBUFFERS);

typedef struct {
    GLuint framebuffer;
    GLuint texture;
    GLuint prev_framebuffer;
    GLuint prev_texture;
    GLuint depth_buffer;
    se_vec2 texture_size;
    se_vec2 scale;
    se_vec2 position;
    se_shader_ptr shader;
} se_render_buffer;
SE_DEFINE_ARRAY(se_render_buffer, se_render_buffers, SE_MAX_RENDER_BUFFERS);
typedef se_render_buffer* se_render_buffer_ptr;
SE_DEFINE_ARRAY(se_render_buffer_ptr, se_render_buffers_ptr, SE_MAX_RENDER_BUFFERS);

typedef struct se_object_2d {
    se_vec2 position;
    se_vec2 scale;
    se_shader_ptr shader;
} se_object_2d;
SE_DEFINE_ARRAY(se_object_2d, se_objects_2d, SE_MAX_2D_OBJECTS);
typedef se_object_2d* se_object_2d_ptr;
SE_DEFINE_ARRAY(se_object_2d_ptr, se_objects_2d_ptr, SE_MAX_2D_OBJECTS);

typedef struct {
    se_framebuffers framebuffers;
    se_render_buffers render_buffers;
    se_textures textures;
    se_shaders shaders;
    se_uniforms global_uniforms;
    se_cameras cameras;
    se_models models;
    se_objects_2d objects_2d;
} se_render_handle;

// helper functions
extern void se_enable_blending();
extern void se_disable_blending();
extern void se_render_clear();
extern void se_render_set_background_color(const se_vec4 color);

// render_handle functions
extern se_render_handle* se_render_handle_create();
extern void se_render_handle_cleanup(se_render_handle* render_handle);
extern void se_render_handle_reload_changed_shaders(se_render_handle* render_handle);
extern se_uniforms* se_render_handle_get_global_uniforms(se_render_handle* render_handle);

// Texture functions
typedef enum { SE_REPEAT, SE_CLAMP } se_texture_wrap;
extern se_texture* se_texture_load(se_render_handle* render_handle, const char* path, const se_texture_wrap wrap);
extern void se_texture_cleanup(se_texture* texture);

// Shader functions
extern se_shader* se_shader_load(se_render_handle* render_handle, const char* vertex_file_path, const char* fragment_file_path);
extern se_shader* se_shader_load_from_memory(se_render_handle* render_handle, const char* vertex_data, const char* fragment_data);
extern b8 se_shader_reload_if_changed(se_shader* shader);
extern void se_shader_use(se_render_handle* render_handle, se_shader* shader, const b8 update_uniforms);
extern void se_shader_cleanup(se_shader* shader);
extern GLuint se_shader_get_uniform_location(se_shader* shader, const char* name);
extern void se_shader_set_float(se_shader* shader, const char* name, f32 value);
extern void se_shader_set_vec2(se_shader* shader, const char* name, const se_vec2* value);
extern void se_shader_set_vec3(se_shader* shader, const char* name, const se_vec3* value);
extern void se_shader_set_vec4(se_shader* shader, const char* name, const se_vec4* value);
extern void se_shader_set_int(se_shader* shader, const char* name, i32 value);
extern void se_shader_set_texture(se_shader* shader, const char* name, GLuint texture);
extern void se_shader_set_buffer_texture(se_shader* shader, const char* name, se_render_buffer* buffer);

// Mesh functions
extern void se_mesh_translate(se_mesh* mesh, const se_vec3* v);
extern void se_mesh_rotate(se_mesh* mesh, const se_vec3* v);
extern void se_mesh_scale(se_mesh* mesh, const se_vec3* v);

// Model functions
extern se_model* se_model_load_obj(se_render_handle* render_handle, const char* path, se_shaders_ptr* shaders);
extern void se_model_render(se_render_handle* render_handle, se_model* model, se_camera* camera);
extern void se_model_cleanup(se_model* model);
extern void se_model_translate(se_model* model, const se_vec3* v);
extern void se_model_rotate(se_model* model, const se_vec3* v);
extern void se_model_scale(se_model* model, const se_vec3* v);

// camera functions
extern se_camera* se_camera_create(se_render_handle* render_handle); 
extern se_mat4 se_camera_get_view_matrix(const se_camera* camera);
extern se_mat4 se_camera_get_projection_matrix(const se_camera* camera);
extern void se_camera_set_aspect(se_camera* camera, const f32 width, const f32 height);
extern void se_camera_destroy(se_render_handle* render_handle, se_camera* camera);

// Framebuffer functions
extern se_framebuffer* se_framebuffer_create(se_render_handle* render_handle, const se_vec2* size);
extern void se_framebuffer_bind(se_framebuffer* framebuffer);
extern void se_framebuffer_unbind(se_framebuffer* framebuffer);
extern void se_framebuffer_cleanup(se_framebuffer* framebuffer);

// Render buffer functions
extern se_render_buffer* se_render_buffer_create(se_render_handle* render_handle, const u32 width, const u32 height, const c8* fragment_shader_path);
extern void se_render_buffer_set_shader(se_render_buffer* buffer, se_shader* shader);
extern void se_render_buffer_unset_shader(se_render_buffer* buffer);
extern void se_render_buffer_bind(se_render_buffer* buffer);
extern void se_render_buffer_unbind(se_render_buffer* buf);
extern void se_render_buffer_set_scale(se_render_buffer* buffer, const se_vec2* scale);
extern void se_render_buffer_set_position(se_render_buffer* buffer, const se_vec2* position);
extern void se_render_buffer_cleanup(se_render_buffer* buffer);

// Uniform functions
extern void se_uniform_set_float    (se_uniforms* uniforms, const char* name, f32 value);
extern void se_uniform_set_vec2     (se_uniforms* uniforms, const char* name, const se_vec2* value);
extern void se_uniform_set_vec3     (se_uniforms* uniforms, const char* name, const se_vec3* value);
extern void se_uniform_set_vec4     (se_uniforms* uniforms, const char* name, const se_vec4* value);
extern void se_uniform_set_int      (se_uniforms* uniforms, const char* name, i32 value);
extern void se_uniform_set_texture  (se_uniforms* uniforms, const char* name, GLuint texture);
extern void se_uniform_set_buffer_texture(se_uniforms* uniforms, const char* name, se_render_buffer* buffer);
extern void se_uniform_apply(se_render_handle* render_handle, se_shader* shader);

// 2D objects functions
extern se_object_2d* se_object_2d_create(se_render_handle* render_handle, const c8* fragment_shader_path, const se_vec2* position, const se_vec2* scale);
extern void se_object_2d_destroy(se_render_handle* render_handle, se_object_2d* object);
extern void se_object_2d_set_position(se_object_2d* object, const se_vec2* position);
extern void se_object_2d_set_scale(se_object_2d* object, const se_vec2* scale);
extern void se_object_2d_set_shader(se_object_2d* object, se_shader* shader);
extern void se_object_2d_update_uniforms(se_object_2d* object);

// Utility functions
extern time_t get_file_mtime(const char* path);
extern char* load_file(const char* path);

#endif // SE_RENDER_H
