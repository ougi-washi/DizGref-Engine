// Syphax-Engine - Ougi Washi

#include "se_scene.h"

#define WIDTH 1920
#define HEIGHT 1080

i32 main() {
    se_render_handle* render_handle = se_render_handle_create();
    
    se_window* window = se_window_create("Syphax-Engine - Scene Example", WIDTH, HEIGHT);

    se_scene_2d* scene_2d = se_scene_2d_create(render_handle, &se_vec(2, WIDTH, HEIGHT));
    
    se_object_2d* borders = se_object_2d_create(render_handle, "examples/scene_example/borders.glsl", &se_vec(2, 0, 0), &se_vec(2, 0.95, 0.95));
    se_object_2d* panel = se_object_2d_create(render_handle, "examples/scene_example/panel.glsl", &se_vec(2, 0, 0), &se_vec(2, 0.5, 0.5));
    //se_object_2d* button_yes = se_object_2d_create(render_handle, "examples/scene_example/button.glsl", &se_vec(2, 0.15, 0.), &se_vec(2, 0.1, 0.1));
    //se_object_2d* button_no = se_object_2d_create(render_handle, "examples/scene_example/button.glsl", &se_vec(2, -0.15, 0.), &se_vec(2, 0.1, 0.1));

    se_scene_2d_add_object(scene_2d, panel);
    se_scene_2d_add_object(scene_2d, borders);
    //se_scene_2d_add_object(scene_2d, button_yes);
    //se_scene_2d_add_object(scene_2d, button_no);

    //se_render_buffer* borders = se_render_buffer_create(&render_handle, WIDTH, HEIGHT, "examples/scene_example/borders.glsl");
    //se_render_buffer_set_scale(borders, &se_vec(2, 0.95, 0.95));
    //se_scene_2d_add_render_buffer(scene_2d, borders);

    //se_render_buffer* panel = se_render_buffer_create(&render_handle, WIDTH, HEIGHT, "examples/scene_example/panel.glsl");
    //se_render_buffer_set_scale(panel, &se_vec(2, 0.5, 0.5));
    //se_scene_2d_add_render_buffer(scene_2d, panel);

    //se_render_buffer* button_yes = se_render_buffer_create(&render_handle, WIDTH, HEIGHT, "examples/scene_example/button.glsl");
    //se_render_buffer_set_scale(button_yes, &se_vec(2, 0.1, 0.1));
    //se_render_buffer_set_position(button_yes, &se_vec(2, 0.15, 0.));
    //se_shader_set_vec3(button_yes->shader, "u_color", &se_vec(3, 0, 1, 0));
    //se_scene_2d_add_render_buffer(scene_2d, button_yes);

    //se_render_buffer* button_no = se_render_buffer_create(&render_handle, WIDTH, HEIGHT, "examples/scene_example/button.glsl");
    //se_render_buffer_set_scale(button_no, &se_vec(2, 0.1, 0.1));
    //se_render_buffer_set_position(button_no, &se_vec(2, -0.15, 0.));
    //se_shader_set_vec3(button_no->shader, "u_color", &se_vec(3, 1, 0, 0));
    //se_scene_2d_add_render_buffer(scene_2d, button_no);

    //se_scene_3d scene_3d = {0};

    //se_shaders_ptr model_shaders = {0};
    //se_shaders_ptr_add(&model_shaders, se_shader_load(&render_handle, "vert.glsl", "frag_mesh.glsl"));
    //se_scene_3d_add_model(&scene_3d, se_model_load_obj(&render_handle, "cube.obj", &model_shaders));
    //se_scene_3d_set_camera(&scene_3d, se_camera_create(&render_handle));
    
    key_combo exit_keys = {0};
    key_combo_add(&exit_keys, GLFW_KEY_ESCAPE);

    while (!se_window_should_close(window)) {
        se_window_poll_events();
        se_window_check_exit_keys(window, &exit_keys);
        se_window_update(window);
        se_render_handle_reload_changed_shaders(render_handle);
        //se_scene_3d_render(&scene_3d, &render_handle);
        se_scene_2d_render(scene_2d, render_handle, window);
        
        se_window_render_screen(window);
    }

    se_render_handle_cleanup(render_handle);
    se_window_destroy(window);
    return 0;
}
