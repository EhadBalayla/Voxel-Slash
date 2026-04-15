#version 450

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec3 FragMRS;
layout (location = 2) out vec3 FragNormal;
layout (location = 3) out vec3 FragPos;

layout (location = 0) in vec2 TexCoords;
layout (location = 1) in float FaceAmbience;
layout (location = 2) in vec3 MRS;
layout (location = 3) in vec3 Normal;
layout (location = 4) in vec3 Pos;

layout (set = 0, binding = 1) uniform sampler2D textureAtlas;

void main() {
    vec4 texColor = texture(textureAtlas, TexCoords);
    if(texColor.a < 0.1) discard;

    FragColor = vec4(texColor.rgb * FaceAmbience, texColor.a);
    FragMRS = MRS;
    FragNormal = Normal;
    FragPos = Pos;
}