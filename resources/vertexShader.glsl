/*
	vertexShader.glsl
	author: Telo PHILIPPE
*/

#version 460 core

layout(location=0) in uint vPosition;
layout(location=1) in float vLighting;
layout(location=2) in vec2 vUV;

uniform mat4 u_viewMat, u_projMat;

uniform ivec3 u_chunkSize;
uniform ivec3 u_chunkPos;

out vec2 textureUV;
out float lighting;

void main() {
    // ipos = pos.x + pos.y * (chunkSize.x + 1) + pos.z * (chunkSize.x + 1) + (chunkSize.y + 1);
	vec3 pos;
	pos.x = vPosition % (u_chunkSize.x+1);
	pos.z = (vPosition / (u_chunkSize.x+1)) % (u_chunkSize.z+1);
	pos.y = vPosition / ((u_chunkSize.x+1) * (u_chunkSize.z+1));

	gl_Position =  u_projMat * u_viewMat * vec4(pos + u_chunkPos * u_chunkSize, 1.0);

	lighting = vLighting;
	textureUV = vUV;
}
