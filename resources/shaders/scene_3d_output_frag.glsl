#version 330 core
in vec2 tex_coord;
out vec4 color;
uniform sampler2D final;
void main() {
    color = texture(final, tex_coord);
}
