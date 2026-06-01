#include "chashmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdatomic.h>

// Define a node in the hashmap.
struct Node_HM_t {
	long m_val; // value of the node
	char padding[PAD];
	_Atomic(struct Node_HM_t *) m_next; // pointer to next node in the bucket
    atomic_bool is_deleted; // logical deletion marker
};

struct List_t {
	_Atomic(Node_HM *) sentinel; // head of bucket list
};

struct hm_t {
    size_t n_buckets;
    List** buckets; // array of buckets
};

// Allocate a hashmap with n_buckets buckets.
HM* alloc_hashmap(size_t n_buckets) {
    if (n_buckets == 0) return NULL;

    HM *hm = malloc(sizeof(HM));
    if (!hm) return NULL;

    hm->n_buckets = n_buckets;
    hm->buckets = calloc(n_buckets, sizeof(List *));
    if (!hm->buckets) {
        free(hm);
        return NULL;
    }

    for (size_t i = 0; i < n_buckets; i++) {
        hm->buckets[i] = malloc(sizeof(List));
        if (!hm->buckets[i])  {
            free_hashmap(hm);
            return NULL;
        }
        atomic_init(&hm->buckets[i]->sentinel, NULL);
    }

    return hm;
}

// Free all nodes and buckets associated with the hashmap.
void free_hashmap(HM* hm) {
    if (!hm) return;

    if (hm->buckets) {
        for (size_t i = 0; i < hm->n_buckets; i++) {
            List *bucket = hm->buckets[i];
            if (!bucket) continue;

            Node_HM *cur = atomic_load_explicit(&bucket->sentinel, memory_order_relaxed);
            while (cur) {
                Node_HM *tmp = cur;
                cur = atomic_load_explicit(&cur->m_next, memory_order_relaxed);
                free(tmp);
            }
            free(bucket);
        }
        free(hm->buckets);
    }

    free(hm);
}

// Insert val into the hashmap.
// Returns 0 on success and 1 on allocation failure.
int insert_item(HM* hm, long val) {
    if (!hm || hm->n_buckets == 0 || !hm->buckets) return 1;
    List *bucket = hm->buckets[(unsigned long)val % hm->n_buckets];
    if (!bucket) return 1;

    Node_HM *node = malloc(sizeof(Node_HM));
    if (!node) return 1;
    node->m_val = val;
    atomic_init(&node->is_deleted, false);  // newly inserted nodes are always visible

    Node_HM *old_sentinel = atomic_load_explicit(&bucket->sentinel, memory_order_acquire); // read the current bucket head
    do {
        atomic_store_explicit(&node->m_next, old_sentinel, memory_order_relaxed); // link the new node to the current head
    } while (!atomic_compare_exchange_weak_explicit(&bucket->sentinel, &old_sentinel, node, memory_order_release, memory_order_acquire)); // try to publish the new node as the new bucket head

    return 0;
}

// Remove one occurrence of val from the hashmap.
// Returns 0 if the item is found and removed, 1 otherwise.
int remove_item(HM* hm, long val)
{
    if (!hm || hm->n_buckets == 0 || !hm->buckets) return 1;

    List *bucket = hm->buckets[(unsigned long)val % hm->n_buckets];
    if (!bucket) return 1;

    _Atomic(Node_HM *) *cur_ptr = &bucket->sentinel;
    Node_HM *cur = atomic_load_explicit(cur_ptr, memory_order_acquire);
    while (cur) {
        Node_HM *next = atomic_load_explicit(&cur->m_next, memory_order_acquire); // snapshot successor for traversal and possible unlink
        if (cur->m_val == val) {
            bool expected = false;  // node must not already be deleted
            if (atomic_compare_exchange_strong_explicit(&cur->is_deleted, &expected, true, memory_order_release, memory_order_acquire)) { // mark the node as logically deleted
                atomic_compare_exchange_weak_explicit(cur_ptr, &cur, next, memory_order_release, memory_order_acquire); // try to physically unlink the node from the list
                return 0;
            }
        }
        cur_ptr = &cur->m_next;
        cur = next;
    }

    return 1;
}

// Check whether val exists in the hashmap.
// Returns 0 if found and 1 otherwise.
int lookup_item(HM* hm, long val) {
    if (!hm || hm->n_buckets == 0 || !hm->buckets) return 1;
    List *bucket = hm->buckets[(unsigned long)val % hm->n_buckets];
    if (!bucket) return 1;

    Node_HM *cur = atomic_load_explicit(&bucket->sentinel, memory_order_acquire);
    while (cur) {
        if (cur->m_val == val && !atomic_load_explicit(&cur->is_deleted, memory_order_acquire)) { // ignore logically deleted nodes
            return 0;
        }
        cur = atomic_load_explicit(&cur->m_next, memory_order_acquire);
    }

    return 1;
}

// Print all buckets and their contents.
void print_hashmap(HM* hm) {
    if (!hm || hm->n_buckets == 0 || !hm->buckets) return;

    for (size_t i = 0; i < hm->n_buckets; i++) {
        List *bucket = hm->buckets[i];
        if (!bucket) continue;
        
        printf("Bucket %zu", i);
        Node_HM *cur = atomic_load_explicit(&bucket->sentinel, memory_order_acquire);
        while (cur) {
            if (!atomic_load_explicit(&cur->is_deleted, memory_order_acquire)) { // skip logically deleted nodes
                printf(" - %ld", cur->m_val);
            }
            cur = atomic_load_explicit(&cur->m_next, memory_order_acquire);
        }
        puts("");
    }
}
