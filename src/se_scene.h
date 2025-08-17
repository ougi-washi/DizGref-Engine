// Syphax-Engine - Ougi Washi

// Scenes are used as a collection of pointers to rendering instances
// They are not responsible for handling memory, and are only used for referencing the rendering instances
// Render handles are the ones responsible for handling memory, so no allocation and deallocation is needed here

#ifndef SE_SCENE_H
#define SE_SCENE_H

#include "se_render.h"
#include "se_window.h"

#define SE_MAX_SCENES 128

typedef struct {
    se_objects_2d_ptr objects;
    se_framebuffer_ptr output;
} se_scene_2d;
SE_DEFINE_ARRAY(se_scene_2d, se_scenes_2d, SE_MAX_SCENES);
typedef se_scene_2d* se_scene_2d_ptr;
SE_DEFINE_ARRAY(se_scene_2d_ptr, se_scenes_2d_ptr, SE_MAX_SCENES);

typedef struct {
    se_models_ptr models;
    se_camera_ptr camera;
    se_render_buffers_ptr post_process;
    
    se_shader_ptr output_shader;
    se_render_buffer_ptr output;
} se_scene_3d;
SE_DEFINE_ARRAY(se_scene_3d, se_scenes_3d, SE_MAX_SCENES);
typedef se_scene_3d* se_scene_3d_ptr;
SE_DEFINE_ARRAY(se_scene_3d_ptr, se_scenes_3d_ptr, SE_MAX_SCENES);

extern se_scene_2d* se_scene_2d_create(se_render_handle* render_handle, const se_vec2* size);
extern void se_scene_2d_destroy(se_scene_2d* scene);
extern void se_scene_2d_render(se_scene_2d* scene, se_render_handle* render_handle, se_window* window);
extern void se_scene_2d_render_to_screen(se_scene_2d* scene, se_render_handle* render_handle, se_window* window);
extern void se_scene_2d_add_object(se_scene_2d* scene, se_object_2d* object);
extern void se_scene_2d_remove_object(se_scene_2d* scene, se_object_2d* object);

extern se_scene_3d* se_scene_3d_create(se_render_handle* render_handle, const se_vec2* size);
extern void se_scene_3d_destroy(se_scene_3d* scene);
extern void se_scene_3d_render(se_scene_3d* scene, se_render_handle* render_handle);
extern void se_scene_3d_add_model(se_scene_3d* scene, se_model* model);
extern void se_scene_3d_remove_model(se_scene_3d* scene, se_model* model);
extern void se_scene_3d_set_camera(se_scene_3d* scene, se_camera* camera);
extern void se_scene_3d_add_post_process_buffer(se_scene_3d* scene, se_render_buffer* buffer);
extern void se_scene_3d_remove_post_process_buffer(se_scene_3d* scene, se_render_buffer* buffer);

#endif // SE_SCENE_H
