#include "cspinlock.h"
#include "chashmap.h"

#include <stdio.h>
#include <stdlib.h>

// Define a node in the hashmap.
struct Node_HM_t {
	long m_val; // value of the node
	char padding[PAD];
	struct Node_HM_t* m_next; // pointer to next node in the bucket
};

struct List_t {
    cspinlock_t *slock; // per-bucket spinlock
	Node_HM* sentinel; // head of bucket list
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

    // Allocate bucket array and initialize one spinlock per bucket.
    // Fine-grained locking allows different buckets to be accessed concurrently.
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
        hm->buckets[i]->slock = cspin_alloc();
        if (!hm->buckets[i]->slock)  {
            free_hashmap(hm);
            return NULL;
        }
        hm->buckets[i]->sentinel = NULL;
    }

    return hm;
}

// Free all nodes, buckets and associated spinlocks.
void free_hashmap(HM* hm) {
    if (!hm) return;

    // Destroy all bucket lists and associated spinlocks.
    if (hm->buckets) {
        for (size_t i = 0; i < hm->n_buckets; i++) {
            List *bucket = hm->buckets[i];
            if (!bucket) continue;

            Node_HM *cur = bucket->sentinel;
            while (cur) {
                Node_HM *tmp = cur;
                cur = cur->m_next;
                free(tmp);
            }
            cspin_free(bucket->slock);
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
    if (!bucket || !bucket->slock) return 1;

    Node_HM *node = malloc(sizeof(Node_HM));
    if (!node) return 1;
    node->m_val = val;

    cspin_lock(bucket->slock);
    node->m_next = bucket->sentinel; // insert at the head of the bucket list
    bucket->sentinel = node; // O(1) insertion without traversal
    cspin_unlock(bucket->slock);

    return 0;
}

// Remove one occurrence of val from the hashmap.
// Returns 0 if the item is found and removed, 1 otherwise.
int remove_item(HM* hm, long val) {
    if (!hm || hm->n_buckets == 0 || !hm->buckets) return 1;
    List *bucket = hm->buckets[(unsigned long)val % hm->n_buckets];
    if (!bucket || !bucket->slock) return 1;

    cspin_lock(bucket->slock);
    // Pointer-to-pointer traversal.
    // cur_ptr always points to the link that references the current node,
    // allowing head and non-head deletions to be handled uniformly.
    Node_HM **cur_ptr = &bucket->sentinel;
    while (*cur_ptr) {
        Node_HM *cur = *cur_ptr;
        if (cur->m_val == val) {
            *cur_ptr = cur->m_next;
            cspin_unlock(bucket->slock);
            free(cur);
            return 0;
        }
        cur_ptr = &cur->m_next;
    }
    cspin_unlock(bucket->slock);
    
    return 1;
}

// Check whether val exists in the hashmap.
// Returns 0 if found and 1 otherwise.
int lookup_item(HM* hm, long val) {
    if (!hm || hm->n_buckets == 0 || !hm->buckets) return 1;
    List *bucket = hm->buckets[(unsigned long)val % hm->n_buckets];
    if (!bucket || !bucket->slock) return 1;

    cspin_lock(bucket->slock); // protect traversal from concurrent updates
    for (Node_HM *cur = bucket->sentinel; cur; cur = cur->m_next) {
        if (cur->m_val == val) {
            cspin_unlock(bucket->slock);
            return 0;
        }
    }
    cspin_unlock(bucket->slock);

    return 1;
}

// Print all buckets and their contents.
void print_hashmap(HM* hm) {
    if (!hm || hm->n_buckets == 0 || !hm->buckets) return;

    for (size_t i = 0; i < hm->n_buckets; i++) {
        List *bucket = hm->buckets[i];
        if (!bucket || !bucket->slock) continue;

        cspin_lock(bucket->slock); // prevent concurrent modifications while printing
        printf("Bucket %zu", i);
        for (Node_HM *cur = bucket->sentinel; cur; cur = cur->m_next) {
            printf(" - %ld", cur->m_val);
        }
        puts("");
        cspin_unlock(bucket->slock);
    }
}
