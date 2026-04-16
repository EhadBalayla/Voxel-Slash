#version 450
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 aTexCoords;
layout (location = 4) in ivec4 aBoneIDs;
layout (location = 5) in vec4 aBoneWeights;

layout (location = 0) out vec3 MRS;
layout (location = 1) out vec3 Normal;
layout (location = 2) out vec3 Pos;

layout (set = 0, binding = 0) uniform MatricesBuffer {
	mat4 proj;
 	mat4 view;
} MatBO;

layout ( push_constant ) uniform trans {
	mat4 model;
} meshTrans;

void main() {
	vec4 worldPos = meshTrans.model * vec4(aPos, 1.0);
    gl_Position = MatBO.proj * MatBO.view * worldPos;
	Pos = worldPos.xyz;

	MRS = vec3(1.0, 1.0, 0.0);

	mat3 normalMatrix = mat3(transpose(inverse(meshTrans.model)));
	Normal = normalize(normalMatrix * aNormal);
}