#version 450

layout (location = 0) out vec2 UV;

vec2 verticies[6] = vec2[](
	vec2(-1.0, -1.0),
	vec2(1.0, -1.0),
	vec2(1.0, 1.0),
		
	vec2(1.0, 1.0),
	vec2(-1.0,  1.0),
	vec2(-1.0, -1.0)
);
	
vec2 uvs[6] = vec2[](
	vec2(0.0, 0.0),
	vec2(1.0, 0.0),
	vec2(1.0, 1.0),
		
	vec2(1.0, 1.0),
	vec2(0.0, 1.0),
	vec2(0.0, 0.0)
);

void main() {
    UV = uvs[gl_VertexIndex];
    gl_Position = vec4(verticies[gl_VertexIndex], 0.0, 1.0);
}