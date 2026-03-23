#version 450

//the values gathered from the vertex shader
layout (location = 0) in vec2 TransUVs;

//whatever the fragment shader cums out of itself
layout (location = 0) out vec4 FragColor;


layout(set = 0, binding = 0) uniform sampler2D fontAtlas;


void main() {
	vec4 color = texture(fontAtlas, TransUVs);
	float alpha = color.a;
	
	if(alpha <= 0.0) discard;
	
	FragColor = vec4(1.0, 1.0, 1.0, alpha);
}