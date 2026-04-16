#version 450

layout (location = 0) out vec4 LightColor;

layout (location = 0) in vec2 UV;

layout (set = 0, binding = 0) uniform sampler2D GColor;
layout (set = 0, binding = 1) uniform sampler2D Gmrs;
layout (set = 0, binding = 2) uniform sampler2D GNormal;
layout (set = 0, binding = 3) uniform sampler2D GPosition;

void main() {
    LightColor = texture(GNormal, UV);
}