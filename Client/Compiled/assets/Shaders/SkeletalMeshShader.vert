#version 450
layout (set = 0, binding = 0) uniform MatricesBuffer {
	mat4 proj;
 	mat4 view;
} MatBO;

layout ( push_constant ) uniform trans {
	mat4 model;
} meshTrans;

vec3 vertices[8] = vec3[](
    vec3(-0.5, -0.5, -0.5),
    vec3( 0.5, -0.5, -0.5),
    vec3( 0.5,  0.5, -0.5),
    vec3(-0.5,  0.5, -0.5),
    vec3(-0.5, -0.5,  0.5),
    vec3( 0.5, -0.5,  0.5),
    vec3( 0.5,  0.5,  0.5),
    vec3(-0.5,  0.5,  0.5) 
);
uint indices[36] = uint[](
    4, 5, 6, 4, 6, 7,
    0, 1, 2, 0, 2, 3,
    0, 3, 7, 0, 7, 4,
    1, 5, 6, 1, 6, 2,
    0, 1, 5, 0, 5, 4,
    3, 2, 6, 3, 6, 7
);

void main() {
    gl_Position = MatBO.proj * MatBO.view * meshTrans.model * vec4(vertices[indices[gl_VertexIndex]], 1.0);
}