#include "chunk_manager.hpp"
#include "chunk_dealer.hpp"

void ChunkManager::updateQueue(glm::vec3 world_pos) {
    this->cam_pos = world_pos;
    glm::ivec2 chunk_pos_center = glm::ivec2((world_pos.x - Chunk::chunk_size.x / 2) / Chunk::chunk_size.x, (world_pos.z - Chunk::chunk_size.z / 2) / Chunk::chunk_size.z);
    for (int i = -load_distance; i <= load_distance; i++) {
        for (int j = -load_distance; j <= load_distance; j++) {
            glm::ivec2 chunk_pos = glm::ivec2(i, j) + chunk_pos_center;
            float dist = chunk_distance(chunk_pos);
            if (dist >= load_distance * Chunk::chunk_size.x) continue;

            Chunk* chunk;
            map_mutex.lock();
            bool empty = chunks.find(chunk_pos) == chunks.end();
            map_mutex.unlock();

            if (empty) {
                chunk = chunk_dealer->getChunk();
                chunk->out_of_thread = false;

                chunk->init(chunk_pos);
                // chunk->concurrent_use = true;
                map_mutex.lock();
                chunks.insert_or_assign(chunk_pos, chunk);
                map_mutex.unlock();
                {
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    taskQueue.push_front(chunk);
                }
                mutex_condition.notify_one();
            }
        }
    }

    cmpChunkPosOrigin::center = glm::vec2(world_pos.x, world_pos.z);
    queue_mutex.lock();
    std::sort(taskQueue.begin(), taskQueue.end(), cmpChunkPosOrigin());
    queue_mutex.unlock();
}

void ChunkManager::unloadUselessChunks() {
    {
        std::unique_lock<std::mutex> lock(map_mutex);
        auto it = chunks.begin();
        while (it != chunks.end()) {
            glm::ivec2 chunk_pos = it->first;
            if (chunk_distance(chunk_pos) >= unload_distance * Chunk::chunk_size.x) {
                if (it->second->concurrent_use)
                    toDelete.push_back(it->second);
                ++it;
            } else {
                ++it;
            }
        }
    }

    auto it = toDelete.begin();
    while (it != toDelete.end()) {
        auto tmp_it = it;
        ++it;
        Chunk* chunk = *tmp_it;

        if (chunk && chunk->chunk_mutex.try_lock()) {
            chunk->concurrent_use = true;
            toDelete.erase(tmp_it);

            if (chunk->hasBeenModified) {
                serializeChunk(chunk->pos);
            }

            chunk_dealer->returnChunk(chunk);

            chunk->chunk_mutex.unlock();
        }
    }
}

void ChunkManager::reloadChunks() {
    queue_mutex.lock();
    thread_pool_paused = true;
    taskQueue.clear();
    queue_mutex.unlock();
    map_mutex.lock();
    for (auto it : chunks) {
        if (it.second->concurrent_use) {
            toDelete.push_back(it.second);
        }
    }
    map_mutex.unlock();

    queue_mutex.lock();
    thread_pool_paused = false;
    queue_mutex.unlock();
    std::cout << "Cleared all chunks\n";
}

void ChunkManager::regenerateOneChunkMesh(glm::ivec2 chunk_pos) {
    map_mutex.lock();
    if (auto search = chunks.find(chunk_pos); search != chunks.end()) {
        search->second->state = ChunkState::BlockArrayInitialized;
    }
    map_mutex.unlock();
}

Chunk* ChunkManager::getChunkFromQueue() {
    bool found_one = false;
    Chunk* chunk{};

    while (!found_one) {
        if (taskQueue.empty()) {
            return nullptr;
        }

        chunk = taskQueue.front();
        taskQueue.pop_front();

        if (!chunk) continue;
        if (chunk->concurrent_use) {
            chunk_dealer->returnChunk(chunk);
            continue;
        }

        bool isInView = chunk_distance(chunk->pos) < unload_distance * Chunk::chunk_size.x;

        map_mutex.lock();
        bool isInMap = chunks.find(chunk->pos) != chunks.end();
        map_mutex.unlock();

        if (isInView && isInMap)
            found_one = true;
        else
            chunk_dealer->returnChunk(chunk);
    }

    return chunk;
}

