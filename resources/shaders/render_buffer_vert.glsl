#version 330 core
layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_tex_coord;

uniform vec2 u_texture_size;
uniform vec2 u_size;
uniform vec2 u_position;

out vec2 tex_coord;

void main() {
    float ratio = u_texture_size.x / u_texture_size.y;
    
    vec2 new_position = in_position;
    
    vec2 corrected_size = u_size;
    vec2 corrected_position = u_position;
    if (ratio > 1.0) {
        corrected_size.y *= ratio;
        corrected_position.y *= ratio;
    } else {
        corrected_size.x *= ratio;
        corrected_position.x *= ratio;
    }
    
    new_position *= corrected_size;
    new_position += corrected_position;
    
    gl_Position = vec4(new_position, 0, 1);
    tex_coord = in_tex_coord;
}
