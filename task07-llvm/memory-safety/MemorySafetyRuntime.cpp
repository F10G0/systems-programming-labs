#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-reserved-identifier"

#include <iterator>
#include <map>

namespace {
    // Metadata for tracked heap and stack allocations.
    struct MemoryBlock {
        size_t size;
        bool freed;
    };

    // Allocations indexed by their starting addresses.
    std::map<uintptr_t, MemoryBlock> blocks;
    const uintptr_t shadow = 16;

    uintptr_t to_addr(void *ptr) {
        return reinterpret_cast<uintptr_t>(ptr);
    }

    void report_error() {
        std::fprintf(stderr, "Illegal memory access\n");
        std::abort();
    }
}

extern "C" {
__attribute__((used))
void __runtime_init() {}

__attribute__((used))
void __runtime_cleanup() {}

// Check the closest allocations before and after the accessed address.
__attribute__((used))
void __runtime_check_addr(void *ptr, size_t size) {
    uintptr_t addr = to_addr(ptr);

    auto next = blocks.upper_bound(addr);
    if (next != blocks.begin()) {
        auto prev = std::prev(next);
        uintptr_t block_begin = prev->first;
        MemoryBlock &block = prev->second;
        uintptr_t block_end = block_begin + block.size;
        if (addr < block_end + shadow) {
            if (!block.freed && addr + size <= block_end) {
                return;
            }
            report_error();
        }
    }
    
    if (next != blocks.end()) {
        uintptr_t block_begin = next->first;
        if (addr >= block_begin - shadow) {
            report_error();
        }
    }
}

// Allocate and register a heap block.
__attribute__((used))
void *__runtime_malloc(size_t size) {
    void *ptr = std::malloc(size);
    if (ptr) {
        blocks[to_addr(ptr)] = {size, false};
    }
    return ptr;
}

// Register a stack allocation.
__attribute__((used))
void __runtime_alloc(void *ptr, size_t size) {
    blocks[to_addr(ptr)] = {size, false};
}

// Release a registered block and detect invalid or repeated frees.
__attribute__((used))
void __runtime_free(void *ptr) {
    if (!ptr) return;
    uintptr_t addr = to_addr(ptr);
    auto it = blocks.find(addr);
    if (it != blocks.end()) {
        MemoryBlock &block = it->second;
        if (!block.freed) {
            block.freed = true;
            std::free(ptr);
            return;
        }
    }
    report_error();
}

}

#pragma clang diagnostic pop
