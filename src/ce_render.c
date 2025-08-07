// Capsian-Engine - Ougi Washi

#include "ce_render.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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


static f64 ce_target_fps = 60.0;


// Forward declarations
static void key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods);
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
static void mouse_button_callback(GLFWwindow* window, i32 button, i32 action, i32 mods);
static void framebuffer_size_callback(GLFWwindow* window, i32 width, i32 height);
static GLuint compile_shader(const char* source, GLenum type);
static GLuint create_shader_program(const char* vertex_source, const char* fragment_source);

// Engine functions
b8 ce_engine_init(ce_engine* engine, u32 width, u32 height, const char* title) {
    memset(engine, 0, sizeof(ce_engine));
    
    // Initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return false;
    }
    
    // Set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwSwapInterval(1);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Create window
    engine->window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!engine->window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return false;
    }

    engine->window_width = width;
    engine->window_height = height;
    
    glfwMakeContextCurrent(engine->window);
    glfwSetWindowUserPointer(engine->window, engine);
    
    // Set callbacks
    glfwSetKeyCallback(engine->window, key_callback);
    glfwSetCursorPosCallback(engine->window, mouse_callback);
    glfwSetMouseButtonCallback(engine->window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(engine->window, framebuffer_size_callback);
    
    ce_init_opengl();
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Create fullscreen quad
    create_fullscreen_quad(&engine->quad_vao, &engine->quad_vbo);

    // Initialize timing
    engine->last_frame_time = get_time();
    
    return true;
}

void ce_engine_cleanup(ce_engine* engine) {
    // Cleanup textures
    for (u32 i = 0; i < engine->texture_count; i++) {
        ce_texture_cleanup(&engine->textures[i]);
    }

    // Cleanup shaders
    for (u32 i = 0; i < engine->ce_shader_count; i++) {
        ce_shader_cleanup(&engine->shaders[i]);
    }
   
    // Cleanup models
    for (u32 i = 0; i < engine->ce_model_count; i++) {
        ce_model_cleanup(&engine->models[i]);
    }
    free(engine->models);
    
    // Cleanup buffers
    for (u32 i = 0; i < engine->buffer_count; i++) {
        ce_render_buffer_cleanup(&engine->buffers[i]);
    }
    
    // Cleanup quad
    glDeleteVertexArrays(1, &engine->quad_vao);
    glDeleteBuffers(1, &engine->quad_vbo);
    
    // Cleanup GLFW
    glfwDestroyWindow(engine->window);
    glfwTerminate();
}

b8 ce_engine_should_close(ce_engine* engine) {
    return glfwWindowShouldClose(engine->window);
}

void ce_engine_update(ce_engine* engine) {
    double current_time = get_time();
    engine->delta_time = current_time - engine->last_frame_time;
    engine->last_frame_time = current_time;
    engine->time = current_time;
    engine->frame_count++;
    
    // Check for shader reloads
    for (u32 i = 0; i < engine->ce_shader_count; i++) {
        ce_shader_reload_if_changed(&engine->shaders[i]);
    }
    
    // Update built-in uniforms
    ce_uniform_set_float(engine, "time", engine->time);
    ce_uniform_set_float(engine, "delta_time", engine->delta_time);
    ce_uniform_set_int(engine, "frame", engine->frame_count);
    ce_uniform_set_vec2(engine, "resolution", (ce_vec2){engine->window_width, engine->window_height});
    ce_uniform_set_vec2(engine, "mouse", (ce_vec2){engine->mouse_x, engine->mouse_y});
}

