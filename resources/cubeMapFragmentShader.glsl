/*
    cubeMapFragmentShader.cpp
    author: Telo PHILIPPE
*/

#version 330 core
out vec4 FragColor;

in vec3 fPosition;

uniform sampler2D u_texture;

void main() {
    // Sample texture with sphere mapping
    vec3 direction = normalize(fPosition);
    vec2 sphereUV = vec2(atan(direction.z, direction.x), asin(direction.y));
    sphereUV *= vec2(0.1591, 0.3183);
    sphereUV += 0.5;
    FragColor = texture(u_texture, sphereUV);
}