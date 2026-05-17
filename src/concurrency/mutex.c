#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include "common/atomic.h"
#include "common/utils.h"
#include "concurrency/signal.h"
#include "concurrency/thread.h"
#include "concurrency/mutex.h"



// MUTEXES

//- DEFINITIONS

#define CORAL_CONCURRENCY_MULTI_OWNED_MUTEX_SINGLY_LOCKED UINT32_MAX



//- FUNCTIONS

void Mutex_init(Mutex_t* mutex) {
    mutex->owner = 0;
    atomic_init(&mutex->lockCount, 0U);
}


/*! \param[out] lastLockCount   Will be set to the last lock count of \p mutex. */
static bool Mutex_tryLock_base(Mutex_t* mutex, uint32_t* lastLockCount) {
    ThreadID_t currentThread = Thread_getCurrentThread();

    if ((*lastLockCount = atomic_load_explicit(&mutex->lockCount, memory_order_acquire))) {
        if (mutex->owner == currentThread) {
            atomic_fetch_add_explicit(&mutex->lockCount, 1U, memory_order_relaxed);
            return true;
        }
        else { return false; }
    }

    bool success = atomic_compare_exchange_strong_explicit(&mutex->lockCount, lastLockCount, 1U, memory_order_acq_rel, memory_order_acquire);

    if (success) { mutex->owner = currentThread; }
    return success;
}


bool Mutex_tryLock(Mutex_t* mutex) {
    return Mutex_tryLock_base(mutex, &((uint32_t){0U}));
}


void Mutex_lock(Mutex_t* mutex) {
    uint32_t lastLockCount;
    while (!Mutex_tryLock_base(mutex, &lastLockCount)) { Signal_await(&mutex->lockCount, lastLockCount); }
}


void Mutex_unlock(Mutex_t* mutex) {
#if CORAL_HAS_ASSERT
    ThreadID_t owner = mutex->owner;

    CORAL_ASSERT(owner, "Tried to unlock a mutex that was not locked.");
    CORAL_ASSERT(owner == Thread_getCurrentThread(), "Tried to unlock a mutex that was not locked by the calling thread.");
#endif

    if (atomic_load_explicit(&mutex->lockCount, memory_order_relaxed) == 1U) {
        mutex->owner = 0;
        atomic_store_explicit(&mutex->lockCount, 0U, memory_order_release);
        Signal_send(&mutex->lockCount, false);
    }
    else { atomic_fetch_sub_explicit(&mutex->lockCount, 1U, memory_order_relaxed); }
}


void MultiOwnedMutex_init(MultiOwnedMutex_t* multiMutex) {
    Mutex_init(&multiMutex->singleOwnerMutex);
    atomic_init(&multiMutex->ownerCount, 0U);
}


static bool MultiOwnedMutex_tryLock_base(MultiOwnedMutex_t* multiMutex, uint32_t* lastOwnerCount) {
    *lastOwnerCount = atomic_load_explicit(&multiMutex->ownerCount, memory_order_acquire);

    if (*lastOwnerCount >= CORAL_CONCURRENCY_MULTI_OWNED_MUTEX_SINGLY_LOCKED - 1U) { return false; }

    return atomic_compare_exchange_strong_explicit(&multiMutex->ownerCount, lastOwnerCount, *lastOwnerCount + 1U, memory_order_acq_rel, memory_order_acquire);
}


bool MultiOwnedMutex_tryLock(MultiOwnedMutex_t* multiMutex) {
    return MultiOwnedMutex_tryLock_base(multiMutex, &((uint32_t){0U}));
}


void MultiOwnedMutex_lock(MultiOwnedMutex_t* multiMutex) {
    uint32_t lastOwnerCount;
    while (!MultiOwnedMutex_tryLock_base(multiMutex, &lastOwnerCount)) {
        if (lastOwnerCount >= CORAL_CONCURRENCY_MULTI_OWNED_MUTEX_SINGLY_LOCKED - 1U) { Signal_await(&multiMutex->ownerCount, lastOwnerCount); }
    }
}


