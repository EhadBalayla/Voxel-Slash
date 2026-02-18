#version 450

layout (location = 0) out vec4 FragColor;

layout (set = 0, binding = 1) uniform sampler2D textureAtlas;

void main() {
    FragColor = vec4(0.0, 0.6, 1.0, 1.0);
}
