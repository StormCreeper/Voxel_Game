/*
    object3d.cpp
    author: Telo PHILIPPE

    Implementation of the Object3D class.
*/

#include "object3d.hpp"
#include "shader.hpp"


void Object3D::render(GLuint program) const {
    // Set uniforms and bind texture
    setUniform(program, "u_modelMat", modelMatrix);
    setUniform(program, "u_shaded", shaded);

    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    setUniform(program, "u_texture", 0);

    if(nightTexture) {
        glActiveTexture(GL_TEXTURE1);
        nightTexture->bind();
        setUniform(program, "u_nightTexture", 1);
    }
    
    setUniform(program, "u_useNightTexture", (bool)nightTexture);

    if(specularTexture) {
        glActiveTexture(GL_TEXTURE2);
        specularTexture->bind();
        setUniform(program, "u_specularTexture", 2);
    }

    setUniform(program, "u_useSpecularTexture", (bool)specularTexture);

    mesh->render();

    if(nightTexture) {
        glActiveTexture(GL_TEXTURE1);
        nightTexture->unbind();
    }

    if(specularTexture) {
        glActiveTexture(GL_TEXTURE2);
        specularTexture->unbind();
    }

    glActiveTexture(GL_TEXTURE0);
    texture->unbind();
    setUniform(program, "u_modelMat", glm::mat4(1.0f));
}