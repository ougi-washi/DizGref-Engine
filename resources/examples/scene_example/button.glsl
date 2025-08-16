#version 330 core

in vec2 tex_coord;
out vec4 frag_color;

uniform vec3 u_color;

void main() {
    float container_mask = smoothstep(0., .1, tex_coord.x);
    container_mask = min(container_mask, smoothstep(0., .1, tex_coord.y));
    container_mask = min(container_mask, smoothstep(1., .9, tex_coord.x));
    container_mask = min(container_mask, smoothstep(1., .9, tex_coord.y));
    frag_color = vec4(u_color, container_mask);
}
