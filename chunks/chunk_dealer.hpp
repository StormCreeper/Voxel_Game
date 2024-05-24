#ifndef CHUNK_DEALER_HPP
#define CHUNK_DEALER_HPP

#include <vector>
#include "chunk.hpp"
#include "chunk_manager.hpp"

class ChunkDealer {
    std::vector<Chunk*> chunk_pool{};
    ChunkManager* chunk_manager;

   public:
    ChunkDealer(int n_initial, ChunkManager* chunk_manager) {
        this->chunk_manager = chunk_manager;

        for (int i = 0; i < n_initial; i++) {
            chunk_pool.push_back(new Chunk({}, chunk_manager));
        }
    }
    ~ChunkDealer() {
        for (Chunk* chunk : chunk_pool)
            delete chunk;
    }

    Chunk* getChunk() {
        Chunk* res;
        if (chunk_pool.size() > 0) {
            res = chunk_pool[chunk_pool.size() - 1];
            chunk_pool.pop_back();
        } else {
            res = new Chunk({}, chunk_manager);
        }

        return res;
    }

    void returnChunk(Chunk* chunk) {
        chunk->hasBeenModified = false;
        chunk->concurrent_use = false;
        chunk->state = Allocated;
        chunk_pool.push_back(chunk);
    }
};

#endif  // CHUNK_DEALER_HPP