ChunkManager::ChunkManager() {
    const uint32_t num_threads = 1;  // Max # of threads the system supports
    for (uint32_t ii = 0; ii < num_threads; ++ii) {
        threads.emplace_back(std::thread(&ChunkManager::ThreadLoop, this));
    }
}

void ChunkManager::destroy() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        should_terminate = true;
    }
    mutex_condition.notify_all();
    for (std::thread& active_thread : threads) {
        active_thread.join();
    }
    threads.clear();

    saveChunks();
    for (const auto& [pos, chunk] : chunks) {
        chunk_dealer->returnChunk(chunk);
    }

    chunks.clear();
}

void ChunkManager::ThreadLoop() {
    {
        while (true) {
            Chunk* chunk;
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                mutex_condition.wait(lock, [this] {
                    return (!thread_pool_paused && !taskQueue.empty()) || should_terminate;
                });
                if (should_terminate) {
                    return;
                }
                chunk = getChunkFromQueue();
            }
            // std::cout << "Load or generate one chunk at (" << chunk->pos.x << ", " << chunk->pos.y << ")\n";

            if (!chunk) continue;

            chunk->chunk_mutex.lock();

            if (!deserializeChunk(chunk)) {
                if (chunk) {
                    chunk->voxel_map_from_noise();
                } else {
                    std::cout << "Noooooo chunk creation failed :(((((\n";
                    exit(-1);
                }
            }

            chunk->state = BlockArrayInitialized;

            chunk->generateLightMap();

            regenerateOneChunkMesh(chunk->pos + glm::ivec2(1, 0));
            regenerateOneChunkMesh(chunk->pos + glm::ivec2(-1, 0));
            regenerateOneChunkMesh(chunk->pos + glm::ivec2(0, 1));
            regenerateOneChunkMesh(chunk->pos + glm::ivec2(0, -1));

            chunk->build_mesh();

            chunk->concurrent_use = false;
            chunk->out_of_thread = true;
            chunk->chunk_mutex.unlock();
        }
    }
}

void ChunkManager::saveChunks() {
    map_mutex.lock();
    for (const auto& [pos, chunk] : chunks) {
        if (!chunk->concurrent_use && chunk->hasBeenModified)
            serializeChunk(pos);
    }
    map_mutex.unlock();
}

bool ChunkManager::isInFrustrum(glm::ivec2 chunk_pos, glm::vec2 cam_dir, float fov) {
    if (chunk_distance(chunk_pos) >= view_distance * Chunk::chunk_size.x)
        return false;

    glm::vec2 chunk_center_front = chunk_center(chunk_pos) + cam_dir * (float)(Chunk::chunk_size.x * sqrt(2));

    glm::vec2 chunk_dir = chunk_center_front - glm::vec2(cam_pos.x, cam_pos.z);

    float angle_front = calculateAngle(cam_dir, chunk_dir);

    return abs(angle_front) < fov / 2.0f;
}

void ChunkManager::renderAll(GLuint program, Camera& camera) {
    glm::vec3 cam_pos = camera.get_position();
    glm::vec3 cam_target = camera.get_target();
    glm::vec2 cam_dir = glm::normalize(glm::vec2(cam_target.x - cam_pos.x, cam_target.z - cam_pos.z));

    map_mutex.lock();
    for (const auto& [pos, chunk] : chunks) {
        if (chunk->out_of_thread /* && isInFrustrum(pos, cam_dir, glm::radians(180.f))*/) {
            map_mutex.unlock();
            if (chunk->chunk_mutex.try_lock()) {
                chunk->render(program);
                chunk->chunk_mutex.unlock();
            }
            map_mutex.lock();
        }
    }
    map_mutex.unlock();
}

