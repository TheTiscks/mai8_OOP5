#include "stack.h"
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <cstdint>

FixedBlockMemoryResource::FixedBlockMemoryResource(size_t size) 
    : size_(size), offset_(0) {
    buffer_ = ::operator new(size);
}

FixedBlockMemoryResource::~FixedBlockMemoryResource() {
    ::operator delete(buffer_);
}

void* FixedBlockMemoryResource::allocate_from_free_blocks(size_t bytes, size_t alignment) {
    for (auto it = free_blocks_.begin(); it != free_blocks_.end(); ++it) {
        void* block_ptr = it->first;
        size_t block_size = it->second;
        auto block_addr = reinterpret_cast<uintptr_t>(block_ptr);
        auto aligned_addr = (block_addr + alignment - 1) & ~(alignment - 1);
        auto adjustment = aligned_addr - block_addr;
        if (block_size >= bytes + adjustment) {
            free_blocks_.erase(it);
            if (adjustment > 0) {
                free_blocks_.emplace_back(block_ptr, adjustment);
            }
            void* result_ptr = reinterpret_cast<void*>(aligned_addr);
            size_t remaining_size = block_size - bytes - adjustment;
            if (remaining_size > 0) {
                void* remaining_ptr = static_cast<char*>(result_ptr) + bytes;
                free_blocks_.emplace_back(remaining_ptr, remaining_size);
            }
            return result_ptr;
        }
    }
    return nullptr;
}

void FixedBlockMemoryResource::merge_adjacent_blocks() {
    if (free_blocks_.empty()) {
		return;
    }
    free_blocks_.sort([](const auto& a, const auto& b) { // сорт по адресу
        return a.first < b.first;
    });
    auto it = free_blocks_.begin();
    auto next = it;
    ++next;
    while (next != free_blocks_.end()) {
        void* current_end = static_cast<char*>(it->first) + it->second;
        if (current_end == next->first) {
            it->second += next->second;
            next = free_blocks_.erase(next);
        } else {
            it = next;
            ++next;
        }
    }
}

void* FixedBlockMemoryResource::do_allocate(size_t bytes, size_t alignment) {
    void* result = allocate_from_free_blocks(bytes, alignment);
    if (result) {
        allocated_blocks_.emplace_back(result, bytes);
        return result;
    }
    size_t aligned_offset = offset_; // не нашли в свободных
    if (aligned_offset % alignment != 0) {
        aligned_offset += alignment - (aligned_offset % alignment);
    }
    if (aligned_offset + bytes > size_) {
        throw std::bad_alloc();
    }
    void* ptr = static_cast<char*>(buffer_) + aligned_offset;
    allocated_blocks_.emplace_back(ptr, bytes);
    offset_ = aligned_offset + bytes;
    return ptr;
}

void FixedBlockMemoryResource::do_deallocate(void* p, size_t bytes, size_t alignment) {
    for (auto it = allocated_blocks_.begin(); it != allocated_blocks_.end(); ++it) {
        if (it->first == p) {
            free_blocks_.emplace_back(p, bytes);
            allocated_blocks_.erase(it);
            merge_adjacent_blocks();
            return;
        }
    }
}

bool FixedBlockMemoryResource::do_is_equal(const std::pmr::memory_resource& other) const noexcept {
    return this == &other;
}