// Syphax-render_handle - Ougi Washi

#include "se_render.h"
#include "se_gl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Fullscreen quad vertex shader
static const char* quad_vertex_shader = 
"#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"layout (location = 1) in vec2 aTexCoord;\n"
"out vec2 TexCoord;\n"
"void main() {\n"
"   TexCoord = aTexCoord;\n"
"   gl_Position = vec4(aPos, 0.0, 1.0);\n"
"}\n";

static f64 se_target_fps = 60.0;

static GLuint compile_shader(const char* source, GLenum type);
static GLuint create_shader_program(const char* vertex_source, const char* fragment_source);

void se_render_clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void se_render_set_background_color(const se_vec4 color) {
    glClearColor(color.x, color.y, color.z, color.w);
}

void se_render_handle_cleanup(se_render_handle* render_handle) {
    // Cleanup textures
    se_foreach(se_textures, render_handle->textures, i) {
        se_texture* curr_texture = se_textures_get(&render_handle->textures, i);
        se_texture_cleanup(curr_texture);
    }

    // Cleanup shaders
    se_foreach(se_shaders, render_handle->shaders, i) {
        se_shader* curr_shader = se_shaders_get(&render_handle->shaders, i);
        se_shader_cleanup(curr_shader);
    }
   
    // Cleanup models
    for (u32 i = 0; i < render_handle->se_model_count; i++) {
        se_model_cleanup(&render_handle->models[i]);
    }
    free(render_handle->models);
    
    // Cleanup buffers
    se_foreach(se_render_buffers, render_handle->render_buffers, i) {
        se_render_buffer* curr_buffer = se_render_buffers_get(&render_handle->render_buffers, i);
        se_render_buffer_cleanup(curr_buffer);
    }
    
    se_window_destroy_all();
}

void se_render_handle_reload_changed_shaders(se_render_handle* render_handle) {
    se_foreach(se_shaders, render_handle->shaders, i) {
        se_shader_reload_if_changed(se_shaders_get(&render_handle->shaders, i));
    }
}

se_uniforms* se_render_handle_get_global_uniforms(se_render_handle* render_handle) {
    return &render_handle->global_uniforms;
}

// Shader functions

char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); return NULL; }
    fseek(f, 0, SEEK_END);
    size_t len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = '\0';
    fclose(f);
    return buf;
}

se_texture* se_texture_load(se_render_handle* render_handle, const char* file_path, const se_texture_wrap wrap) {
    stbi_set_flip_vertically_on_load(1);

    se_texture* texture = se_textures_increment(&render_handle->textures);
    
    const c8 full_path[MAX_PATH_LENGTH] = RESOURCES_DIR;
    strncat((c8*)full_path, file_path, MAX_PATH_LENGTH - strlen(full_path) - 1);

    unsigned char* pixels = stbi_load(full_path, &texture->width, &texture->height, &texture->channels, 0);
    if (!pixels) {
        fprintf(stderr, "Error: could not load image %s\n", file_path);
        return 0;
    }

    glGenTextures(1, &texture->id);
    glActiveTexture(GL_TEXTURE0);             // always bind to unit 0 by default
    glBindTexture(GL_TEXTURE_2D, texture->id);

    // Upload to GPU
    GLenum format = (texture->channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, texture->width, texture->height, 0, format, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set filtering/wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (wrap == SE_CLAMP) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
    } else if (wrap == SE_REPEAT) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
    }
    
    stbi_image_free(pixels);
    return texture;
}

void se_texture_cleanup(se_texture* texture){
    glDeleteTextures(1, &texture->id);
    texture->id = 0;
    texture->width = 0;
    texture->height = 0;
    texture->channels = 0;
    texture->path[0] = '\0';
}

