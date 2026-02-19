#version 450

layout (set = 0, binding = 0) uniform MatricesBuffer {
	mat4 proj;
 	mat4 view;
} MatBO;
layout ( push_constant ) uniform trans {
	mat4 model;
} meshTrans;

const vec3 boxVerts[8] = vec3[](
    vec3(-0.5, -0.5, -0.5),
    vec3( 0.5, -0.5, -0.5),
    vec3( 0.5, -0.5,  0.5),
    vec3(-0.5, -0.5,  0.5),
    vec3(-0.5,  0.5, -0.5),
    vec3( 0.5,  0.5, -0.5),
    vec3( 0.5,  0.5,  0.5),
    vec3(-0.5,  0.5,  0.5)
);

const int boxIndices[24] = int[](
    0,1, 1,2, 2,3, 3,0,  // bottom
    4,5, 5,6, 6,7, 7,4,  // top
    0,4, 1,5, 2,6, 3,7   // vertical
);

void main() {
    gl_Position = MatBO.proj * MatBO.view * meshTrans.model * vec4(boxVerts[boxIndices[gl_VertexIndex]], 1.0);
}