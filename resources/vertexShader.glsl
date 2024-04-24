/*
	vertexShader.glsl
	author: Telo PHILIPPE
*/

#version 330 core            // Minimal GL version support expected from the GPU

layout(location=0) in vec3 vPosition;
layout(location=1) in float vLighting;
layout(location=2) in vec2 vUV;

uniform mat4 u_viewMat, u_projMat, u_modelMat;

out vec3 worldPos;
out vec2 textureUV;
out float lighting;

void main() {
	gl_Position =  u_projMat * u_viewMat * u_modelMat * vec4(vPosition, 1.0);
	
	vec4 worldPos_Homo = u_modelMat * vec4(vPosition, 1.0);
	worldPos = worldPos_Homo.xyz / worldPos_Homo.w;
	

	lighting = vLighting;
	textureUV = vUV;
}
