#pragma once

#include "base.hpp"

namespace interpreter {
    struct HeapAllocator {
        static const int PREFIX_SIZE = 16;

        static int64_t& block_size(int8_t* block) {
            return *(int64_t*)(block);
        }
        static int64_t block_size_with_prefix(int8_t* block) {
            return PREFIX_SIZE + block_size(block);
        }
        static int8_t*& block_next_free(int8_t* block) {
            return *(int8_t**)(block+8);
        }
        static int8_t* block_next_imm(int8_t* block) {
            return block + block_size_with_prefix(block);
        }

        vector<int8_t>& heap_mem;
        int8_t* dummy_first = heap_mem.data();
        int8_t* dummy_last = heap_mem.data()+heap_mem.size()-PREFIX_SIZE;

        public:
        HeapAllocator(vector<int8_t>& heap_mem): heap_mem(heap_mem) {
            int8_t* block_first = dummy_first+PREFIX_SIZE;
            block_size(dummy_first) = 0;
            block_next_free(dummy_first) = block_first;
            block_size(block_first) = dummy_last-block_first-PREFIX_SIZE;
            block_next_free(block_first) = dummy_last;
            block_size(dummy_last) = 0;
            block_next_free(dummy_last) = NULL;
        }

        void simple_alloc(size_t bytes) {
            int8_t* prev = dummy_first;
            int8_t* block = block_next_free(prev);
            while (block_size(block) < bytes) {
                prev = block;
                block = block_next_free(block);
                if (block == dummy_last) {
                    throw std::runtime_error("Out of heap memory");
                }
            }
            if (block_size(block) >= PREFIX_SIZE + bytes + 8) {
                // split block
                int8_t* new_block = block + PREFIX_SIZE + bytes;
                block_size(new_block) = block_size(block) - PREFIX_SIZE - bytes;
                block_next_free(new_block) = block_next_free(block);
                block_next_free(prev) = new_block;
                block_size(block) = bytes;
            } else {
                block_next_free(prev) = block_next_free(block);
            }
        }
        void simple_free(int8_t* block) {
            int8_t* prev = dummy_first;
            int8_t* next = block_next_free(prev);
            while (next < block) {
                prev = next;
                next = block_next_free(next);
            }
            if (block_next_imm(prev) == block) {
                if (block_next_imm(block) == next) {
                    // merge with prev and next
                    block_size(prev) += block_size_with_prefix(block) + block_size_with_prefix(next);
                    block_next_free(prev) = block_next_free(next);
                } else {
                    // merge with prev
                    block_size(prev) += block_size_with_prefix(block);
                    block_next_free(prev) = next;
                }
            } else if (block_next_imm(block) == next) {
                // merge with next
                block_size(block) += block_size_with_prefix(next);
                block_next_free(block) = block_next_free(next);
            } else {
                // don't merge with anything but connect blocks
                block_next_free(prev) = block;
                block_next_free(block) = block_next_free(prev);
            }
        }
    };
}