b8 se_shader_load_internal(se_shader* shader) {
   
    assert(shader);

    se_shader_cleanup(shader);
    
    char* vertex_source = load_file(shader->vertex_path);
    char* fragment_source = load_file(shader->fragment_path);
    
    if (!vertex_source || !fragment_source) {
        free(vertex_source);
        free(fragment_source);
        return false;
    }
    shader->program = create_shader_program(vertex_source, fragment_source);
    printf("Shader - created program: %d, from %s, %s\n", shader->program, shader->vertex_path, shader->fragment_path);

    free(vertex_source);
    free(fragment_source);
    
    if (!shader->program) {
        return false;
    }
    
    shader->vertex_mtime = get_file_mtime(shader->vertex_path);
    shader->fragment_mtime = get_file_mtime(shader->fragment_path);
    printf("Shader - loaded: %s, %s\n", shader->vertex_path, shader->fragment_path);
    return true;
}

se_shader* se_shader_load(se_render_handle* render_handle, const char* vertex_path, const char* fragment_path) {
    se_shader* new_shader = se_shaders_increment(&render_handle->shaders);
    // make path absolute
    char* new_vertex_path = NULL;
    char* new_fragment_path = NULL;
    
    if (strlen(vertex_path) > 0) {
        new_vertex_path = malloc(strlen(RESOURCES_DIR) + strlen(vertex_path) + 1);
        strcpy(new_vertex_path, RESOURCES_DIR);
        strcat(new_vertex_path, vertex_path);
    }
    
    if (strlen(fragment_path) > 0) {
        new_fragment_path = malloc(strlen(RESOURCES_DIR) + strlen(fragment_path) + 1);
        strcpy(new_fragment_path, RESOURCES_DIR);
        strcat(new_fragment_path, fragment_path);
    }

    strcpy(new_shader->vertex_path, new_vertex_path);
    strcpy(new_shader->fragment_path, new_fragment_path);
    free(new_vertex_path);
    free(new_fragment_path);
    
    if (se_shader_load_internal(new_shader)) {
        return new_shader;
    }
    return NULL;
}

b8 se_shader_reload_if_changed(se_shader* shader) {
    if (strlen(shader->vertex_path) == 0 || strlen(shader->fragment_path) == 0) {
        return false;
    }
    
    time_t vertex_mtime = get_file_mtime(shader->vertex_path);
    time_t fragment_mtime = get_file_mtime(shader->fragment_path);
    
    if (vertex_mtime != shader->vertex_mtime || fragment_mtime != shader->fragment_mtime) {
        printf("Reloading shader: %s, %s\n", shader->vertex_path, shader->fragment_path);
        return se_shader_load_internal(shader);
    }
    
    return false;
}

void se_shader_use(se_render_handle* render_handle, se_shader* shader, const b8 update_uniforms) {
    glUseProgram(shader->program);
    if (update_uniforms) {
        se_uniform_apply(render_handle, shader);
    }
}

void se_shader_cleanup(se_shader* shader) {
    if (shader->program) {
        glDeleteProgram(shader->program);
        shader->program = 0;
    }
}

GLuint se_shader_get_uniform_location(se_shader* shader, const char* name) {
    return glGetUniformLocation(shader->program, name);
}

void se_shader_set_float(se_shader* shader, const char* name, f32 value){
    se_uniform_set_float(&shader->uniforms, name, value);
}

void se_shader_set_vec2(se_shader* shader, const char* name, const se_vec2* value){
    se_uniform_set_vec2(&shader->uniforms, name, value);
}

void se_shader_set_vec3(se_shader* shader, const char* name, const se_vec3* value){
    se_uniform_set_vec3(&shader->uniforms, name, value);
}

void se_shader_set_vec4(se_shader* shader, const char* name, const se_vec4* value){
    se_uniform_set_vec4(&shader->uniforms, name, value);
}

void se_shader_set_int(se_shader* shader, const char* name, i32 value){
    se_uniform_set_int(&shader->uniforms, name, value);
}

