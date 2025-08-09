#version 330 core

uniform sampler2D model_buffer;

in vec2 TexCoord;
out vec4 FragColor;

void main() {
    FragColor = texture(model_buffer, TexCoord);
}