void ce_engine_render_quad(ce_engine* engine) {
    glBindVertexArray(engine->quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void ce_engine_render(ce_engine* engine) {
    glViewport(0, 0, engine->window_width, engine->window_height);
    ce_engine_clear(); 
    ce_engine_render_quad(engine);
}

void ce_engine_clear(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ce_engine_swap_buffers(ce_engine* engine) {
    // 60fps
    f64 time_left = 1 / ce_target_fps - engine->delta_time;
    if (time_left > 0) {
        ce_engine_sleep(time_left);
    }
    
    glfwSwapBuffers(engine->window);
}

void ce_engine_poll_events(ce_engine* engine) {
    glfwPollEvents();
}

void ce_engine_check_exit_keys(ce_engine* engine, i32* keys, i32 key_count) {
    if (key_count == 0) {
        return;
    }
    for (i32 i = 0; i < key_count; i++) {
        if (!engine->keys[keys[i]]) {
            return;
        }
    }
    glfwSetWindowShouldClose(engine->window, GLFW_TRUE);
}

extern b8 ce_engine_is_key_down(ce_engine* engine, i32 key) {
    return engine->keys[key];
}


void ce_enigne_set_fps(const f64 fps) {
    ce_target_fps = fps;
}

void ce_engine_sleep(const f64 seconds) {
    usleep(seconds * 1000000);
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

ce_texture* ce_texture_load(ce_engine* engine, const char* file_path, const ce_texture_wrap wrap) {
    stbi_set_flip_vertically_on_load(1);

    engine->texture_count++;
    ce_texture* texture = &engine->textures[engine->texture_count - 1];
    
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
    if (wrap == CE_CLAMP) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
    } else if (wrap == CE_REPEAT) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
    }
    
    stbi_image_free(pixels);
    return texture;
}

void ce_texture_cleanup(ce_texture* texture){
    glDeleteTextures(1, &texture->id);
    texture->id = 0;
    texture->width = 0;
    texture->height = 0;
    texture->channels = 0;
    texture->path[0] = '\0';
}

b8 ce_shader_load_internal(ce_shader* shader) {
   
    assert(shader);

    ce_shader_cleanup(shader);
    
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

ce_shader* ce_shader_load(ce_engine* engine, const char* vertex_path, const char* fragment_path) {
    engine->ce_shader_count++;
    ce_shader* new_shader = &engine->shaders[engine->ce_shader_count - 1];
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
    
    if (ce_shader_load_internal(new_shader)) {
        return new_shader;
    }
    return NULL;
}

b8 ce_shader_reload_if_changed(ce_shader* shader) {
    if (strlen(shader->vertex_path) == 0 || strlen(shader->fragment_path) == 0) {
        return false;
    }
    
    time_t vertex_mtime = get_file_mtime(shader->vertex_path);
    time_t fragment_mtime = get_file_mtime(shader->fragment_path);
    
    if (vertex_mtime != shader->vertex_mtime || fragment_mtime != shader->fragment_mtime) {
        printf("Reloading shader: %s, %s\n", shader->vertex_path, shader->fragment_path);
        return ce_shader_load_internal(shader);
    }
    
    return false;
}

void ce_shader_use(ce_engine* engine, ce_shader* shader, const b8 update_uniforms) {
    glUseProgram(shader->program);
    if (update_uniforms) {
        ce_uniform_apply(engine, shader);
    }
}

void ce_shader_cleanup(ce_shader* shader) {
    if (shader->program) {
        glDeleteProgram(shader->program);
        shader->program = 0;
    }
}

GLuint ce_shader_get_uniform_location(ce_shader* shader, const char* name) {
    return glGetUniformLocation(shader->program, name);
}

// Mesh functions
void ce_mesh_translate(ce_mesh* mesh, const ce_vec3* v){
    mesh->matrix = mat4_mul(mesh->matrix, mat4_translate(v));
}

void ce_mesh_rotate(ce_mesh* mesh, const ce_vec3* v){
    mesh->matrix = mat4_mul(mesh->matrix, mat4_rotate_x(mat4_identity(), v->x));
    mesh->matrix = mat4_mul(mesh->matrix, mat4_rotate_y(mat4_identity(), v->y));
    mesh->matrix = mat4_mul(mesh->matrix, mat4_rotate_z(mat4_identity(), v->z));
}

void ce_mesh_scale(ce_mesh* mesh, const ce_vec3* v){
    mesh->matrix = mat4_mul(mesh->matrix, mat4_scale(v));
}

// Helper function to finalize a mesh
void finalize_mesh(ce_mesh* mesh, ce_vertex* vertices, u32* indices, u32 vertex_count, u32 index_count, 
                   ce_shader** shaders, sz ce_shader_count, u32 ce_mesh_index) {
// Allocate mesh data
    mesh->vertices = malloc(vertex_count * sizeof(ce_vertex));
    mesh->indices = malloc(index_count * sizeof(u32));
    memcpy(mesh->vertices, vertices, vertex_count * sizeof(ce_vertex));
    memcpy(mesh->indices, indices, index_count * sizeof(u32));
    mesh->vertex_count = vertex_count;
    mesh->index_count = index_count;
    mesh->matrix = mat4_identity();
    
    // Assign shader (cycle through available shaders)
    if (ce_shader_count > 0) {
        mesh->shader = shaders[ce_mesh_index % ce_shader_count];
    } else {
        mesh->shader = NULL;
        fprintf(stderr, "No shaders provided for mesh %u\n", ce_mesh_index);
    }
    
    // Create OpenGL objects
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);
    
    glBindVertexArray(mesh->vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(ce_vertex), mesh->vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(u32), mesh->indices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ce_vertex), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ce_vertex), (void*)offsetof(ce_vertex, normal));
    glEnableVertexAttribArray(1);
    
    // UV attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ce_vertex), (void*)offsetof(ce_vertex, uv));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
}



