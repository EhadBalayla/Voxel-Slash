#version 450
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 aTexCoords;
layout (location = 4) in ivec4 aBoneIDs;
layout (location = 5) in vec4 aBoneWeights;

layout (set = 0, binding = 0) uniform MatricesBuffer {
	mat4 proj;
 	mat4 view;
} MatBO;

layout ( push_constant ) uniform trans {
	mat4 model;
} meshTrans;

void main() {
    gl_Position = MatBO.proj * MatBO.view * meshTrans.model * vec4(aPos, 1.0);
}