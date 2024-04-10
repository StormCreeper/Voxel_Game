/*
	fragmentShader.glsl
	author: Telo PHILIPPE
*/

#version 330 core

out vec4 outColor;	  // Shader output: the color response attached to this fragment

in vec3 vertexNormal;  // Input from the vertex shader
in vec3 worldPos;
in vec2 textureUV;

uniform vec3 u_cameraPosition;

uniform vec3 u_ambientLight;

uniform vec3 u_sunColor;
uniform vec3 u_sunPosition;

uniform vec3 u_objectColor;

uniform sampler2D u_texture;

void main() {
	vec3 objColor = u_objectColor;//texture(u_texture, textureUV).xyz;

	vec3 normal = normalize(vertexNormal);
	
	vec3 lightDir = normalize(u_sunPosition - worldPos);

	vec3 ambient = objColor * u_ambientLight;

	float diff = max(dot(normal, lightDir), 0.0f);

	vec3 diffuse = objColor * diff;

	outColor = vec4(ambient + diffuse, 1.0f);
}
