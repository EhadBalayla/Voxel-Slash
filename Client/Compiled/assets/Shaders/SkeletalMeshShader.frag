#version 450

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec3 FragMRS;
layout (location = 2) out vec3 FragNormal;
layout (location = 3) out vec3 FragPos;

layout (location = 0) in vec3 MRS;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec3 Pos;


void main() {
    FragColor = vec4(1.0);
    FragMRS = MRS;
    FragNormal = normalize(Normal) * 0.5 + 0.5;
    FragPos = Pos;
}