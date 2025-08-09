#version 330 core
in vec2 v_uv;
in vec3 v_normal;
in vec3 v_fag_pos;

uniform vec3 amps;

out vec4 FragColor;

void main() {
    FragColor = vec4(mix(vec3(.1), v_normal, amps), 1.0);
}