void se_shader_set_texture(se_shader* shader, const char* name, GLuint texture){
    se_uniform_set_texture(&shader->uniforms, name, texture);
}

void se_shader_set_buffer_texture(se_shader* shader, const char* name, se_render_buffer* buffer){
    se_uniform_set_buffer_texture(&shader->uniforms, name, buffer);
}

// Mesh functions
void se_mesh_translate(se_mesh* mesh, const se_vec3* v){
    mesh->matrix = mat4_mul(mesh->matrix, mat4_translate(v));
}

void se_mesh_rotate(se_mesh* mesh, const se_vec3* v){
    mesh->matrix = mat4_mul(mesh->matrix, mat4_rotate_x(mat4_identity(), v->x));
    mesh->matrix = mat4_mul(mesh->matrix, mat4_rotate_y(mat4_identity(), v->y));
    mesh->matrix = mat4_mul(mesh->matrix, mat4_rotate_z(mat4_identity(), v->z));
}

void se_mesh_scale(se_mesh* mesh, const se_vec3* v){
    mesh->matrix = mat4_mul(mesh->matrix, mat4_scale(v));
}

// Helper function to finalize a mesh
void finalize_mesh(se_mesh* mesh, se_vertex* vertices, u32* indices, u32 vertex_count, u32 index_count, 
                   se_shader** shaders, sz se_shader_count, u32 se_mesh_index) {
// Allocate mesh data
    mesh->vertices = malloc(vertex_count * sizeof(se_vertex));
    mesh->indices = malloc(index_count * sizeof(u32));
    memcpy(mesh->vertices, vertices, vertex_count * sizeof(se_vertex));
    memcpy(mesh->indices, indices, index_count * sizeof(u32));
    mesh->vertex_count = vertex_count;
    mesh->index_count = index_count;
    mesh->matrix = mat4_identity();
    
    // Assign shader (cycle through available shaders)
    if (se_shader_count > 0) {
        mesh->shader = shaders[se_mesh_index % se_shader_count];
    } else {
        mesh->shader = NULL;
        fprintf(stderr, "No shaders provided for mesh %u\n", se_mesh_index);
    }
    
    // Create OpenGL objects
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);
    
    glBindVertexArray(mesh->vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(se_vertex), mesh->vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(u32), mesh->indices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(se_vertex), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(se_vertex), (void*)offsetof(se_vertex, normal));
    glEnableVertexAttribArray(1);
    
    // UV attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(se_vertex), (void*)offsetof(se_vertex, uv));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
}



