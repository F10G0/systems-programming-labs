#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>

// Round allocation sizes up to the next multiple of 8 bytes.
#define ALIGN8(size) (((size) + 7) & ~((size_t)7))

// Number of independent allocation arenas.
#define NUM_ARENAS 5

// Extra heap space reserved after contention on the global
// sbrk mutex has been detected.
#define CONTENTION_RESERVE_SIZE 2048

// Metadata stored immediately before each user-visible allocation.
// The doubly linked list is local to the arena that owns the block.
typedef struct BlockHeader {
    int arena;
    int freed;
    size_t size;
    struct BlockHeader *prev;
    struct BlockHeader *next;
} BlockHeader;

// Each arena owns an independent block list and lock.
// Cache-line alignment reduces false sharing between arenas.
typedef struct __attribute__((aligned(64))) Arena {
    pthread_mutex_t lock;
    BlockHeader *head;
    BlockHeader *tail;
} Arena;

// Protects process-wide changes to the program break.
static pthread_mutex_t sbrk_mutex = PTHREAD_MUTEX_INITIALIZER;

// Pool of independently locked allocation arenas.
static Arena arena_pool[NUM_ARENAS] = {
    {PTHREAD_MUTEX_INITIALIZER, NULL, NULL},
    {PTHREAD_MUTEX_INITIALIZER, NULL, NULL},
    {PTHREAD_MUTEX_INITIALIZER, NULL, NULL},
    {PTHREAD_MUTEX_INITIALIZER, NULL, NULL},
    {PTHREAD_MUTEX_INITIALIZER, NULL, NULL},
};

// Arena IDs are assigned round-robin when a thread first allocates.
static _Atomic int next_arena_id = 0;

// Each thread continues using its initially assigned arena.
static _Thread_local int thread_arena_id = -1;

// Once sbrk lock contention is observed, later heap extensions
// reserve additional space to reduce future sbrk calls.
static _Atomic int contention_detected = 0;

// Return the arena assigned to the current thread.
static Arena *get_thread_arena() {
    if (thread_arena_id == -1) {
        thread_arena_id = atomic_fetch_add_explicit(&next_arena_id, 1, memory_order_relaxed) % NUM_ARENAS;
    }
    return &arena_pool[thread_arena_id];
}

// Convert a user payload pointer to its preceding block header.
static BlockHeader *get_header(void *ptr) {
    return (BlockHeader *)ptr - 1;
}

// Return the beginning of a block's user-visible payload.
static unsigned char *get_payload(BlockHeader *header) {
    return (unsigned char *)(header + 1);
}

// Return the first free block large enough for the request.
static BlockHeader *first_hit(BlockHeader *head, size_t size) {
    for (BlockHeader *block = head; block; block = block->next) {
        if (block->freed && block->size >= size) {
            return block;
        }
    }
    return NULL;
}

// Split a block into an allocated prefix and a free remainder.
// The remainder must hold a header and at least 8 payload bytes.
static void split_block(BlockHeader **tail, BlockHeader *block, size_t size) {
    if (block->size < size + sizeof(BlockHeader) + 8) {
        return;
    }

    // Place the remainder header immediately after the requested payload.
    BlockHeader *remainder = (BlockHeader *)(get_payload(block) + size);
    remainder->arena = block->arena;
    remainder->freed = 1;
    remainder->size = block->size - size - sizeof(BlockHeader);
    remainder->prev = block;
    remainder->next = block->next;

    // Insert the remainder into the arena's doubly linked list.
    if (block->next) {
        block->next->prev = remainder;
    } else {
        *tail = remainder;
    }
    block->next = remainder;
    block->size = size;
}

// Merge a block with its next free and physically adjacent block.
static void coalesce_with_next(BlockHeader **tail, BlockHeader *block) {
    BlockHeader *next = block->next;
    if (next && next->freed && get_payload(block) + block->size == (unsigned char *)next) {
        block->size += sizeof(BlockHeader) + next->size;
        block->next = next->next;
        if (next->next) {
            next->next->prev = block;
        } else {
            *tail = block;
        }
    }
}

// Coalesce a newly freed block with neighboring free blocks.
static void merge_blocks(BlockHeader **tail, BlockHeader *block) {
    coalesce_with_next(tail, block);
    BlockHeader *prev = block->prev;
    if (prev && prev->freed) {
        coalesce_with_next(tail, prev);
    }
}

