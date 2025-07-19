// DizGref-Engine - Ougi Washi

#include "dg_render.h"
#include "dg_audio.h"
#include <stdio.h>

#define WIDTH 1920
#define HEIGHT 1080

i32 main() {
    // audio setup  
    dg_audio_init();
    
    // engine setup
    dg_engine engine = {0};
    bool dg_engine_success = dg_engine_init(&engine, WIDTH, HEIGHT, "Abstract Shader Engine");
    assert(dg_engine_success);
   
    dg_shader* main_shader = dg_shader_load(&engine, "vert.glsl", "frag_main.glsl");
    assert(main_shader);

    // buffer 1 setup
    dg_shader* buffer1_shader = dg_shader_load(&engine, "vert.glsl", "frag_buffer1.glsl");
    assert(buffer1_shader);
    dg_render_buffer buffer1 = {0};
    bool dg_render_buffer_res = dg_render_buffer_create(&buffer1, WIDTH, HEIGHT);
    assert(dg_render_buffer_res);
   
    // mesh setup
    dg_shader* dg_mesh_shader_0 = dg_shader_load(&engine, "vert_mesh.glsl", "frag_mesh.glsl");
    dg_shader** dg_mesh_shaders = &dg_mesh_shader_0;
    assert(dg_mesh_shaders);
    dg_model model = {0};
    dg_model_load_obj(&model, "cube.obj", dg_mesh_shaders, 1);
    dg_render_buffer model_buf = {0};
    dg_render_buffer_create(&model_buf, WIDTH, HEIGHT);

    // texture setup
    dg_texture* texture = dg_texture_load(&engine, "perlin_256.png");
    assert(texture);
  
    while (!dg_engine_should_close(&engine)) {

        // input
        dg_engine_poll_events(&engine);
        i32 keys[] = {GLFW_KEY_ESCAPE}; //{GLFW_KEY_LEFT_ALT, GLFW_KEY_F4};
        dg_engine_check_exit_keys(&engine, keys, sizeof(keys) / sizeof(i32));

        dg_engine_update(&engine);
        
        dg_uniform_set_vec3(&engine, "amps", dg_audio_get_amplitudes());
        dg_uniform_set_texture(&engine, "perlin_noise", texture->id);
        
        dg_shader_use(&engine, buffer1_shader, true);
        
        // render model
        dg_render_buffer_bind(&model_buf);
        dg_engine_clear();
        const dg_vec3 rot_angle = {0,0,get_delta_time(&engine)};
        dg_model_rotate(&model, &rot_angle);
        dg_model_render(&engine, &model);
        dg_render_buffer_unbind();

        // render buffer 1
        dg_render_buffer_bind(&buffer1);
        dg_engine_clear();
        dg_shader_use(&engine, buffer1_shader, true);
        dg_engine_render_quad(&engine);
        dg_render_buffer_unbind();
       
        // render main shader (screen)
        dg_shader_use(&engine, main_shader, true);
        dg_uniform_set_texture(&engine, "buffer1", buffer1.texture);
        dg_uniform_set_texture(&engine, "model", model_buf.texture);
        dg_engine_render(&engine);
        dg_engine_swap_buffers(&engine);
        // Print some debug info occasionally
        if (engine.frame_count % 300 == 0) {
            printf("Frame %d, Time: %.2f, FPS: %.1f\n", 
                   engine.frame_count, engine.time, 1.0 / engine.delta_time);
        }
    }
    
    dg_model_cleanup(&model);
    dg_audio_cleanup();
    dg_render_buffer_cleanup(&buffer1);
    dg_render_buffer_cleanup(&model_buf);
    dg_engine_cleanup(&engine);
    return 0;
}
