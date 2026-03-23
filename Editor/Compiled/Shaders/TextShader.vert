#version 450

//values to move to the fragment shader
layout (location = 0) out vec2 TransUVs;

//the push constant for the transform
layout ( push_constant ) uniform trans {
	mat4 MVP;
	uint idx;
} ui_trans;

vec2 verticies[6] = vec2[](
	vec2(0.0, 0.0),
	vec2(1.0, 0.0),
	vec2(1.0, 1.0),
		
	vec2(1.0, 1.0),
	vec2(0.0, 1.0),
	vec2(0.0, 0.0)
);

void main() {
    uint xStart = ui_trans.idx % 16;
    uint yStart = ui_trans.idx / 16;

    vec2 StartUV = vec2(float(xStart), float(yStart));
    float OffsetUV = 1.0 / 16.0; //literally for both atlas directions

	if(gl_VertexIndex == 0 || gl_VertexIndex == 5) TransUVs = StartUV;
	else if(gl_VertexIndex == 1) TransUVs = vec2(StartUV.x + OffsetUV, StartUV.y);
	else if(gl_VertexIndex == 2 || gl_VertexIndex == 3) TransUVs = vec2(StartUV.x + OffsetUV, StartUV.y + OffsetUV);
	else TransUVs = vec2(StartUV.x, StartUV.y + OffsetUV);
	
	gl_Position = ui_trans.MVP * vec4(verticies[gl_VertexIndex], 0.0, 1.0);
}