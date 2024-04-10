#ifndef CHUNK_HPP
#define CHUNK_HPP

#include "object3d.hpp"
#include <iostream>

class Chunk : public Object3D {
public:
    const static int CHUNK_SIZE = 16;
    Chunk() {
    }

    ~Chunk() {
        if(voxelMap)
            free(voxelMap);
    }

    void init() {
        generateVoxelMap();
        buildMesh();
    }

private:

    void generateVoxelMap() {
        
    }
    void push_vertex(std::vector<float> &vp, std::vector<float> &vn, std::vector<float> &vuv,
                    glm::vec3 pos, glm::vec3 norm, glm::vec2 uv) {
        vp.push_back(pos.x); vp.push_back(pos.y); vp.push_back(pos.z);
        vn.push_back(norm.x); vn.push_back(norm.y); vn.push_back(norm.z);
        vuv.push_back(uv.x); vuv.push_back(uv.y);
    }

    void push_triangle(std::vector<unsigned int> &ti, glm::ivec3 tri) {
        ti.push_back(tri.x); ti.push_back(tri.y); ti.push_back(tri.z);
    }

    void buildMesh() {
        std::vector<float> vp{};
        std::vector<float> vn{};
        std::vector<float> vuv{};
        std::vector<unsigned int> ti{};

        int i = 0;

        push_vertex(vp, vn, vuv, { 0,  0,  0}, {0, 0, 1}, {1, 1});
        push_vertex(vp, vn, vuv, { 1,  1,  0}, {0, 0, 1}, {1, 1});
        push_vertex(vp, vn, vuv, { 1,  0,  0}, {0, 0, 1}, {1, 1});
        push_vertex(vp, vn, vuv, { 1,  1,  0}, {0, 0, 1}, {1, 1});
        push_vertex(vp, vn, vuv, { 0,  0,  0}, {0, 0, 1}, {1, 1});
        push_vertex(vp, vn, vuv, { 0,  1,  0}, {0, 0, 1}, {1, 1});

        push_triangle(ti, {i++, i++, i++});
        push_triangle(ti, {i++, i++, i++});

        push_vertex(vp, vn, vuv, { 0,  0,  1}, {0, 0, -1}, {1, 1});
        push_vertex(vp, vn, vuv, { 1,  0,  1}, {0, 0, -1}, {1, 1});
        push_vertex(vp, vn, vuv, { 1,  1,  1}, {0, 0, -1}, {1, 1});
        push_vertex(vp, vn, vuv, { 0,  0,  1}, {0, 0, -1}, {1, 1});
        push_vertex(vp, vn, vuv, { 1,  1,  1}, {0, 0, -1}, {1, 1});
        push_vertex(vp, vn, vuv, { 0,  1,  1}, {0, 0, -1}, {1, 1});

        push_triangle(ti, {i++, i++, i++});
        push_triangle(ti, {i++, i++, i++});

        push_vertex(vp, vn, vuv, { 0,  0,  0}, {0, -1, 0}, {1, 1});
        push_vertex(vp, vn, vuv, { 1,  0,  0}, {0, -1, 0}, {1, 1});
        push_vertex(vp, vn, vuv, { 1,  0,  1}, {0, -1, 0}, {1, 1});
        push_vertex(vp, vn, vuv, { 0,  0,  0}, {0, -1, 0}, {1, 1});
        push_vertex(vp, vn, vuv, { 1,  0,  1}, {0, -1, 0}, {1, 1});
        push_vertex(vp, vn, vuv, { 0,  0,  1}, {0, -1, 0}, {1, 1});

        push_triangle(ti, {i++, i++, i++});
        push_triangle(ti, {i++, i++, i++});

        push_vertex(vp, vn, vuv, { 1,  1,  0}, {0, 1, 0}, {1, 1});
        push_vertex(vp, vn, vuv, { 0,  1,  0}, {0, 1, 0}, {1, 1});
        push_vertex(vp, vn, vuv, { 1,  1,  1}, {0, 1, 0}, {1, 1});
        push_vertex(vp, vn, vuv, { 1,  1,  1}, {0, 1, 0}, {1, 1});
        push_vertex(vp, vn, vuv, { 0,  1,  0}, {0, 1, 0}, {1, 1});
        push_vertex(vp, vn, vuv, { 0,  1,  1}, {0, 1, 0}, {1, 1});

        push_triangle(ti, {i++, i++, i++});
        push_triangle(ti, {i++, i++, i++});

        push_vertex(vp, vn, vuv, { 1,  0,  0}, {1, 0, 0}, {1, 1});
        push_vertex(vp, vn, vuv, { 1,  1,  0}, {1, 0, 0}, {1, 1});
        push_vertex(vp, vn, vuv, { 1,  1,  1}, {1, 0, 0}, {1, 1});
        push_vertex(vp, vn, vuv, { 1,  0,  0}, {1, 0, 0}, {1, 1});
        push_vertex(vp, vn, vuv, { 1,  1,  1}, {1, 0, 0}, {1, 1});
        push_vertex(vp, vn, vuv, { 1,  0,  1}, {1, 0, 0}, {1, 1});

        push_triangle(ti, {i++, i++, i++});
        push_triangle(ti, {i++, i++, i++});

        push_vertex(vp, vn, vuv, { 0,  1,  0}, {-1, 0, 0}, {1, 1});
        push_vertex(vp, vn, vuv, { 0,  0,  0}, {-1, 0, 0}, {1, 1});
        push_vertex(vp, vn, vuv, { 0,  1,  1}, {-1, 0, 0}, {1, 1});
        push_vertex(vp, vn, vuv, { 0,  1,  1}, {-1, 0, 0}, {1, 1});
        push_vertex(vp, vn, vuv, { 0,  0,  0}, {-1, 0, 0}, {1, 1});
        push_vertex(vp, vn, vuv, { 0,  0,  1}, {-1, 0, 0}, {1, 1});

        push_triangle(ti, {i++, i++, i++});
        push_triangle(ti, {i++, i++, i++});

        std::cout << vp.size() << ", " << vn.size() << ", " << vuv.size() << ", " << ti.size() << "\n";

        mesh->initGPUGeometry(vp, vn, vuv, ti);
    }
private:
    int *voxelMap {};
};

#endif // CHUNK_HPP