#version 330 core
layout(location = 0) in vec2 position;
out vec2 tex_coord;
void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    tex_coord = position * 0.5 + 0.5;
}
