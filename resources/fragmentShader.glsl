/*
	fragmentShader.glsl
	author: Telo PHILIPPE
*/

#version 330 core

out vec4 outColor;

in float lighting;
in vec3 worldPos;
in vec2 textureUV;

uniform vec3 u_cameraPosition;

uniform sampler2D u_texture;

void main() {
	vec3 objColor = texture(u_texture, textureUV).xyz;

	outColor = vec4(objColor * lighting, 1.0f);
}
