/*
	cubeMapVertexShader.cpp
	author: Telo PHILIPPE
*/

#version 460 core

layout(location=0) in vec3 vPosition;

uniform mat4 u_projMat;
uniform mat4 u_viewMat;

out vec3 fPosition;

void main() {
	gl_Position =  u_projMat * u_viewMat * vec4(vPosition, 1.0);
	fPosition = vPosition;
}