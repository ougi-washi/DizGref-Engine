// Capsian-Engine - Ougi Washi

#include "ce_render.h"
#include "ce_audio.h"
#include <stdio.h>

#define WIDTH 1920
#define HEIGHT 1080

i32 main() {
    // audio setup  
    ce_audio_input_init();
    
    // engine setup
    ce_engine engine = {0};
    ce_engine_init(&engine, WIDTH, HEIGHT, "Abstract Shader Engine");
   
    ce_shader* main_shader = ce_shader_load(&engine, "vert.glsl", "frag_main.glsl");

    // buffer 1 setup
    ce_shader* buffer1_shader = ce_shader_load(&engine, "vert.glsl", "frag_buffer1.glsl");
    ce_render_buffer buffer1 = {0};
    ce_render_buffer_create(&buffer1, WIDTH, HEIGHT);
   
    // mesh setup
    ce_shader* ce_mesh_shader_0 = ce_shader_load(&engine, "vert_mesh.glsl", "frag_mesh.glsl");
    ce_shader** ce_mesh_shaders = &ce_mesh_shader_0;
    ce_model model = {0};
    ce_model_load_obj(&model, "cube.obj", ce_mesh_shaders, 1);
    ce_render_buffer model_buf = {0};
    ce_render_buffer_create(&model_buf, WIDTH, HEIGHT);

    ce_render_buffer post_process_buf = {0};
    ce_render_buffer_create(&post_process_buf, WIDTH, HEIGHT);
    ce_shader* post_process_shader = ce_shader_load(&engine, "vert.glsl", "frag_post_process.glsl");

    // texture setup
    ce_texture* perlin_256= ce_texture_load(&engine, "perlin_256.png", CE_REPEAT);

    while (!ce_engine_should_close(&engine)) {

        // input
        ce_engine_poll_events(&engine);
        i32 keys[] = {GLFW_KEY_ESCAPE}; //{GLFW_KEY_LEFT_ALT, GLFW_KEY_F4};
        ce_engine_check_exit_keys(&engine, keys, sizeof(keys) / sizeof(i32));

        ce_engine_update(&engine);
        
        ce_uniform_set_vec3(&engine, "amps", ce_audio_input_get_amplitudes());
        ce_uniform_set_texture(&engine, "perlin_256", perlin_256->id);
        
        // render buffer 1
        ce_render_buffer_bind(&buffer1);
        ce_engine_clear();
        ce_shader_use(&engine, buffer1_shader, true);
        ce_engine_render_quad(&engine);
        ce_render_buffer_unbind(&buffer1);
        ce_uniform_set_texture(&engine, "buffer1", buffer1.texture);

        // render model
        ce_render_buffer_bind(&model_buf);
        ce_engine_clear();
        const ce_vec3 rot_angle = {0.01,get_delta_time(&engine),.02};
        ce_model_rotate(&model, &rot_angle);
        ce_model_render(&engine, &model);
        ce_render_buffer_unbind(&model_buf);
       
        // render post process
        ce_render_buffer_bind(&post_process_buf);
        ce_engine_clear();
        ce_uniform_set_texture(&engine, "model", model_buf.texture);
        ce_uniform_set_texture(&engine, "perlin_noise", perlin_256->id);
        ce_uniform_set_texture(&engine, "previous_pp_frame", post_process_buf.prev_texture);
        ce_shader_use(&engine, post_process_shader, true);
        ce_engine_render_quad(&engine);
        ce_render_buffer_unbind(&post_process_buf);

        // render main shader (screen)
        ce_shader_use(&engine, main_shader, true);
        ce_uniform_set_texture(&engine, "final_frame", post_process_buf.texture);
        ce_engine_render(&engine);
        ce_engine_swap_buffers(&engine);
        // Print some debug info occasionally
        if (engine.frame_count % 300 == 0) {
            printf("Frame %d, Time: %.2f, FPS: %.1f\n", 
                   engine.frame_count, engine.time, 1.0 / engine.delta_time);
        }
    }
    
    ce_model_cleanup(&model);
    ce_audio_input_cleanup();
    ce_render_buffer_cleanup(&buffer1);
    ce_render_buffer_cleanup(&model_buf);
    ce_engine_cleanup(&engine);
    return 0;
}
