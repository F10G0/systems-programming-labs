#include "cspinlock.h"

#include <stdlib.h>
#include <stdatomic.h>

struct cspinlock {
    atomic_flag is_locked;
};

//acquire the lock
int cspin_lock(cspinlock_t *slock) {
    if (!slock) return 1;
    while (atomic_flag_test_and_set_explicit(&slock->is_locked, memory_order_acquire)) {}
    return 0;
}

//if the lock can not be acquired, return immediately
int cspin_trylock(cspinlock_t *slock) {
    if (!slock) return 1;
    return atomic_flag_test_and_set_explicit(&slock->is_locked, memory_order_acquire);
}

//release the lock
int cspin_unlock(cspinlock_t *slock) {
    if (!slock) return 1;
    atomic_flag_clear_explicit(&slock->is_locked, memory_order_release);
    return 0;
}

//allocate a lock
cspinlock_t* cspin_alloc() {
    cspinlock_t *slock = malloc(sizeof(cspinlock_t));
    if (slock) atomic_flag_clear_explicit(&slock->is_locked, memory_order_relaxed);
    return slock;
}

//free a lock
void cspin_free(cspinlock_t* slock) {
    if (slock) free(slock);
}
