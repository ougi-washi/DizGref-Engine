#version 330 core

uniform vec2 resolution;
uniform vec2 mouse;
uniform int frame;
uniform float time;
uniform sampler2D final_frame;

in vec2 TexCoord;
out vec4 FragColor;

void main() {
    FragColor = texture(final_frame, TexCoord);
}
