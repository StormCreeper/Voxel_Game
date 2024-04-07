#ifndef CHUNK_HPP
#define CHUNK_HPP

#include "object3d.hpp"

class Chunk : public Object3D {
public:
    const static int CHUNK_SIZE = 16;
    Chunk() {
        buildMesh();
    }

private:
    void buildMesh() {
        std::vector<float> vertexPositions{};
        std::vector<float> vertexNormals{};
        std::vector<float> vertexUVs{};
        std::vector<unsigned int> triangleIndices{};


        mesh->initGPUGeometry(vertexPositions, vertexNormals, vertexUVs, triangleIndices);
    }
};

#endif // CHUNK_HPP