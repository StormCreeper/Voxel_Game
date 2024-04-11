#ifndef CHUNK_HPP
#define CHUNK_HPP

#include "object3d.hpp"
#include <iostream>

const int chunk_size = 10;
const float tex_num_x = 4;
const float tex_num_y = 1;

enum DIR {
    UP, DOWN, LEFT, RIGHT, FRONT, BACK
};

class Chunk : public Object3D {
public:
    const static int CHUNK_SIZE = 16;
    Chunk() {
        std::cout << "Chunk creates" <<  std::endl;
    }

    ~Chunk() {
        if(voxelMap)
            free(voxelMap);
    }

    void init() {
        texture = std::make_shared<Texture>("../resources/media/atlas.jpg");
        generateVoxelMap();
        buildMesh();
    }

private:

    int index(int x, int y, int z) {
        return x + y * chunk_size + z * chunk_size * chunk_size;
    }

    void generateVoxelMap() {
        voxelMap = (int *) malloc(chunk_size*chunk_size*chunk_size*sizeof(int));

        for(int x = 0; x<chunk_size; x++) {
            for(int y = 0; y<chunk_size; y++) {
                for(int z = 0; z<chunk_size; z++) {
                    voxelMap[index(x, y, z)] = sin((x+z) * 0.5) * 5 + 5 < y;
                }
            }
        }

    }
    int push_vertex(glm::vec3 pos, glm::vec3 norm, glm::vec2 uv) {
        pos += world_offset;
        uv = tex_offset + uv * tex_size;
        vp.push_back(pos.x); vp.push_back(pos.y); vp.push_back(pos.z);
        vn.push_back(norm.x); vn.push_back(norm.y); vn.push_back(norm.z);
        vuv.push_back(uv.x); vuv.push_back(uv.y);

        return vert_index++;
    }

    void push_triangle(glm::ivec3 tri) {
        ti.push_back(tri.x); ti.push_back(tri.y); ti.push_back(tri.z);
    }

