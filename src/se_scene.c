// Syphax-Engine - Ougi Washi

#include "se_scene.h"

se_scenes_2d scenes_2d = {0};
se_scenes_3d scenes_3d = {0};

se_scene_2d* se_scene_2d_create(se_render_handle* render_handle, const se_vec2* size) {
    printf("Creating scene 2D\n");
    se_scene_2d* new_scene = se_scenes_2d_increment(&scenes_2d);
    new_scene->output = se_framebuffer_create(render_handle, size);
    return new_scene;
}

void se_scene_2d_destroy(se_scene_2d* scene) {
    // TODO: unsure if we should request cleanup of all render buffers and shaders
    se_scenes_2d_remove(&scenes_2d, scene);
}

void se_scene_2d_render(se_scene_2d* scene, se_render_handle* render_handle, se_window* window) {
    se_framebuffer_bind(scene->output);
    se_render_clear();
    se_enable_blending();
    
    se_foreach(se_objects_2d_ptr, scene->objects, i) {
        se_object_2d_ptr* object_ptr = se_objects_2d_ptr_get(&scene->objects, i);
        if (object_ptr == NULL) {
            printf("Warning: se_scene_2d_render :: object_ptr is null\n");
            continue;
        }
        se_object_2d* current_object = *object_ptr;
        se_object_2d_update_uniforms(current_object);
        se_shader_use(render_handle, current_object->shader, true, true);
        se_window_render_quad(window);
    }
    se_disable_blending();
    se_framebuffer_unbind(scene->output);

    //se_shader_set_texture(scene->output->shader, "u_texture", scene->output->texture);
    //se_shader_use(render_handle, scene->output->shader, true);
    //se_render_buffer_bind(scene->output);
    //se_render_clear();
    //se_window_render_quad(window);
    //se_render_buffer_unbind(scene->output);

    //se_foreach(se_render_buffers_ptr, scene->render_buffers, i) {
    //    se_render_buffer_ptr* buffer_ptr = se_render_buffers_ptr_get(&scene->render_buffers, i);
    //    if (buffer_ptr == NULL) {
    //        continue;
    //    }

    //    se_render_buffer* buffer = *buffer_ptr;

    //    // render buffer
    //    se_render_buffer_bind(buffer);
    //    se_shader_use(render_handle, buffer->shader, true);
    //    se_render_clear();
    //    se_window_render_quad(window);
    //    se_render_buffer_unbind(buffer);

    //    // render output
    //    se_shader_set_texture(scene->output->shader, "u_texture", buffer->texture);
    //    se_shader_use(render_handle, scene->output->shader, true);
    //    se_render_buffer_bind(scene->output);
    //    se_render_clear();
    //    se_window_render_quad(window);
    //    se_render_buffer_unbind(scene->output);
    //}
}

void se_scene_2d_render_to_screen(se_scene_2d* scene, se_render_handle* render_handle, se_window* window) {
    se_unbind_framebuffer();
    se_enable_blending();
    se_framebuffer_use_quad_shader(scene->output, render_handle);
    se_window_render_quad(window);
    se_disable_blending();
}

void se_scene_2d_add_object(se_scene_2d* scene, se_object_2d* object) {
    se_assertf(scene, "se_scene_2d_add_object :: scene is null");
    se_assertf(object, "se_scene_2d_add_object :: object is null");
    se_objects_2d_ptr_add(&scene->objects, object);
}

void se_scene_2d_remove_object(se_scene_2d* scene, se_object_2d* object) {
    se_assertf(scene, "se_scene_2d_remove_object :: scene is null");
    se_assertf(object, "se_scene_2d_remove_object :: object is null");
    se_objects_2d_ptr_remove(&scene->objects, &object);
}

se_scene_3d* se_scene_3d_create(se_render_handle* render_handle, const se_vec2* size) {
    se_scene_3d* new_scene = se_scenes_3d_increment(&scenes_3d);
    new_scene->output = se_render_buffer_create(render_handle, size->x, size->y, "shaders/scene_3d_output_frag.glsl");
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

