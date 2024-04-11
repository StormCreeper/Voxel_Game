/*
    object3d.hpp
    author: Telo PHILIPPE

    A class to represent a 3D object, with a mesh, a texture and a model matrix.
*/

#ifndef OBJECT_3D_H
#define OBJECT_3D_H

#include "mesh.hpp"
#include "texture.hpp"

#include <iostream>
class Object3D {
public:
    Object3D() {
        std::cout << "Object created" << std::endl;
        mesh = std::make_shared<Mesh>();
        texture = std::make_shared<Texture>();
    }
    Object3D(std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture> texture) {
        this->mesh = mesh;
        this->texture = texture;
    }

    void render(GLuint program) const;

    void setModelMatrix(const glm::mat4 &modelMatrix) {
        this->modelMatrix = modelMatrix;
    }
    glm::mat4 getModelMatrix() const {
        return modelMatrix;
    }

protected:
    std::shared_ptr<Mesh> mesh {};
    std::shared_ptr<Texture> texture {};
    glm::mat4 modelMatrix = glm::mat4(1.0f);
};


#endif // OBJECT_3D_H