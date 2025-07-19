#version 330 core

uniform vec2 resolution;
uniform vec2 mouse;
uniform int frame;
uniform float time;
uniform sampler2D model;
uniform sampler2D previous_pp_frame;
uniform sampler2D perlin_256;

in vec2 TexCoord;
out vec4 FragColor;

float noise(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    FragColor = texture(model, TexCoord);
    float perlin = texture(perlin_256, TexCoord).r;
    FragColor = mix(texture(previous_pp_frame, TexCoord + vec2(sin(time), cos(time)) * 0.01), FragColor, 0.15 * perlin);
}
