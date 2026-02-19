#version 450

vec3 cubeVertices[36] = vec3[](
    // Back face
    vec3(0.0, 0.0, 0.0), vec3(32.0, 32.0, 0.0), vec3(32.0, 0.0, 0.0),
    vec3(0.0, 0.0, 0.0), vec3(0.0, 32.0, 0.0), vec3(32.0, 32.0, 0.0),

    // Front face
    vec3(0.0, 0.0, 32.0), vec3(32.0, 0.0, 32.0), vec3(32.0, 32.0, 32.0),
    vec3(0.0, 0.0, 32.0), vec3(32.0, 32.0, 32.0), vec3(0.0, 32.0, 32.0),

    // Left face
    vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 32.0), vec3(0.0, 32.0, 32.0),
    vec3(0.0, 0.0, 0.0), vec3(0.0, 32.0, 32.0), vec3(0.0, 32.0, 0.0),

    // Right face
    vec3(32.0, 0.0, 0.0), vec3(32.0, 32.0, 0.0), vec3(32.0, 32.0, 32.0),
    vec3(32.0, 0.0, 0.0), vec3(32.0, 32.0, 32.0), vec3(32.0, 0.0, 32.0),

    // Bottom face
    vec3(0.0, 0.0, 0.0), vec3(32.0, 0.0, 0.0), vec3(32.0, 0.0, 32.0),
    vec3(0.0, 0.0, 0.0), vec3(32.0, 0.0, 32.0), vec3(0.0, 0.0, 32.0),

    // Top face
    vec3(0.0, 32.0, 0.0), vec3(0.0, 32.0, 32.0), vec3(32.0, 32.0, 32.0),
    vec3(0.0, 32.0, 0.0), vec3(32.0, 32.0, 32.0), vec3(32.0, 32.0, 0.0)
);

const vec3 borderLinesVerts[24] = vec3[](
    // bottom ring
    vec3( 0.0,   0.0,   0.0 ), vec3(32.0,   0.0,   0.0 ), // 0-1
    vec3(32.0,   0.0,   0.0 ), vec3(32.0,   0.0,  32.0 ), // 1-2
    vec3(32.0,   0.0,  32.0 ), vec3( 0.0,   0.0,  32.0 ), // 2-3
    vec3( 0.0,   0.0,  32.0 ), vec3( 0.0,   0.0,   0.0 ), // 3-0

    // top ring
    vec3( 0.0, 32.0,   0.0 ), vec3(32.0, 32.0,   0.0 ), // 4-5
    vec3(32.0, 32.0,   0.0 ), vec3(32.0, 32.0,  32.0 ), // 5-6
    vec3(32.0, 32.0,  32.0 ), vec3( 0.0, 32.0,  32.0 ), // 6-7
    vec3( 0.0, 32.0,  32.0 ), vec3( 0.0, 32.0,   0.0 ), // 7-4

    // verticals
    vec3( 0.0,   0.0,   0.0 ), vec3( 0.0, 32.0,   0.0 ), // 0-4
    vec3(32.0,   0.0,   0.0 ), vec3(32.0, 32.0,   0.0 ), // 1-5
    vec3(32.0,   0.0,  32.0 ), vec3(32.0, 32.0,  32.0 ), // 2-6
    vec3( 0.0,   0.0,  32.0 ), vec3( 0.0, 32.0,  32.0 )  // 3-7
);

layout (set = 0, binding = 0)uniform MatricesBuffer {
	mat4 proj;
 	mat4 view;
} MatBO;

layout ( push_constant ) uniform trans {
	mat4 model;
} meshTrans;

void main() {
    gl_Position = MatBO.proj * MatBO.view * meshTrans.model * vec4(cubeVertices[gl_VertexIndex], 1.0);
}
