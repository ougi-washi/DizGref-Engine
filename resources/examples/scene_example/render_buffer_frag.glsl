#version 330 core

in vec2 tex_coord;
out vec4 frag_color;

void main() {
    frag_color = vec4(tex_coord.x, tex_coord.y, 0.0, 1.0);
}
