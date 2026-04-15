#version 450

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec3 FragMRS;
layout (location = 2) out vec3 FragNormal;
layout (location = 3) out vec3 FragPos;

layout (location = 0) in vec2 uv;

layout (set = 0, binding = 0) uniform sampler2D tex;

void main() {
    FragColor = texture(tex, uv);
    FragMRS = vec3(0.0);
    FragNormal = vec3(0.0);
    FragPos = vec3(0.0);
}