void ChunkManager::serializeChunk(glm::ivec2 chunk_pos) {
    if (chunks.find(chunk_pos) == chunks.end()) {
        std::cout << "Noooooo couldn't write an inexistant chunk to a file\n";
        return;
    }

    std::stringstream ss{};
    ss << "../map_data/" << chunk_pos.x << "." << chunk_pos.y << ".txt";

    std::ofstream myfile;
    myfile.open(ss.str(), std::ios::binary);

    if (!myfile.is_open()) {
        std::cerr << "Error !! Couldn't open file !!\n";
    } else {
        myfile.write((const char*)chunks[chunk_pos]->voxelMap, Chunk::chunk_size.x * Chunk::chunk_size.y * Chunk::chunk_size.z * sizeof(char));
        myfile.close();

        std::cout << "Wrote one chunk at (" << chunk_pos.x << ", " << chunk_pos.y << ")\n";
    }
}

bool ChunkManager::deserializeChunk(Chunk* chunk) {
    std::stringstream ss{};
    ss << "../map_data/" << chunk->pos.x << "." << chunk->pos.y << ".txt";

    std::ifstream myfile;
    myfile.open(ss.str(), std::ios::binary);

    if (!myfile.is_open()) {
        return false;
    }

    size_t size = Chunk::chunk_size.x * Chunk::chunk_size.y * Chunk::chunk_size.z * sizeof(char);

    myfile.read((char*)chunk->voxelMap, size);

    myfile.close();

    return true;
}

uint8_t ChunkManager::getBlock(glm::ivec3 world_pos) {
    glm::ivec2 chunk_pos = glm::ivec2(
        floor(world_pos.x / (float)Chunk::chunk_size.x),
        floor(world_pos.z / (float)Chunk::chunk_size.z));

    glm::ivec2 chunk_coords = glm::ivec2(world_pos.x, world_pos.z) - chunk_pos * glm::ivec2(Chunk::chunk_size.x, Chunk::chunk_size.z);

    map_mutex.lock();
    auto search = chunks.find(chunk_pos);
    auto end = chunks.end();
    map_mutex.unlock();

    if (search != end) {
        return search->second->getBlock({chunk_coords.x, world_pos.y, chunk_coords.y}, false);
    } else
        return 0;
}

uint8_t ChunkManager::getLightValue(glm::ivec3 world_pos) {
    if (world_pos.y >= Chunk::chunk_size.y) return 0b11111111;
    glm::ivec2 chunk_pos = glm::ivec2(
        floor(world_pos.x / (float)Chunk::chunk_size.x),
        floor(world_pos.z / (float)Chunk::chunk_size.z));

    glm::ivec2 chunk_coords = glm::ivec2(world_pos.x, world_pos.z) - chunk_pos * glm::ivec2(Chunk::chunk_size.x, Chunk::chunk_size.z);

    map_mutex.lock();
    auto search = chunks.find(chunk_pos);
    auto end = chunks.end();
    map_mutex.unlock();

    if (search != end) {
        return search->second->get_light_value({chunk_coords.x, world_pos.y, chunk_coords.y}, false);
    } else
        return 0;
}

void ChunkManager::floodFill(glm::ivec3 world_pos, uint8_t value, bool sky) {
    glm::ivec2 chunk_pos = glm::ivec2(
        floor(world_pos.x / (float)Chunk::chunk_size.x),
        floor(world_pos.z / (float)Chunk::chunk_size.z));

    glm::ivec2 chunk_coords = glm::ivec2(world_pos.x, world_pos.z) - chunk_pos * glm::ivec2(Chunk::chunk_size.x, Chunk::chunk_size.z);

    map_mutex.lock();
    auto search = chunks.find(chunk_pos);
    auto end = chunks.end();
    map_mutex.unlock();

    if (search != end) {
        if (!search->second->concurrent_use)
            search->second->floodFill({chunk_coords.x, world_pos.y, chunk_coords.y}, value, sky);
    }
}

