#version 450

//outputs to fragment shader
layout (location = 0) out vec2 TexCoords;
layout (location = 1) out float FaceAmbience;
layout (location = 2) out vec3 MRS;
layout (location = 3) out vec3 Normal;
layout (location = 4) out vec3 Pos;

//remember to remove at some point too
layout (set = 0, binding = 0) uniform MatricesBuffer {
	mat4 proj;
 	mat4 view;
} MatBO;

//the 3D matrix
layout ( push_constant ) uniform trans {
	mat4 model;
} meshTrans;

//the chunk mesh
layout (set = 1, binding = 0) readonly buffer ChunkMesh {
	uint faces[];
} Mesh;


//reconstruction constants
const uvec3 verticies[8] = uvec3[8](
    uvec3(0,0,0), uvec3(1,0,0), uvec3(1,0,1), uvec3(0,0,1),
    uvec3(0,1,0), uvec3(1,1,0), uvec3(1,1,1), uvec3(0,1,1)
);
const uint faceVertIndicies[6][6] = uint[6][6] (
	uint[6] (4, 5, 6, 6, 7, 4),
	uint[6] (0, 3, 2, 2, 1, 0),
	uint[6] (0, 4, 7, 7, 3, 0),
	uint[6] (1, 2, 6, 6, 5, 1),
	uint[6] (3, 7, 6, 6, 2, 3),
	uint[6] (0, 1, 5, 5, 4, 0)
);

const vec2 uvs[4] = vec2[4](
	vec2(0, 0), vec2(0.0625, 0), vec2(0.0625, 0.0625), vec2(0, 0.0625)
);
const uint faceUVIndicies[6][6] = uint[6][6] (
	uint[6] (0, 1, 2, 2, 3, 0),
	uint[6] (3, 2, 1, 1, 0, 3),
	uint[6] (3, 0, 1, 1, 2, 3),
	uint[6] (2, 3, 0, 0, 1, 2),
	uint[6] (3, 0, 1, 1, 2, 3),
	uint[6] (2, 3, 0, 0, 1, 2)
);

const vec3 faceNormals[6] = vec3[](
    vec3(0,1,0),
    vec3(0,-1,0),
    vec3(-1,0,0),
    vec3(1,0,0),
    vec3(0,0,-1),
    vec3(0,0,1)
);

const uint FACE_TOP    = 0u;
const uint FACE_BOTTOM = 1u;
const uint FACE_LEFT   = 2u;
const uint FACE_RIGHT  = 3u;
const uint FACE_FRONT  = 4u;
const uint FACE_BACK   = 5u;


//forward declarations
uvec3 DecodePos(uint Face);
uint DecodeAtlasIdx(uint Face);
uint DecodeFaceType(uint Face);

void main() {
	uint faceIdx = gl_VertexIndex / 6u; //which face are we on
	uint idxFace = gl_VertexIndex % 6u; //index inside the face

	//picking face
	uint Face = Mesh.faces[faceIdx];

	//decoding face
	uvec3 FacePos = DecodePos(Face);
	uint AtlasIDX = DecodeAtlasIdx(Face);
	uint faceType = DecodeFaceType(Face);

	//extracting vertex and pos for gbuffer
	uvec3 vertex = verticies[faceVertIndicies[faceType][idxFace]] + FacePos;
	vec4 worldPos = meshTrans.model * vec4(vertex, 1.0);
	gl_Position = MatBO.proj * MatBO.view * worldPos;
	Pos = worldPos.xyz;

	//extracting UV
	uint texIdxX = AtlasIDX % 16u;
	uint texIdxY = AtlasIDX / 16u;
	vec2 startUVs = vec2(float(texIdxX) / 16.0, float(texIdxY) / 16.0);
	vec2 offsetUVs = uvs[faceUVIndicies[faceType][idxFace]];
	TexCoords = startUVs + offsetUVs;

	//extracting face shading
	if(faceType == FACE_TOP) {
        FaceAmbience = 1.0;
    } else if(faceType == FACE_BOTTOM) {
        FaceAmbience = 0.2;
    } else if(faceType == FACE_LEFT || faceType == FACE_RIGHT) {
        FaceAmbience = 0.4;
    } else {
        FaceAmbience = 0.6;
    }

	//extracting MRS for gbuffer
	MRS = vec3(0.0);

	//extracting normal for gbuffer
	Normal = faceNormals[faceType];
}

uvec3 DecodePos(uint Face) {
	uvec3 ret;
	ret.x = Face & 0x1F;
	ret.z = (Face >> 5) & 0x1F;
	ret.y = (Face >> 10) & 0x1F;
	return ret;
}
uint DecodeAtlasIdx(uint Face) {
	return (Face >> 15) & 0xFF;
}
uint DecodeFaceType(uint Face) {
	return (Face >> 23) & 0x7;
}