void MultiOwnedMutex_unlock(MultiOwnedMutex_t* multiMutex) {
    uint32_t lastOwnerCount = atomic_load_explicit(&multiMutex->ownerCount, memory_order_acquire);

    do {
        CORAL_ASSERT(lastOwnerCount, "Tried to unlock a multi-owned mutex that was not locked.");
        CORAL_ASSERT(lastOwnerCount != CORAL_CONCURRENCY_MULTI_OWNED_MUTEX_SINGLY_LOCKED, "Tried to unlock a multi-owned mutex that was exclusively locked.");
    } while (!atomic_compare_exchange_weak_explicit(&multiMutex->ownerCount, &lastOwnerCount, lastOwnerCount - 1U, memory_order_acq_rel, memory_order_acquire));

    if (lastOwnerCount == 1U) { Signal_send(&multiMutex->ownerCount, false); }
}


static bool MultiOwnedMutex_tryLock_single_base(MultiOwnedMutex_t* multiMutex, uint32_t* lastOwnerCount) {
    *lastOwnerCount = atomic_load_explicit(&multiMutex->ownerCount, memory_order_acquire);

    bool success = !(*lastOwnerCount) && atomic_compare_exchange_strong_explicit(&multiMutex->ownerCount,
                                                                                 lastOwnerCount,
                                                                                 CORAL_CONCURRENCY_MULTI_OWNED_MUTEX_SINGLY_LOCKED,
                                                                                 memory_order_acq_rel,
                                                                                 memory_order_acquire
    );

    // Only attempt to lock singleOwnerMutex if ownerCount was successfully set to anounce a single lock or multiMutex is already singly locked by the calling thread
    if (success || (multiMutex->singleOwnerMutex.owner == Thread_getCurrentThread())) {
        CORAL_ASSERT(Mutex_tryLock(&multiMutex->singleOwnerMutex), "Locking failed.");
        return true;
    }

    return false;
}


bool MultiOwnedMutex_tryLock_single(MultiOwnedMutex_t* multiMutex) {
    return MultiOwnedMutex_tryLock_single_base(multiMutex, &((uint32_t){0U}));
}


void MultiOwnedMutex_lock_single(MultiOwnedMutex_t* multiMutex) {
    uint32_t lastOwnerCount;
    while (!MultiOwnedMutex_tryLock_single_base(multiMutex, &lastOwnerCount)) { Signal_await(&multiMutex->ownerCount, lastOwnerCount); }
}


void MultiOwnedMutex_lock__single(MultiOwnedMutex_t* multiMutex) {
    MultiOwnedMutex_unlock(multiMutex);
    MultiOwnedMutex_lock_single(multiMutex);
}


static bool MultiOwnedMutex_unlock_single_base(MultiOwnedMutex_t* multiMutex, uint32_t initOwnerCount) {
    uint32_t ownerCount = atomic_load_explicit(&multiMutex->ownerCount, memory_order_acquire);
    CORAL_ASSERT(ownerCount == CORAL_CONCURRENCY_MULTI_OWNED_MUTEX_SINGLY_LOCKED, "Given multiMutex is not singly locked");

    Mutex_unlock(&multiMutex->singleOwnerMutex);

    // Only reopens multiMutex to collective locking after the calling thread completely unlocks it
    if (!atomic_load_explicit(&multiMutex->singleOwnerMutex.lockCount, memory_order_relaxed)) {
        atomic_store_explicit(&multiMutex->ownerCount, initOwnerCount, memory_order_release);
        Signal_send(&multiMutex->ownerCount, false);
        return true;
    }

    return false;
}


void MultiOwnedMutex_unlock_single(MultiOwnedMutex_t* multiMutex) {
    (void)MultiOwnedMutex_unlock_single_base(multiMutex, 0U);
}


bool MultiOwnedMutex_unlock__single(MultiOwnedMutex_t* multiMutex) {
    return MultiOwnedMutex_unlock_single_base(multiMutex, 1U);
}