b8 se_model_load_obj(se_model* model, const char* path, se_shader** shaders, const sz se_shader_count) {
    char full_path[MAX_PATH_LENGTH];
    strncpy(full_path, RESOURCES_DIR, MAX_PATH_LENGTH - 1);
    strncat(full_path, path, MAX_PATH_LENGTH - strlen(full_path) - 1);
    
    FILE* file = fopen(full_path, "r");
    if (!file) {
        fprintf(stderr, "Failed to open OBJ file: %s\n", path);
        return false;
    }

    
    // Arrays for temporary storage (shared across all meshes)
    se_vec3* temp_vertices = malloc(SE_MAX_VERTICES * sizeof(se_vec3));
    se_vec3* temp_normals = malloc(SE_MAX_VERTICES * sizeof(se_vec3));
    se_vec2* temp_uvs = malloc(SE_MAX_VERTICES * sizeof(se_vec2));
    
    u32 vertex_count = 0;
    u32 normal_count = 0;
    u32 uv_count = 0;
    
    // Dynamic array for meshes
    
    // Current mesh data
    se_vertex* current_vertices = malloc(SE_MAX_VERTICES * sizeof(se_vertex));
    u32* current_indices = malloc(SE_MAX_INDICES * sizeof(u32));
    u32 current_vertex_count = 0;
    u32 current_index_count = 0;
    
    char line[256];
    char current_object[256] = "default";
    b8 hse_faces = false;
    
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            // Vertex position
            sscanf(line, "v %f %f %f", &temp_vertices[vertex_count].x, 
                   &temp_vertices[vertex_count].y, &temp_vertices[vertex_count].z);
            vertex_count++;
        } else if (strncmp(line, "vn ", 3) == 0) {
            // Vertex normal
            sscanf(line, "vn %f %f %f", &temp_normals[normal_count].x,
                   &temp_normals[normal_count].y, &temp_normals[normal_count].z);
            normal_count++;
        } else if (strncmp(line, "vt ", 3) == 0) {
            // Vertex texture coordinate
            sscanf(line, "vt %f %f", &temp_uvs[uv_count].x, &temp_uvs[uv_count].y);
            uv_count++;
        } else if (strncmp(line, "o ", 2) == 0 || strncmp(line, "g ", 2) == 0) {
            // New object/group - finalize current mesh if it has faces
            if (hse_faces && current_vertex_count > 0) {
                se_mesh* new_mesh = se_meshes_increment(&model->meshes);
                const sz mesh_count = se_meshes_get_size(&model->meshes);
                finalize_mesh(new_mesh, current_vertices, current_indices, 
                             current_vertex_count, current_index_count, shaders, se_shader_count, mesh_count - 1);
                
                // Reset for next mesh
                current_vertex_count = 0;
                current_index_count = 0;
                hse_faces = false;
            }
            
            // Get new object name
            sscanf(line, "%*s %255s", current_object);
        } else if (strncmp(line, "f ", 2) == 0) {
            // Face
            hse_faces = true;
            u32 v1, v2, v3, n1, n2, n3, t1, t2, t3;
            i32 matches = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                                &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3);
            
            if (matches == 9) {
                // Create vertices for this face
                for (i32 i = 0; i < 3; i++) {
                    u32 vi = (i == 0) ? v1 - 1 : (i == 1) ? v2 - 1 : v3 - 1;
                    u32 ni = (i == 0) ? n1 - 1 : (i == 1) ? n2 - 1 : n3 - 1;
                    u32 ti = (i == 0) ? t1 - 1 : (i == 1) ? t2 - 1 : t3 - 1;
                    
                    // Check bounds
                    if (vi >= vertex_count || ni >= normal_count || ti >= uv_count) {
                        fprintf(stderr, "OBJ file contains invalid face indices\n");
                        continue;
                    }
                    
                    current_vertices[current_vertex_count].position = temp_vertices[vi];
                    current_vertices[current_vertex_count].normal = temp_normals[ni];
                    current_vertices[current_vertex_count].uv = temp_uvs[ti];
                    
                    current_indices[current_index_count] = current_vertex_count;
                    current_vertex_count++;
                    current_index_count++;
                }
            } else {
                // Try to parse face without texture coordinates (v//n format)
                matches = sscanf(line, "f %d//%d %d//%d %d//%d", &v1, &n1, &v2, &n2, &v3, &n3);
                if (matches == 6) {
                    for (i32 i = 0; i < 3; i++) {
                        u32 vi = (i == 0) ? v1 - 1 : (i == 1) ? v2 - 1 : v3 - 1;
                        u32 ni = (i == 0) ? n1 - 1 : (i == 1) ? n2 - 1 : n3 - 1;
                        
                        if (vi >= vertex_count || ni >= normal_count) {
                            fprintf(stderr, "OBJ file contains invalid face indices\n");
                            continue;
                        }
                        
                        current_vertices[current_vertex_count].position = temp_vertices[vi];
                        current_vertices[current_vertex_count].normal = temp_normals[ni];
                        current_vertices[current_vertex_count].uv = (se_vec2){0.0f, 0.0f}; // Default UV
                        
                        current_indices[current_index_count] = current_vertex_count;
                        current_vertex_count++;
                        current_index_count++;
                    }
                }
            }
        }
    }
    
    // Finalize the last mesh
    if (hse_faces && current_vertex_count > 0) {
        const sz mesh_count = se_meshes_get_size(&model->meshes);
        se_mesh* new_mesh = se_meshes_increment(&model->meshes);
        finalize_mesh(new_mesh, current_vertices, current_indices, 
                     current_vertex_count, current_index_count, shaders, se_shader_count, mesh_count - 1);
    }
    
    fclose(file);
    
    // If no meshes were created, create a default one
    if (se_meshes_get_size(&model->meshes) == 0) {
        fprintf(stderr, "No valid meshes found in OBJ file: %s\n", path);
        free(temp_vertices);
        free(temp_normals);
        free(temp_uvs);
        se_meshes_clear(&model->meshes);
        free(current_vertices);
        free(current_indices);
        return false;
    }
    
    // Cleanup temporary arrays
    free(temp_vertices);
    free(temp_normals);
    free(temp_uvs);
    free(current_vertices);
    free(current_indices);
    
    return true;
}

