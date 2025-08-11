// Syphax-Engine - Ougi Washi

#include "se_scene.h"

se_scenes_2d scenes_2d = {0};
se_scenes_3d scenes_3d = {0};

c8 vert_shader_2d[] = "#version 330 core\n"
                      "layout(location = 0) in vec2 position;\n"
                      "out vec2 texCoord;\n"
                      "void main() {\n"
                      "    gl_Position = vec4(position, 0.0, 1.0);\n"
                      "    texCoord = position * 0.5 + 0.5;\n"
                      "}\n";

c8 frag_shader_2d[] = "#version 330 core\n"
                      "in vec2 texCoord;\n"
                      "out vec4 color;\n"
                      "uniform sampler2D final;\n"
                      "void main() {\n"
                      "    color = texture(final, texCoord);\n"
                      "}\n";

se_scene_2d* se_scene_2d_create(se_render_handle* render_handle, const se_vec2* size) {
    se_scene_2d* new_scene = se_scenes_2d_increment(&scenes_2d);
    new_scene->output = se_render_buffer_create(render_handle, size->x, size->y);
    new_scene->output_shader = se_shader_load(render_handle, vert_shader_2d, frag_shader_2d);
    return new_scene;
}

void se_scene_2d_destroy(se_scene_2d* scene) {
    // TODO: Request cleanup of all render buffers and shader
    se_scenes_2d_remove(&scenes_2d, scene);
}

void se_scene_2d_render(se_scene_2d* scene, se_render_handle* render_handle, se_window* window) {
    se_foreach(se_render_buffers_ptr, scene->render_buffers, i) {
        se_render_buffer_ptr* buffer_ptr = se_render_buffers_ptr_get(&scene->render_buffers, i);
        if (buffer_ptr == NULL) {
            continue;
        }

        se_render_buffer_bind(*buffer_ptr);
        se_render_clear();
        se_window_render_quad(window);
        se_render_buffer_unbind(*buffer_ptr);
    }
}

void se_scene_2d_add_render_buffer(se_scene_2d* scene, se_render_buffer* buffer) {
    se_render_buffers_ptr_add(&scene->render_buffers, buffer);
}

void se_scene_2d_remove_render_buffer(se_scene_2d* scene, se_render_buffer* buffer) {
    se_render_buffers_ptr_remove(&scene->render_buffers, &buffer);
}

se_scene_3d* se_scene_3d_create(se_render_handle* render_handle, const se_vec2* size) {
    se_scene_3d* new_scene = se_scenes_3d_increment(&scenes_3d);
    new_scene->output = se_render_buffer_create(render_handle, size->x, size->y);
    return new_scene;
}

void se_scene_3d_destroy(se_scene_3d* scene) {
    se_scenes_3d_remove(&scenes_3d, scene);
}

void se_scene_3d_render(se_scene_3d* scene, se_render_handle* render_handle) {
    se_foreach(se_models_ptr, scene->models, i) {
        se_model_ptr* model_ptr = se_models_ptr_get(&scene->models, i);
        if (model_ptr == NULL) {
            continue;
        }
        
        se_model_render(render_handle, *model_ptr, scene->camera);
    }
    
    se_foreach(se_render_buffers_ptr, scene->post_process, i) {
        se_render_buffer_ptr* buffer_ptr = se_render_buffers_ptr_get(&scene->post_process, i);
        if (buffer_ptr == NULL) {
            continue;
        }
        
        se_render_buffer_bind(*buffer_ptr);
        se_render_clear();
        se_render_buffer_unbind(*buffer_ptr);
    }
}

void se_scene_3d_add_model(se_scene_3d* scene, se_model* model) {
    se_models_ptr_add(&scene->models, model);
}

void se_scene_3d_remove_model(se_scene_3d* scene, se_model* model) {
    se_models_ptr_remove(&scene->models, &model);
}

void se_scene_3d_set_camera(se_scene_3d* scene, se_camera* camera) {
    scene->camera = camera;
}

void se_scene_3d_add_post_process_buffer(se_scene_3d* scene, se_render_buffer* buffer) {
    se_render_buffers_ptr_add(&scene->post_process, buffer);
}

void se_scene_3d_remove_post_process_buffer(se_scene_3d* scene, se_render_buffer* buffer) {
    se_render_buffers_ptr_remove(&scene->post_process, &buffer);
}