void ChunkManager::setBlock(glm::ivec3 world_pos, uint8_t block, bool rebuild) {
    glm::ivec2 chunk_pos = glm::ivec2(
        floor(world_pos.x / (float)Chunk::chunk_size.x),
        floor(world_pos.z / (float)Chunk::chunk_size.z));

    glm::ivec2 chunk_coords = glm::ivec2(world_pos.x, world_pos.z) - chunk_pos * glm::ivec2(Chunk::chunk_size.x, Chunk::chunk_size.z);

    map_mutex.lock();
    auto search = chunks.find(chunk_pos);
    auto end = chunks.end();
    map_mutex.unlock();

    if (search != end) {
        search->second->setBlock({chunk_coords.x, world_pos.y, chunk_coords.y}, block);
        if (rebuild) {
            if (chunk_coords.x == 0) regenerateOneChunkMesh(chunk_pos + glm::ivec2(-1, 0));
            if (chunk_coords.x == Chunk::chunk_size.x - 1) regenerateOneChunkMesh(chunk_pos + glm::ivec2(1, 0));
            if (chunk_coords.y == 0) regenerateOneChunkMesh(chunk_pos + glm::ivec2(0, -1));
            if (chunk_coords.y == Chunk::chunk_size.z - 1) regenerateOneChunkMesh(chunk_pos + glm::ivec2(0, 1));
        }
    }
}

bool ChunkManager::raycast(glm::vec3 origin, glm::vec3 direction, int nSteps, glm::ivec3& block_pos, glm::ivec3& normal) {
    block_pos.x = int(floor(origin.x));
    block_pos.y = int(floor(origin.y));
    block_pos.z = int(floor(origin.z));

    float sideDistX;
    float sideDistY;
    float sideDistZ;

    float deltaDX = abs(1 / direction.x);
    float deltaDY = abs(1 / direction.y);
    float deltaDZ = abs(1 / direction.z);
    float perpWallDist = -1;

    int stepX;
    int stepY;
    int stepZ;

    int hit = 0;
    int side;

    if (direction.x < 0) {
        stepX = -1;
        sideDistX = (origin.x - block_pos.x) * deltaDX;
    } else {
        stepX = 1;
        sideDistX = (float(block_pos.x) + 1.0 - origin.x) * deltaDX;
    }
    if (direction.y < 0) {
        stepY = -1;
        sideDistY = (origin.y - block_pos.y) * deltaDY;
    } else {
        stepY = 1;
        sideDistY = (float(block_pos.y) + 1.0 - origin.y) * deltaDY;
    }
    if (direction.z < 0) {
        stepZ = -1;
        sideDistZ = (origin.z - block_pos.z) * deltaDZ;
    } else {
        stepZ = 1;
        sideDistZ = (float(block_pos.z) + 1.0 - origin.z) * deltaDZ;
    }

    int step = 1;

    for (int i = 0; i < nSteps; i++) {
        if (sideDistX < sideDistY && sideDistX < sideDistZ) {
            sideDistX += deltaDX;
            block_pos.x += stepX * step;
            side = 0;
        } else if (sideDistY < sideDistX && sideDistY < sideDistZ) {
            sideDistY += deltaDY;
            block_pos.y += stepY * step;
            side = 1;
        } else {
            sideDistZ += deltaDZ;
            block_pos.z += stepZ * step;
            side = 2;
        }

        uint8_t block = getBlock(block_pos);
        // std::cout << "pos: (" << block_pos.x << ", " << block_pos.y << ", " << block_pos.z << "), block: " << (int)block << "\n";
        if (block != 0) {
            if (side == 0) {
                perpWallDist = (block_pos.x - origin.x + (1 - stepX * step) / 2) / direction.x;
                normal = glm::ivec3(1, 0, 0) * -stepX;
            } else if (side == 1) {
                perpWallDist = (block_pos.y - origin.y + (1 - stepY * step) / 2) / direction.y;
                normal = glm::ivec3(0, 1, 0) * -stepY;
            } else {
                perpWallDist = (block_pos.z - origin.z + (1 - stepZ * step) / 2) / direction.z;
                normal = glm::ivec3(0, 0, 1) * -stepZ;
            }

            break;
        }
    }
    return perpWallDist >= 0;
}