void se_model_render(se_render_handle* render_handle, se_model* model, se_camera* camera) {
    // set up global view/proj once per frame
    const se_mat4 proj = se_camera_get_projection_matrix(camera);
    const se_mat4 view = se_camera_get_view_matrix(camera);
    se_foreach(se_meshes, model->meshes, i) {
        se_mesh* mesh = se_meshes_get(&model->meshes, i);
        se_shader* sh = mesh->shader;

        //glUseProgram(sh->program);
        se_shader_use(render_handle, sh, true);

        se_mat4 vp  = mat4_mul(proj, view);
        se_mat4 mvp = mat4_mul(vp, mesh->matrix); 

        GLint loc_mvp = glGetUniformLocation(sh->program, "u_mvp");
        if (loc_mvp >= 0) {
            glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, mvp.m);
        }

        GLint loc_model = glGetUniformLocation(sh->program, "u_model");
        if (loc_model >= 0) {
            glUniformMatrix4fv(loc_model, 1, GL_FALSE, mesh->matrix.m);
        }

        // send to the GPU other uniforms (lights if forward rendering, etc)

        // draw
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);  
        glBindVertexArray(mesh->vao);
        glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
    }
    // unbind
    glBindVertexArray(0);
    glUseProgram(0);
}

void se_model_cleanup(se_model* model) {
    se_foreach(se_meshes, model->meshes, i) {
        se_mesh* mesh = se_meshes_get(&model->meshes, i);
        glDeleteVertexArrays(1, &mesh->vao);
        glDeleteBuffers(1, &mesh->vbo);
        glDeleteBuffers(1, &mesh->ebo);
        free(mesh->vertices);
        free(mesh->indices);
    }
    se_meshes_clear(&model->meshes);
}

void se_model_translate(se_model* model, const se_vec3* v){
    se_foreach(se_meshes, model->meshes, i) {
        se_mesh* mesh = se_meshes_get(&model->meshes, i);
        se_mesh_translate(mesh, v);
    }
}

void se_model_rotate(se_model* model, const se_vec3* v){
    se_foreach(se_meshes, model->meshes, i) {
        se_mesh* mesh = se_meshes_get(&model->meshes, i);
        se_mesh_rotate(mesh, v);
    }  
}

void se_model_scale(se_model* model, const se_vec3* v){
    se_foreach(se_meshes, model->meshes, i) {
        se_mesh* mesh = se_meshes_get(&model->meshes, i);
        se_mesh_scale(mesh, v);
    }
}

// camera functions
se_camera* se_camera_create(se_render_handle* render_handle) {
    se_camera* camera = se_cameras_increment(&render_handle->cameras);
    camera->position = (se_vec3){0, 0, 5};
    camera->target = (se_vec3){0, 0, 0};
    camera->up = (se_vec3){0, 1, 0};
    camera->right = (se_vec3){1, 0, 0};
    camera->fov = 45.0f;
    camera->near = 0.1f;
    camera->far = 100.0f;
    camera->aspect = 1.78; 
    return camera;
}

