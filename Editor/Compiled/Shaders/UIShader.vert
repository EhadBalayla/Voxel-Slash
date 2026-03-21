#version 450

vec2 verticies[6] = vec2[](
	vec2(-0.5, -0.5),
	vec2(0.5, -0.5),
	vec2(0.5, 0.5),
		
	vec2(0.5, 0.5),
	vec2(-0.5,  0.5),
	vec2(-0.5, -0.5)
);
	
vec2 uvs[6] = vec2[](
	vec2(0.0, 0.0),
	vec2(1.0, 0.0),
	vec2(1.0, 1.0),
		
	vec2(1.0, 1.0),
	vec2(0.0, 1.0),
	vec2(0.0, 0.0)
);

layout ( push_constant) uniform matrices {
    mat4 MVP;
} ModelViewProj;

void main() {
    gl_Position = ModelViewProj.MVP * vec4(verticies[gl_VertexIndex], 0.0, 1.0);
}