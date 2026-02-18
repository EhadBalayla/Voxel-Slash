#version 450
layout (location = 0) in uint vert;

layout (set = 0, binding = 0) uniform MatricesBuffer {
	mat4 proj;
 	mat4 view;
} MatBO;

layout (std140, set = 0, binding = 2) readonly buffer TransBuffer {
	mat4 trans[];
} TransBO;

layout ( push_constant ) uniform trans {
	mat4 model;
} meshTrans;

layout (location = 0) out vec2 TexCoords;
layout (location = 1) out float Lighting;

uvec3 decodePos(uint pos);
uvec3 decodeCorner(uint corner);
vec2 decodeUVoffset(uint texCorner);

void main() {
    uint pos = vert & 0xFFFFu;
    uint corner = (vert >> 15) & 7u;
	uint texCorner = (vert >> 18) & 3u;
	uint texOffset = (vert >> 20) & 0xFFu;
	uint faceID = (vert >> 28) & 7u;

    uvec3 aPos = decodePos(pos);
    uvec3 aCorner = decodeCorner(corner);

    gl_Position = MatBO.proj * MatBO.view * meshTrans.model * vec4(aPos + aCorner, 1.0);

    // Lighting based on face direction
    // Top face: brightest, Bottom: darkest, Sides: medium
    float ambientLight = 0.4;
    float directionalLight = 0.6;
    
    if(faceID == 0u) { // Top
        Lighting = ambientLight + directionalLight * 1.0;
    } else if(faceID == 1u) { // Bottom
        Lighting = ambientLight + directionalLight * 0.2;
    } else if(faceID == 2u || faceID == 3u) { // Left/Right
        Lighting = ambientLight + directionalLight * 0.4;
    } else { // Front/Back
        Lighting = ambientLight + directionalLight * 0.6;
    }

	uint indexX = texOffset % uint(8);
	uint indexY = texOffset / uint(8);
	float uvX =  float(indexX) / 8.0;
	float uvY = float(indexY) / 8.0;
	vec2 uvOffset = decodeUVoffset(texCorner);

	TexCoords = vec2(uvX, uvY) + uvOffset;
}

uvec3 decodePos(uint pos) {
	uvec3 ret;
	ret.x = pos & 31u;
	pos >>= 5u;
	ret.z = pos & 31u;
	pos >>= 5u;
	ret.y = pos & 31u;
	
	return ret;
}
uvec3 decodeCorner(uint corner) {
	const uvec3 offsets[8] = uvec3[8](
        uvec3(0,0,0), uvec3(1,0,0), uvec3(1,0,1), uvec3(0,0,1),
        uvec3(0,1,0), uvec3(1,1,0), uvec3(1,1,1), uvec3(0,1,1)
    );
	return offsets[corner];
}
vec2 decodeUVoffset(uint texCorner) {
	const vec2 offsets[4] = vec2[4](
		vec2(0, 0), vec2(0.125, 0), vec2(0.125, 0.125), vec2(0, 0.125)
	);
	return offsets[texCorner];
}