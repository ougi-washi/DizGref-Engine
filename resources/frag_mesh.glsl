#version 330 core
in vec2 v_uv;
in vec3 v_normal;
in vec3 v_fag_pos;

uniform sampler2D buffer1;

out vec4 FragColor;

void main() {
    FragColor = vec4(texture(buffer1, 2.5 * v_uv - .5).rgb + .5, 1.0);
}