// Extend the process heap and append the new block to an arena.
static BlockHeader *extend_arena_heap(Arena *arena, size_t size) {
    // Before contention is detected, trylock also acts as contention detection.
    // After contention is detected, acquire the global lock directly.
    if (contention_detected || pthread_mutex_trylock(&sbrk_mutex)) {
        contention_detected = 1;
        pthread_mutex_lock(&sbrk_mutex);
    }

    // Reserve extra payload space after contention has been observed.
    size_t reserve_size = contention_detected * CONTENTION_RESERVE_SIZE;

    // Request one contiguous region for the header, payload and reserve.
    BlockHeader *block = (BlockHeader *)sbrk(sizeof(BlockHeader) + size + reserve_size);
    pthread_mutex_unlock(&sbrk_mutex);
    if (block == (void *)-1) {
        return NULL;
    }
    
    // Initialize the newly obtained block.
    block->arena = (int)(arena - arena_pool);
    block->freed = 0;
    block->size = size + reserve_size;
    block->prev = arena->tail;
    block->next = NULL;

    // Append the block to the owning arena's list.
    if (!arena->head) {
        arena->head = block;
    } else {
        arena->tail->next = block;
    }
    arena->tail = block;

    // Split the reserved space into a reusable free block.
    split_block(&arena->tail, block, size);
    return block;
}

// Allocate from an existing free block or extend the heap.
static void *arena_malloc(Arena *arena, size_t size) {
    BlockHeader *block = first_hit(arena->head, size);
    if (block) {
        split_block(&arena->tail, block, size);
        block->freed = 0;
    } else {
        block = extend_arena_heap(arena, size);
    }
    return block ? get_payload(block) : NULL;
}

// Allocate an 8-byte-aligned block from the current thread's arena.
void *malloc(size_t size) {
    size = ALIGN8(size);
    if (size == 0) {
        return NULL;
    }
    Arena *arena = get_thread_arena();

    pthread_mutex_lock(&arena->lock);
    void *ptr = arena_malloc(arena, size);
    pthread_mutex_unlock(&arena->lock);

    return ptr;
}

// Allocate an array and initialize the requested bytes to zero.
void *calloc(size_t nitems, size_t nsize) {
    // Reject multiplication overflow.
    if (nitems != 0 && nsize > __SIZE_MAX__ / nitems) {
        return NULL;
    }
    size_t size = nitems * nsize;

    void *ptr = malloc(size);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

// Mark a block as free and merge adjacent free blocks.
static void arena_free(Arena *arena, BlockHeader *block) {
    block->freed = 1;
    merge_blocks(&arena->tail, block);
}

// Return a block to its owning arena.
void free(void *ptr) {
    if (!ptr) {
        return;
    }
    BlockHeader *block = get_header(ptr);
    Arena *arena = &arena_pool[block->arena];

    pthread_mutex_lock(&arena->lock);
    arena_free(arena, block);
    pthread_mutex_unlock(&arena->lock);
}

// Resize a block while its owning arena is locked.
static void *arena_realloc(Arena *arena, BlockHeader *block, size_t size) {
    void *old_ptr = get_payload(block);

    // Preserve the original payload size before possible coalescing.
    size_t copy_size = block->size;

    // First try to grow in place into the following free block.
    coalesce_with_next(&arena->tail, block);
    if (size <= block->size) {
        split_block(&arena->tail, block, size);
        return old_ptr;
    }

    // Otherwise allocate a new block and copy the previous payload.
    void *new_ptr = arena_malloc(arena, size);
    if (new_ptr) {
        memcpy(new_ptr, old_ptr, copy_size);
        arena_free(arena, block);
    }
    return new_ptr;
}

// Resize an allocation while preserving its existing contents.
void *realloc(void *ptr, size_t size) {
    size = ALIGN8(size);
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    
    // A null input pointer is equivalent to malloc.
    if (!ptr) {
        return malloc(size);
    }
    BlockHeader *block = get_header(ptr);
    Arena *arena = &arena_pool[block->arena];

    pthread_mutex_lock(&arena->lock);
    void *new_ptr = arena_realloc(arena, block, size);
    pthread_mutex_unlock(&arena->lock);

    return new_ptr;
}
