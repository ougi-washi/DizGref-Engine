#version 330 core
in vec2 v_uv;
in vec3 v_normal;
in vec3 v_fag_pos;

uniform sampler2D buffer1;

out vec4 FragColor;

void main() {
    FragColor = vec4(v_normal, 1.0);
}