    void push_face(DIR dir, int texIndex) {

        tex_offset.x = (float)(texIndex) / tex_num_x;
        tex_offset.y = 0;
        tex_size.x = 1.0 / tex_num_x;
        tex_size.y = 1.0;

        switch (dir) {
        case DIR::UP:
            push_triangle({
                push_vertex({ 1,  1,  0}, {0, 1, 0}, {1, 0}),
                push_vertex({ 0,  1,  0}, {0, 1, 0}, {0, 0}),
                push_vertex({ 1,  1,  1}, {0, 1, 0}, {1, 1})
            });
            push_triangle({
                push_vertex({ 1,  1,  1}, {0, 1, 0}, {1, 1}),
                push_vertex({ 0,  1,  0}, {0, 1, 0}, {0, 0}),
                push_vertex({ 0,  1,  1}, {0, 1, 0}, {0, 1}),
            });
            break;
        case DIR::DOWN:
            push_triangle({
                push_vertex({ 0,  0,  0}, {0, -1, 0}, {0, 0}),
                push_vertex({ 1,  0,  0}, {0, -1, 0}, {1, 0}),
                push_vertex({ 1,  0,  1}, {0, -1, 0}, {1, 1})
            });
            push_triangle({
                push_vertex({ 0,  0,  0}, {0, -1, 0}, {0, 0}),
                push_vertex({ 1,  0,  1}, {0, -1, 0}, {1, 1}),
                push_vertex({ 0,  0,  1}, {0, -1, 0}, {0, 1})
            });
            break;
        case DIR::FRONT:
            push_triangle({
                push_vertex({ 1,  0,  0}, {0, 0, 1}, {1, 1}),
                push_vertex({ 0,  0,  0}, {0, 0, 1}, {0, 1}),
                push_vertex({ 1,  1,  0}, {0, 0, 1}, {1, 0})
            });
            push_triangle({
                push_vertex({ 1,  1,  0}, {0, 0, 1}, {1, 0}),
                push_vertex({ 0,  0,  0}, {0, 0, 1}, {0, 1}),
                push_vertex({ 0,  1,  0}, {0, 0, 1}, {0, 0})
            });
            break;
        case DIR::BACK:
            push_triangle({
                push_vertex({ 0,  0,  1}, {0, 0, -1}, {0, 1}),
                push_vertex({ 1,  0,  1}, {0, 0, -1}, {1, 1}),
                push_vertex({ 1,  1,  1}, {0, 0, -1}, {1, 0})
            });
            push_triangle({
                push_vertex({ 0,  0,  1}, {0, 0, -1}, {0, 1}),
                push_vertex({ 1,  1,  1}, {0, 0, -1}, {1, 0}),
                push_vertex({ 0,  1,  1}, {0, 0, -1}, {0, 0})
            });
            break;
        case DIR::RIGHT:
            push_triangle({
                push_vertex({ 0,  1,  0}, {-1, 0, 0}, {0, 0}),
                push_vertex({ 0,  0,  0}, {-1, 0, 0}, {0, 1}),
                push_vertex({ 0,  1,  1}, {-1, 0, 0}, {1, 0})
            });
            push_triangle({
                push_vertex({ 0,  1,  1}, {-1, 0, 0}, {1, 0}),
                push_vertex({ 0,  0,  0}, {-1, 0, 0}, {0, 1}),
                push_vertex({ 0,  0,  1}, {-1, 0, 0}, {1, 1})
            });
            break;
        case DIR::LEFT:
            push_triangle({
                push_vertex({ 1,  0,  0}, {1, 0, 0}, {0, 1}),
                push_vertex({ 1,  1,  0}, {1, 0, 0}, {0, 0}),
                push_vertex({ 1,  1,  1}, {1, 0, 0}, {1, 0})
            });
            push_triangle({
                push_vertex({ 1,  0,  0}, {1, 0, 0}, {0, 1}),
                push_vertex({ 1,  1,  1}, {1, 0, 0}, {1, 0}),
                push_vertex({ 1,  0,  1}, {1, 0, 0}, {1, 1})
            });
            break;
        }
    }

    int getBlock(int x, int y, int z) {
        if(x < 0 || y < 0 || z < 0) return 0;
        if(x >= chunk_size || y >= chunk_size || z >= chunk_size) return 0;

        return voxelMap[index(x, y, z)];
    }

    void buildMesh() {
        vp.clear();
        vn.clear();
        vuv.clear();
        ti.clear();

        vert_index = 0;
        
        for(int x = 0; x<chunk_size; x++) {
            for(int y = 0; y<chunk_size; y++) {
                for(int z = 0; z<chunk_size; z++) {
                    world_offset = {x, y, z};
                    if(getBlock(x, y, z)) {
                        /* if (!getBlock(x, y, z+1)) */ push_face(DIR::FRONT, 1);
                        /* if (!getBlock(x, y, z-1)) */ push_face(DIR::BACK, 1);
                        /* if (!getBlock(x, y-1, z)) */ push_face(DIR::DOWN, 0);
                        /* if (!getBlock(x, y+1, z)) */ push_face(DIR::UP, 2);
                        /* if (!getBlock(x+1, y, z)) */ push_face(DIR::LEFT, 1);
                        /* if (!getBlock(x-1, y, z)) */ push_face(DIR::RIGHT, 1);
                    }
                }
            }
        }

        std::cout << vp.size() << ", " << vn.size() << ", " << vuv.size() << ", " << ti.size() << "\n";

        mesh->initGPUGeometry(vp, vn, vuv, ti);
    }
private:
    int *voxelMap {};
    
    std::vector<float> vp{};
    std::vector<float> vn{};
    std::vector<float> vuv{};
    std::vector<unsigned int> ti{};

    glm::vec3 world_offset {};
    glm::vec2 tex_offset {};
    glm::vec2 tex_size {1, 1};

    int vert_index {};
};

#endif // CHUNK_HPP