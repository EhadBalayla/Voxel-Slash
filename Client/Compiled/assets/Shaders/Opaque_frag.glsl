#version 450

layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec2 TexCoords;
layout (location = 1) in float Lighting;

layout (set = 0, binding = 1) uniform sampler2D textureAtlas;

void main() {
    vec4 texColor = texture(textureAtlas, TexCoords);
    if(texColor.a < 0.1) discard;
    FragColor = vec4(texColor.rgb * Lighting, texColor.a);
}