se_mat4 se_camera_get_view_matrix(const se_camera* camera) {
    return mat4_look_at(camera->position, camera->target, camera->up);
}

se_mat4 se_camera_get_projection_matrix(const se_camera* camera) {
    return mat4_perspective(camera->fov * (PI/180.0f), camera->aspect, camera->near, camera->far);
}

void se_camera_set_aspect(se_camera* camera, const f32 width, const f32 height) {
    camera->aspect = width / height;
}
void se_camera_destroy(se_render_handle* render_handle, se_camera* camera) {
    se_cameras_remove(&render_handle->cameras, camera);
}
 
// Buffer functions
b8 se_render_buffer_create(se_render_buffer* buffer, u32 width, u32 height) {
    buffer->width = width;
    buffer->height = height;
    
    // Create framebuffer
    glGenFramebuffers(1, &buffer->framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, buffer->framebuffer);
    
    // Create color texture
    glGenTextures(1, &buffer->texture);
    glBindTexture(GL_TEXTURE_2D, buffer->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->texture, 0);
    
    // Create depth buffer
    glGenRenderbuffers(1, &buffer->depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, buffer->depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffer->depth_buffer);
    
    // Create previous texture
    glGenTextures(1, &buffer->prev_texture);
    glBindTexture(GL_TEXTURE_2D, buffer->prev_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   
    // Create framebuffer for previous frame (for easy copying)
    glGenFramebuffers(1, &buffer->prev_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, buffer->prev_framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->prev_texture, 0);

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Framebuffer not complete!\n");
        se_render_buffer_cleanup(buffer);
        return false;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void se_render_buffer_copy_to_previous(se_render_buffer* buffer) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, buffer->framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, buffer->prev_framebuffer);
    
    glBlitFramebuffer(
        0, 0, buffer->width, buffer->height,  // Source rectangle
        0, 0, buffer->width, buffer->height,  // Destination rectangle
        GL_COLOR_BUFFER_BIT,                  // Copy color buffer
        GL_NEAREST                            // Use nearest filtering for exact copy
    );
    
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void se_render_buffer_bind(se_render_buffer* buffer) {
    se_render_buffer_copy_to_previous(buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, buffer->framebuffer);
    glViewport(0, 0, buffer->width, buffer->height);
}

void se_render_buffer_unbind(se_render_buffer* buf) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //GLuint tmp = buf->prev_texture;
    //buf->prev_texture = buf->texture;
    //buf->texture = tmp;
}

void se_render_buffer_cleanup(se_render_buffer* buffer) {
    if (buffer->framebuffer) {
        glDeleteFramebuffers(1, &buffer->framebuffer);
        buffer->framebuffer = 0;
    }
    if (buffer->texture) {
        glDeleteTextures(1, &buffer->texture);
        buffer->texture = 0;
    }
    if (buffer->depth_buffer) {
        glDeleteRenderbuffers(1, &buffer->depth_buffer);
        buffer->depth_buffer = 0;
    }
}

// Uniform functions
void se_uniform_set_float(se_uniforms* uniforms, const char* name, f32 value) {
    se_foreach(se_uniforms, *uniforms, i) {
        se_uniform* found_uniform = se_uniforms_get(uniforms, i);
        if (found_uniform && strcmp(found_uniform->name, name) == 0) {
            found_uniform->type = SE_UNIFORM_FLOAT;
            found_uniform->value.f = value;
            return;
        }
        else {
            se_uniform* new_uniform = se_uniforms_increment(uniforms);
            strncpy(new_uniform->name, name, sizeof(new_uniform->name) - 1);
            new_uniform->type = SE_UNIFORM_FLOAT;
            new_uniform->value.f = value;
            return;
        }
    }
}

