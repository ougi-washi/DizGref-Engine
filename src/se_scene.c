// Syphax-Engine - Ougi Washi

#include "se_scene.h"

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

