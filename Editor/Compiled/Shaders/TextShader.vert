#version 450

//values to move to the fragment shader
layout (location = 0) out vec2 TransUVs;

vec2 verticies[6] = vec2[](
	vec2(0.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0),

    vec2(1.0, 1.0),
    vec2(1.0, 0.0),
    vec2(0.0, 0.0)
);

//the push constant for the transform
layout ( push_constant ) uniform trans {
	mat4 MVP;
	vec2 uvStart;
	vec2 uvOffset;
} ui_trans;

void main() {
    TransUVs = ui_trans.uvStart + ui_trans.uvOffset * verticies[gl_VertexIndex];
    gl_Position = ui_trans.MVP * vec4(verticies[gl_VertexIndex], 0.0, 1.0);
}