void se_uniform_set_vec2(se_uniforms* uniforms, const char* name, const se_vec2* value) {
    se_foreach(se_uniforms, *uniforms, i) {
        se_uniform* found_uniform = se_uniforms_get(uniforms, i);
        if (found_uniform && strcmp(found_uniform->name, name) == 0) {
            found_uniform->type = SE_UNIFORM_VEC2;
            memcpy(&found_uniform->value.vec2, value, sizeof(se_vec2));
            return;
        }
        else {
            se_uniform* new_uniform = se_uniforms_increment(uniforms);
            strncpy(new_uniform->name, name, sizeof(new_uniform->name) - 1);
            new_uniform->type = SE_UNIFORM_VEC2;
            memcpy(&new_uniform->value.vec2, value, sizeof(se_vec2));
            return;
        }
    }
}

void se_uniform_set_vec3(se_uniforms* uniforms, const char* name, const se_vec3* value) {
    se_foreach(se_uniforms, *uniforms, i) {
        se_uniform* found_uniform = se_uniforms_get(uniforms, i);
        if (found_uniform && strcmp(found_uniform->name, name) == 0) {
            found_uniform->type = SE_UNIFORM_VEC3;
            memcpy(&found_uniform->value.vec3, value, sizeof(se_vec3));
            return;
        }
        else {
            se_uniform* new_uniform = se_uniforms_increment(uniforms);
            strncpy(new_uniform->name, name, sizeof(new_uniform->name) - 1);
            new_uniform->type = SE_UNIFORM_VEC3;
            memcpy(&new_uniform->value.vec3, value, sizeof(se_vec3));
            return;
        }
    }
}

void se_uniform_set_vec4(se_uniforms* uniforms, const char* name, const se_vec4* value) {
    se_foreach(se_uniforms, *uniforms, i) {
        se_uniform* found_uniform = se_uniforms_get(uniforms, i);
        if (found_uniform && strcmp(found_uniform->name, name) == 0) {
            found_uniform->type = SE_UNIFORM_VEC4;
            memcpy(&found_uniform->value.vec4, value, sizeof(se_vec4));
            return;
        }
        else {
            se_uniform* new_uniform = se_uniforms_increment(uniforms);
            strncpy(new_uniform->name, name, sizeof(new_uniform->name) - 1);
            new_uniform->type = SE_UNIFORM_VEC4;
            memcpy(&new_uniform->value.vec4, value, sizeof(se_vec4));
            return;
        }
    }
}           

void se_uniform_set_int(se_uniforms* uniforms, const char* name, i32 value) {
    se_foreach(se_uniforms, *uniforms, i) {
        se_uniform* found_uniform = se_uniforms_get(uniforms, i);
        if (found_uniform && strcmp(found_uniform->name, name) == 0) {
            found_uniform->type = SE_UNIFORM_INT;
            found_uniform->value.i = value;
            return;
        }
        else {
            se_uniform* new_uniform = se_uniforms_increment(uniforms);
            strncpy(new_uniform->name, name, sizeof(new_uniform->name) - 1);
            new_uniform->type = SE_UNIFORM_INT;
            new_uniform->value.i = value;
            return;
        }
    }
}

void se_uniform_set_texture(se_uniforms* uniforms, const char* name, GLuint texture) {
    se_foreach(se_uniforms, *uniforms, i) {
        se_uniform* found_uniform = se_uniforms_get(uniforms, i);
        if (found_uniform && strcmp(found_uniform->name, name) == 0) {
            found_uniform->type = SE_UNIFORM_TEXTURE;
            found_uniform->value.texture = texture;
            return;
            
        }
        else {
            se_uniform* new_uniform = se_uniforms_increment(uniforms);
            strncpy(new_uniform->name, name, sizeof(new_uniform->name) - 1);
            new_uniform->type = SE_UNIFORM_TEXTURE;
            new_uniform->value.texture = texture;
            return;
        }
    }
}