b8 ce_model_load_obj(ce_model* model, const char* path, ce_shader** shaders, const sz ce_shader_count) {
    char full_path[MAX_PATH_LENGTH];
    strncpy(full_path, RESOURCES_DIR, MAX_PATH_LENGTH - 1);
    strncat(full_path, path, MAX_PATH_LENGTH - strlen(full_path) - 1);
    
    FILE* file = fopen(full_path, "r");
    if (!file) {
        fprintf(stderr, "Failed to open OBJ file: %s\n", path);
        return false;
    }
    
    // Arrays for temporary storage (shared across all meshes)
    ce_vec3* temp_vertices = malloc(MAX_VERTICES * sizeof(ce_vec3));
    ce_vec3* temp_normals = malloc(MAX_VERTICES * sizeof(ce_vec3));
    ce_vec2* temp_uvs = malloc(MAX_VERTICES * sizeof(ce_vec2));
    
    u32 vertex_count = 0;
    u32 normal_count = 0;
    u32 uv_count = 0;
    
    // Dynamic array for meshes
    ce_mesh* meshes = malloc(MAX_MESHES * sizeof(ce_mesh));
    u32 ce_mesh_count = 0;
    
    // Current mesh data
    ce_vertex* current_vertices = malloc(MAX_VERTICES * sizeof(ce_vertex));
    u32* current_indices = malloc(MAX_INDICES * sizeof(u32));
    u32 current_vertex_count = 0;
    u32 current_index_count = 0;
    
    char line[256];
    char current_object[256] = "default";
    b8 hce_faces = false;
    
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
            if (hce_faces && current_vertex_count > 0) {
                finalize_mesh(&meshes[ce_mesh_count], current_vertices, current_indices, 
                             current_vertex_count, current_index_count, shaders, ce_shader_count, ce_mesh_count);
                ce_mesh_count++;
                
                // Reset for next mesh
                current_vertex_count = 0;
                current_index_count = 0;
                hce_faces = false;
            }
            
            // Get new object name
            sscanf(line, "%*s %255s", current_object);
        } else if (strncmp(line, "f ", 2) == 0) {
            // Face
            hce_faces = true;
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
                        current_vertices[current_vertex_count].uv = (ce_vec2){0.0f, 0.0f}; // Default UV
                        
                        current_indices[current_index_count] = current_vertex_count;
                        current_vertex_count++;
                        current_index_count++;
                    }
                }
            }
        }
    }
    
    // Finalize the last mesh
    if (hce_faces && current_vertex_count > 0) {
        finalize_mesh(&meshes[ce_mesh_count], current_vertices, current_indices, 
                     current_vertex_count, current_index_count, shaders, ce_shader_count, ce_mesh_count);
        ce_mesh_count++;
    }
    
    fclose(file);
    
    // If no meshes were created, create a default one
    if (ce_mesh_count == 0) {
        fprintf(stderr, "No valid meshes found in OBJ file: %s\n", path);
        free(temp_vertices);
        free(temp_normals);
        free(temp_uvs);
        free(meshes);
        free(current_vertices);
        free(current_indices);
        return false;
    }
    
    // Set up the model
    model->ce_mesh_count = ce_mesh_count;
    model->meshes = malloc(ce_mesh_count * sizeof(ce_mesh));
    memcpy(model->meshes, meshes, ce_mesh_count * sizeof(ce_mesh));
    
    // Cleanup temporary arrays
    free(temp_vertices);
    free(temp_normals);
    free(temp_uvs);
    free(meshes);
    free(current_vertices);
    free(current_indices);
    
    return true;
}

