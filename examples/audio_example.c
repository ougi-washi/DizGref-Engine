// Syphax-Engine - Ougi Washi

#include "se_render.h"
#include "se_audio.h"
#include <stdio.h>

#define WIDTH 1920
#define HEIGHT 1080

i32 main() {
    // audio setup  
    se_audio_input_init();
    
    // engine setup
    se_engine engine = {0};
    se_engine_init(&engine, WIDTH, HEIGHT, "Capsian Engine");
   
    se_shader* main_shader = se_shader_load(&engine, "vert.glsl", "frag_main.glsl");

    // buffer 1 setup
    se_shader* buffer1_shader = se_shader_load(&engine, "vert.glsl", "frag_buffer1.glsl");
    se_render_buffer buffer1 = {0};
    se_render_buffer_create(&buffer1, WIDTH, HEIGHT);
   
    // mesh setup
    se_shader* se_mesh_shader_0 = se_shader_load(&engine, "vert_mesh.glsl", "frag_mesh.glsl");
    se_shader** se_mesh_shaders = &se_mesh_shader_0;
    se_model model = {0};
    se_model_load_obj(&model, "cube.obj", se_mesh_shaders, 1);
    se_render_buffer model_buf = {0};
    se_render_buffer_create(&model_buf, WIDTH, HEIGHT);

    se_render_buffer post_process_buf = {0};
    se_render_buffer_create(&post_process_buf, WIDTH, HEIGHT);
    se_shader* post_process_shader = se_shader_load(&engine, "vert.glsl", "frag_post_process.glsl");

    // texture setup
    se_texture* perlin_256= se_texture_load(&engine, "perlin_256.png", SE_REPEAT);

    while (!se_engine_should_close(&engine)) {

        // input
        se_engine_poll_events(&engine);
        i32 keys[] = {GLFW_KEY_ESCAPE}; //{GLFW_KEY_LEFT_ALT, GLFW_KEY_F4};
        se_engine_check_exit_keys(&engine, keys, sizeof(keys) / sizeof(i32));

        se_engine_update(&engine);
       
        se_uniforms* global_uniforms = se_engine_get_global_uniforms(&engine);
        const se_vec3 amps = se_audio_input_get_amplitudes();
        se_uniform_set_vec3(global_uniforms, "amps", &amps);
        se_uniform_set_texture(global_uniforms, "perlin_256", perlin_256->id);
        
        // render buffer 1
        se_render_buffer_bind(&buffer1);
        se_engine_clear();
        se_shader_use(&engine, buffer1_shader, true);
        se_engine_render_quad(&engine);
        se_render_buffer_unbind(&buffer1);
        se_uniform_set_texture(global_uniforms, "buffer1", buffer1.texture);

        // render model
        se_render_buffer_bind(&model_buf);
        se_engine_clear();
        const se_vec3 rot_angle = {0.01,get_delta_time(&engine),.02};
        se_model_rotate(&model, &rot_angle);
        se_model_render(&engine, &model);
        se_render_buffer_unbind(&model_buf);
       
        // render post process
        se_render_buffer_bind(&post_process_buf);
        se_engine_clear();
        se_shader_set_texture(post_process_shader, "model", model_buf.texture);
        se_shader_set_texture(post_process_shader, "perlin_noise", perlin_256->id);
        se_shader_set_texture(post_process_shader, "previous_pp_frame", post_process_buf.prev_texture);
        se_shader_use(&engine, post_process_shader, true);
        se_engine_render_quad(&engine);
        se_render_buffer_unbind(&post_process_buf);

        // render main shader (screen)
        se_shader_use(&engine, main_shader, true);
        se_uniform_set_texture(global_uniforms, "final_frame", post_process_buf.texture);
        se_engine_render(&engine);
        se_engine_swap_buffers(&engine);
        // Print some debug info occasionally
        if (engine.frame_count % 300 == 0) {
            printf("Frame %d, Time: %.2f, FPS: %.1f\n", 
                   engine.frame_count, engine.time, 1.0 / engine.delta_time);
        }
    }
    
    se_model_cleanup(&model);
    se_audio_input_cleanup();
    se_render_buffer_cleanup(&buffer1);
    se_render_buffer_cleanup(&model_buf);
    se_engine_cleanup(&engine);
    return 0;
}