void se_uniform_set_buffer_texture(se_uniforms* uniforms, const char* name, se_render_buffer* buffer) {
    se_uniform_set_texture(uniforms, name, buffer->texture);
}

void se_uniform_apply(se_render_handle* render_handle, se_shader* shader) {
    glUseProgram(shader->program);
    u32 texture_unit = 0;
    se_foreach(se_uniforms, shader->uniforms, i) {
        se_uniform* uniform = se_uniforms_get(&shader->uniforms, i);
        GLint location = glGetUniformLocation(shader->program, uniform->name); 
        if (location == -1) {
            continue;
        };
        switch (uniform->type) {
            case SE_UNIFORM_FLOAT:
                glUniform1fv(location, 1, &uniform->value.f);
                break;
            case SE_UNIFORM_VEC2:
                glUniform2fv(location, 1, &uniform->value.vec2.x);
                break;
            case SE_UNIFORM_VEC3:
                glUniform3fv(location, 1, &uniform->value.vec3.x);
                break;
            case SE_UNIFORM_VEC4:
                glUniform4fv(location, 1, &uniform->value.vec4.x);
                break;
            case SE_UNIFORM_INT:
                glUniform1i(location, uniform->value.i);
                break;
            case SE_UNIFORM_TEXTURE:
                glActiveTexture(GL_TEXTURE0 + texture_unit);
                glBindTexture(GL_TEXTURE_2D, uniform->value.texture);
                glUniform1i(location, texture_unit);
                texture_unit++;
                break;
        }
    }
    
    // apply global uniforms
    se_uniforms* global_uniforms = se_render_handle_get_global_uniforms(render_handle);
    se_foreach(se_uniforms, *global_uniforms, i) {
        se_uniform* uniform = se_uniforms_get(global_uniforms, i);
        GLint location = glGetUniformLocation(shader->program, uniform->name); 
        if (location == -1) {
            continue;
        };
        switch (uniform->type) {
            case SE_UNIFORM_FLOAT:
                glUniform1fv(location, 1, &uniform->value.f);
                break;
            case SE_UNIFORM_VEC2:
                glUniform2fv(location, 1, &uniform->value.vec2.x);
                break;
            case SE_UNIFORM_VEC3:
                glUniform3fv(location, 1, &uniform->value.vec3.x);
                break;
            case SE_UNIFORM_VEC4:
                glUniform4fv(location, 1, &uniform->value.vec4.x);
                break;
            case SE_UNIFORM_INT:
                glUniform1i(location, uniform->value.i);
                break;
            case SE_UNIFORM_TEXTURE:
                glActiveTexture(GL_TEXTURE0 + texture_unit);
                glBindTexture(GL_TEXTURE_2D, uniform->value.texture);
                glUniform1i(location, texture_unit);
                texture_unit++;
                break;
        }
    }
}

time_t get_file_mtime(const char* path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return st.st_mtime;
    }
    return 0;
}

char* load_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", path);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* buffer = malloc(size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    
    fread(buffer, 1, size, file);
    buffer[size] = '\0';
    
    fclose(file);
    return buffer;
}

static GLuint compile_shader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        fprintf(stderr, "Shader compilation failed: %s\n", info_log);
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

static GLuint create_shader_program(const char* vertex_source, const char* fragment_source) {
    GLuint vertex_shader = compile_shader(vertex_source, GL_VERTEX_SHADER);
    GLuint fragment_shader = compile_shader(fragment_source, GL_FRAGMENT_SHADER);
    
    if (!vertex_shader || !fragment_shader) {
        if (vertex_shader) glDeleteShader(vertex_shader);
        if (fragment_shader) glDeleteShader(fragment_shader);
        return 0;
    }
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(program, 512, NULL, info_log);
        fprintf(stderr, "Shader program linking failed: %s\n", info_log);
        glDeleteProgram(program);
        program = 0;
    }
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    return program;
}

