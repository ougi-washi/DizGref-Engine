// Syphax-Engine - Ougi Washi

#ifndef SE_SCENE_H
#define SE_SCENE_H

#include "se_render.h"

#define SE_MAX_SCENES 128

typedef struct {
    se_render_buffers_ptr render_buffers;
} se_scene_2d;
SE_DEFINE_ARRAY(se_scene_2d, se_scenes_2d, SE_MAX_SCENES);

typedef struct {
    se_models_ptr models;
    se_camera* camera;
    se_render_buffers_ptr post_process;
} se_scene_3d;
SE_DEFINE_ARRAY(se_scene_3d, se_scenes_3d, SE_MAX_SCENES);

extern void se_scene_2d_render(se_scene_2d* scene, se_render_handle* render_handle, se_window* window);
extern void se_scene_2d_add_render_buffer(se_scene_2d* scene, se_render_buffer* buffer);
extern void se_scene_2d_remove_render_buffer(se_scene_2d* scene, se_render_buffer* buffer);

extern void se_scene_3d_render(se_scene_3d* scene, se_render_handle* render_handle);
extern void se_scene_3d_add_model(se_scene_3d* scene, se_model* model);
extern void se_scene_3d_remove_model(se_scene_3d* scene, se_model* model);
extern void se_scene_3d_set_camera(se_scene_3d* scene, se_camera* camera);
extern void se_scene_3d_add_post_process_buffer(se_scene_3d* scene, se_render_buffer* buffer);
extern void se_scene_3d_remove_post_process_buffer(se_scene_3d* scene, se_render_buffer* buffer);

#endif // SE_SCENE_H
