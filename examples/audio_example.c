// Syphax-render_handle - Ougi Washi

#include "se_render.h"
#include "se_window.h"
#include "se_audio.h"

#define WIDTH 1920
#define HEIGHT 1080

i32 main() {
    // audio setup  
    se_audio_input_init();
   
    se_window* window = se_window_create("Syphax-Engine - Audio Example", WIDTH, HEIGHT);
    se_render_handle render_handle = {0};
    se_camera* camera = se_camera_create(&render_handle);

    se_shader* main_shader = se_shader_load(&render_handle, "vert.glsl", "frag_main.glsl");

    //// buffer 1 setup
    //se_shader* buffer1_shader = se_shader_load(&render_handle, "vert.glsl", "frag_buffer1.glsl");
    //se_render_buffer buffer1 = {0};
    //se_render_buffer_create(&buffer1, WIDTH, HEIGHT);
   
    // mesh setup
    se_shader* se_mesh_shader_0 = se_shader_load(&render_handle, "vert_mesh.glsl", "frag_mesh.glsl");
    se_shader** se_mesh_shaders = &se_mesh_shader_0;
    se_model model = {0};
    se_model_load_obj(&model, "cube.obj", se_mesh_shaders, 1);
    se_render_buffer model_buf = {0};
    se_render_buffer_create(&model_buf, WIDTH, HEIGHT);

    //se_render_buffer post_process_buf = {0};
    //se_render_buffer_create(&post_process_buf, WIDTH, HEIGHT);
    //se_shader* post_process_shader = se_shader_load(&render_handle, "vert.glsl", "frag_post_process.glsl");

    // texture setup
    //se_texture* perlin_256= se_texture_load(&render_handle, "perlin_256.png", SE_REPEAT);

    while (!se_window_should_close(window)) {

        // input
        key_combo exit_keys = {0};
        key_combo_add(&exit_keys, GLFW_KEY_ESCAPE);
        se_window_poll_events();
        se_window_check_exit_keys(window, &exit_keys);

        se_window_update(window);
      
        se_render_handle_reload_changed_shaders(&render_handle);
       
        se_uniforms* global_uniforms = se_render_handle_get_global_uniforms(&render_handle);
        const se_vec3 amps = se_audio_input_get_amplitudes();
        //se_uniform_set_vec3(global_uniforms, "amps", &amps);
        //se_uniform_set_texture(global_uniforms, "perlin_256", perlin_256->id);
        //
        //// render buffer 1
        //se_render_buffer_bind(&buffer1);
        //se_render_clear();
        //se_shader_use(&render_handle, buffer1_shader, true);
        //se_window_render_quad(window);
        //se_render_buffer_unbind(&buffer1);
        //se_uniform_set_texture(global_uniforms, "buffer1", buffer1.texture);

        // render model
        se_render_buffer_bind(&model_buf);
        se_render_clear();
        const se_vec3 rot_angle = {0.006, se_window_get_delta_time(window) * 0.1, .004};
        se_model_rotate(&model, &rot_angle);
        se_model_render(&render_handle, &model, camera);
        se_render_buffer_unbind(&model_buf);
       
        // render post process
        //se_render_buffer_bind(&post_process_buf);
        //se_render_clear();
        //se_shader_set_texture(post_process_shader, "model", model_buf.texture);
        //se_shader_set_texture(post_process_shader, "perlin_noise", perlin_256->id);
        //se_shader_set_texture(post_process_shader, "previous_pp_frame", post_process_buf.prev_texture);
        //se_shader_use(&render_handle, post_process_shader, true);
        //se_window_render_quad(window);
        //se_render_buffer_unbind(&post_process_buf);

        // render main shader (screen)
        se_shader_use(&render_handle, main_shader, true);
        se_uniform_set_texture(global_uniforms, "final_frame", model_buf.texture);
        
        se_window_render_screen(window);
    }
   
    se_model_cleanup(&model);
    se_audio_input_cleanup();
    //se_render_buffer_cleanup(&buffer1);
    se_render_buffer_cleanup(&model_buf);
    se_render_handle_cleanup(&render_handle);
    se_camera_destroy(&render_handle, camera);
    se_window_destroy(window);
    return 0;
}