void ce_model_render(ce_engine* engine, ce_model* model) {
    // set up global view/proj once per frame
    ce_mat4 proj = mat4_perspective(  // 45° FOV
        45.0f * (3.14159f/180.0f),
        engine->window_width / engine->window_height,
        0.1f, 100.0f
    );
    ce_vec3 cam_pos    = { 0, 0,  5 };
    ce_vec3 cam_target = { 0, 0,  0 };
    ce_vec3 cam_up     = { 0, 1,  0 };
    ce_mat4 view = mat4_look_at(cam_pos, cam_target, cam_up);

    for(u32 i = 0; i < model->ce_mesh_count; i++) {
        ce_mesh* mesh = &model->meshes[i];
        ce_shader* sh = mesh->shader;

        //glUseProgram(sh->program);
        ce_shader_use(engine, sh, true);

        ce_mat4 vp  = mat4_mul(proj, view);
        ce_mat4 mvp = mat4_mul(vp, mesh->matrix); 

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

void ce_model_cleanup(ce_model* model) {
    for (u32 i = 0; i < model->ce_mesh_count; i++) {
        ce_mesh* mesh = &model->meshes[i];
        glDeleteVertexArrays(1, &mesh->vao);
        glDeleteBuffers(1, &mesh->vbo);
        glDeleteBuffers(1, &mesh->ebo);
        free(mesh->vertices);
        free(mesh->indices);
    }
    free(model->meshes);
    model->ce_mesh_count = 0;
}

void ce_model_translate(ce_model* model, const ce_vec3* v){
    for (u32 i = 0; i < model->ce_mesh_count; i++) {
        ce_mesh* mesh = &model->meshes[i];
        ce_mesh_translate(mesh, v);
    }
}

void ce_model_rotate(ce_model* model, const ce_vec3* v){
    for (u32 i = 0; i < model->ce_mesh_count; i++) {
        ce_mesh* mesh = &model->meshes[i];
        ce_mesh_rotate(mesh, v);
    }  
}

void ce_model_scale(ce_model* model, const ce_vec3* v){
    for (u32 i = 0; i < model->ce_mesh_count; i++) {
        ce_mesh* mesh = &model->meshes[i];
        ce_mesh_scale(mesh, v);
    }
}

// Buffer functions
b8 ce_render_buffer_create(ce_render_buffer* buffer, u32 width, u32 height) {
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
        ce_render_buffer_cleanup(buffer);
        return false;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void ce_render_buffer_copy_to_previous(ce_render_buffer* buffer) {
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

void ce_render_buffer_bind(ce_render_buffer* buffer) {
    ce_render_buffer_copy_to_previous(buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, buffer->framebuffer);
    glViewport(0, 0, buffer->width, buffer->height);
}

void ce_render_buffer_unbind(ce_render_buffer* buf) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //GLuint tmp = buf->prev_texture;
    //buf->prev_texture = buf->texture;
    //buf->texture = tmp;
}

void ce_render_buffer_cleanup(ce_render_buffer* buffer) {
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
void ce_uniform_set_float(ce_engine* engine, const char* name, f32 value) {
    for (u32 i = 0; i < engine->ce_uniform_count; i++) {
        if (strcmp(engine->uniforms[i].name, name) == 0) {
            engine->uniforms[i].type = ce_uniform_FLOAT;
            engine->uniforms[i].value.f = value;
            return;
        }
    }
    
    if (engine->ce_uniform_count < MAX_UNIFORMS) {
        ce_uniform* uniform = &engine->uniforms[engine->ce_uniform_count];
        strncpy(uniform->name, name, sizeof(uniform->name) - 1);
        uniform->type = ce_uniform_FLOAT;
        uniform->value.f = value;
        engine->ce_uniform_count++;
    }
}

void ce_uniform_set_vec2(ce_engine* engine, const char* name, ce_vec2 value) {
    for (u32 i = 0; i < engine->ce_uniform_count; i++) {
        if (strcmp(engine->uniforms[i].name, name) == 0) {
            engine->uniforms[i].type = ce_uniform_VEC2;
            engine->uniforms[i].value.vec2 = value;
            return;
        }
    }
    
    if (engine->ce_uniform_count < MAX_UNIFORMS) {
        ce_uniform* uniform = &engine->uniforms[engine->ce_uniform_count];
        strncpy(uniform->name, name, sizeof(uniform->name) - 1);
        uniform->type = ce_uniform_VEC2;
        uniform->value.vec2 = value;
        engine->ce_uniform_count++;
    }
}

void ce_uniform_set_vec3(ce_engine* engine, const char* name, ce_vec3 value) {
    for (u32 i = 0; i < engine->ce_uniform_count; i++) {
        if (strcmp(engine->uniforms[i].name, name) == 0) {
            engine->uniforms[i].type = ce_uniform_VEC3;
            engine->uniforms[i].value.vec3 = value;
            return;
        }
    }
    
    if (engine->ce_uniform_count < MAX_UNIFORMS) {
        ce_uniform* uniform = &engine->uniforms[engine->ce_uniform_count];
        strncpy(uniform->name, name, sizeof(uniform->name) - 1);
        uniform->type = ce_uniform_VEC3;
        uniform->value.vec3 = value;
        engine->ce_uniform_count++;
    }
}

void ce_uniform_set_vec4(ce_engine* engine, const char* name, ce_vec4 value) {
    for (u32 i = 0; i < engine->ce_uniform_count; i++) {
        if (strcmp(engine->uniforms[i].name, name) == 0) {
            engine->uniforms[i].type = ce_uniform_VEC4;
            engine->uniforms[i].value.vec4 = value;
            return;
        }
    }
    
    if (engine->ce_uniform_count < MAX_UNIFORMS) {
        ce_uniform* uniform = &engine->uniforms[engine->ce_uniform_count];
        strncpy(uniform->name, name, sizeof(uniform->name) - 1);
        uniform->type = ce_uniform_VEC4;
        uniform->value.vec4 = value;
        engine->ce_uniform_count++;
    }
}

void ce_uniform_set_int(ce_engine* engine, const char* name, i32 value) {
    for (u32 i = 0; i < engine->ce_uniform_count; i++) {
        if (strcmp(engine->uniforms[i].name, name) == 0) {
            engine->uniforms[i].type = ce_uniform_INT;
            engine->uniforms[i].value.i = value;
            return;
        }
    }
    
    if (engine->ce_uniform_count < MAX_UNIFORMS) {
        ce_uniform* uniform = &engine->uniforms[engine->ce_uniform_count];
        strncpy(uniform->name, name, sizeof(uniform->name) - 1);
        uniform->type = ce_uniform_INT;
        uniform->value.i = value;
        engine->ce_uniform_count++;
    }
}

void ce_uniform_set_texture(ce_engine* engine, const char* name, GLuint texture) {
    for (u32 i = 0; i < engine->ce_uniform_count; i++) {
        if (strcmp(engine->uniforms[i].name, name) == 0) {
            engine->uniforms[i].type = ce_uniform_TEXTURE;
            engine->uniforms[i].value.texture = texture;
            return;
        }
    }
    
    if (engine->ce_uniform_count < MAX_UNIFORMS) {
        ce_uniform* uniform = &engine->uniforms[engine->ce_uniform_count];
        strncpy(uniform->name, name, sizeof(uniform->name) - 1);
        uniform->type = ce_uniform_TEXTURE;
        uniform->value.texture = texture;
        engine->ce_uniform_count++;
    }
}

void ce_uniform_set_buffer_texture(ce_engine* engine, const char* name, ce_render_buffer* buffer) {
    ce_uniform_set_texture(engine, name, buffer->texture);
}

void ce_uniform_apply(ce_engine* engine, ce_shader* shader) {
    glUseProgram(shader->program);
    u32 texture_unit = 0;
    for (u32 i = 0; i < engine->ce_uniform_count; i++) {
        ce_uniform* uniform = &engine->uniforms[i];
        GLint location = glGetUniformLocation(shader->program, uniform->name); 
        if (location == -1) {
            continue;
        };
        switch (uniform->type) {
            case ce_uniform_FLOAT:
                glUniform1fv(location, 1, &uniform->value.f);
                break;
            case ce_uniform_VEC2:
                glUniform2fv(location, 1, &uniform->value.vec2.x);
                break;
            case ce_uniform_VEC3:
                glUniform3fv(location, 1, &uniform->value.vec3.x);
                break;
            case ce_uniform_VEC4:
                glUniform4fv(location, 1, &uniform->value.vec4.x);
                break;
            case ce_uniform_INT:
                glUniform1i(location, uniform->value.i);
                break;
            case ce_uniform_TEXTURE:
                glActiveTexture(GL_TEXTURE0 + texture_unit);
                glBindTexture(GL_TEXTURE_2D, uniform->value.texture);
                glUniform1i(location, texture_unit);
                texture_unit++;
                break;
        }
    }
}

// Utility functions
f64 get_time(void) {
    return glfwGetTime();
}

f64 get_delta_time(const ce_engine* engine) {
   return engine->delta_time;
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

void create_fullscreen_quad(GLuint* vao, GLuint* vbo) {
    f32 quad_vertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
    };
    
    glGenVertexArrays(1, vao);
    glGenBuffers(1, vbo);
    
    glBindVertexArray(*vao);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)(2 * sizeof(f32)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}

// Static helper functions
static void key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods) {
    ce_engine* engine = (ce_engine*)glfwGetWindowUserPointer(window);
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            engine->keys[key] = true;
        } else if (action == GLFW_RELEASE) {
            engine->keys[key] = false;
        }
    }
}

static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    ce_engine* engine = (ce_engine*)glfwGetWindowUserPointer(window);
    engine->mouse_dx = xpos - engine->mouse_x;
    engine->mouse_dy = ypos - engine->mouse_y;
    engine->mouse_x = xpos;
    engine->mouse_y = ypos;
}

static void mouse_button_callback(GLFWwindow* window, i32 button, i32 action, i32 mods) {
    ce_engine* engine = (ce_engine*)glfwGetWindowUserPointer(window);
    if (button >= 0 && button < 8) {
        if (action == GLFW_PRESS) {
            engine->mouse_buttons[button] = true;
        } else if (action == GLFW_RELEASE) {
            engine->mouse_buttons[button] = false;
        }
    }
}

static void framebuffer_size_callback(GLFWwindow* window, i32 width, i32 height) {
    ce_engine* engine = (ce_engine*)glfwGetWindowUserPointer(window);
    engine->window_width = width;
    engine->window_height = height;
    glViewport(0, 0, width, height);
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

