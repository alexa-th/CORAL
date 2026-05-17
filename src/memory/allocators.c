#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>
#include <stdbool.h>
#include "common/atomic.h"
#include "common/utils.h"
#include "memory/dynalloc.h"
#include "memory/allocators.h"



// CUSTOM ALLOCATORS

//- STACK ALLOCATOR

void* StackAllocator_alloc(StackAllocator_t* stackAlloc, size_t blockSize) {
    uintptr_t newStackPtr = (uintptr_t)stackAlloc->stackPtr + (uintptr_t)blockSize;
    void* block = stackAlloc->stackPtr;

    if ((uintptr_t)stackAlloc->stackBase + (uintptr_t)stackAlloc->stackSize >= newStackPtr) {
        stackAlloc->stackPtr = (void*)newStackPtr;
        return block;
    }

    return NULL;
}


void* StackAllocator_alignedAlloc(StackAllocator_t* stackAlloc, size_t blockSize, size_t alignment) {
    if(!StackAllocator_toNextAlignmentBarrier(stackAlloc, alignment)) { return NULL; }

    void* previousStackPtr = stackAlloc->stackPtr;
    void* alignedBlock = StackAllocator_alloc(stackAlloc, blockSize);
    if (!alignedBlock) { stackAlloc->stackPtr = previousStackPtr; }

    return alignedBlock;
}


bool StackAllocator_toNextAlignmentBarrier(StackAllocator_t* stackAlloc, size_t alignment) {
    uintptr_t alignedStackPtr = CORAL_TO_ALIGNMENT((uintptr_t)stackAlloc->stackPtr, (uintptr_t)alignment);

    if ((uintptr_t)stackAlloc->stackBase + (uintptr_t)stackAlloc->stackSize > alignedStackPtr) {
        stackAlloc->stackPtr = (void*)alignedStackPtr;
        return true;
    }

    return false;
}



//- POOL ALLOCATOR

static inline void PoolAllocator_checkElementSize(uint32_t elementSize) {
    CORAL_ASSERT(elementSize >= sizeof(uint32_t), "Tried to create a pool allocator with an elementSize of less than sizeof(uint32_t).");
}


void PoolAllocator_init(PoolAllocator_t* poolAlloc, void* poolBase, uint32_t elementCount, uint32_t elementSize) {
    PoolAllocator_checkElementSize(elementSize);

    *poolAlloc = ((PoolAllocator_t){poolBase, elementCount, elementSize, elementCount, 0U});
}


void PoolAllocator_init_atomic(PoolAllocator_Atomic_t* poolAlloc, void* poolBase, uint32_t elementCount, uint32_t elementSize) {
    PoolAllocator_checkElementSize(elementSize);

    *poolAlloc = ((PoolAllocator_Atomic_t){poolBase, elementCount, elementSize});
    atomic_store_explicit(&poolAlloc->nextFreeIndex, elementCount, memory_order_relaxed);
    atomic_store_explicit(&poolAlloc->nextUnusedFreeIndex, 0U, memory_order_relaxed);
    atomic_thread_fence(memory_order_release);
}


void PoolAllocator_init__atomic(PoolAllocator_Atomic_t* poolAlloc, void* poolBase, uint32_t elementCount, uint32_t elementSize) {
    PoolAllocator_checkElementSize(elementSize);

    *poolAlloc = ((PoolAllocator_Atomic_t){poolBase, elementCount, elementSize});
    atomic_init(&poolAlloc->nextFreeIndex, elementCount);
    atomic_init(&poolAlloc->nextUnusedFreeIndex, 0U);
    atomic_thread_fence(memory_order_release);
}


/*! \remarks
 *      Gradually adds more elements to the usable initialized pool by incrementing \p poolAlloc->nextUnusedFreeIndex
 *      by \c 1U and only doing so if needed, such that initialization is not done all at once, which would waste
 *      time if poolAlloc is not heavily utilized.
 */
void* PoolAllocator_alloc(PoolAllocator_t* poolAlloc) {
    uintptr_t newElement = 0U;

    if (poolAlloc->nextFreeIndex < poolAlloc->elementCount) {
        newElement = (uintptr_t)poolAlloc->poolBase + poolAlloc->nextFreeIndex * poolAlloc->elementSize;
        poolAlloc->nextFreeIndex = *(uint32_t*)newElement;
    }
    else if (poolAlloc->nextUnusedFreeIndex < poolAlloc->elementCount) {
        newElement = (uintptr_t)poolAlloc->poolBase + poolAlloc->nextUnusedFreeIndex * poolAlloc->elementSize;
        poolAlloc->nextUnusedFreeIndex++;
    }

    return (void*)newElement;
}


/*! \remarks Implements the same mechanism as \p PoolAllocator_alloc(), but atomic. */
void* PoolAllocator_alloc_atomic(PoolAllocator_Atomic_t* poolAlloc) {
    uintptr_t newElement = 0U;
    uint32_t newElementIndex = atomic_load_explicit(&poolAlloc->nextFreeIndex, memory_order_acquire);

    while (newElementIndex < poolAlloc->elementCount) {
        newElement = (uintptr_t)poolAlloc->poolBase + newElementIndex * poolAlloc->elementSize;

        bool success = atomic_compare_exchange_weak_explicit(&poolAlloc->nextFreeIndex,
                                                             &newElementIndex,
                                                             *(uint32_t*)newElement,
                                                             memory_order_acq_rel,
                                                             memory_order_acquire
        );

        if (success) { return (void*)newElement; }
    }

    newElementIndex = atomic_load_explicit(&poolAlloc->nextUnusedFreeIndex, memory_order_acquire);

    while (newElementIndex < poolAlloc->elementCount) {
        newElement = (uintptr_t)poolAlloc->poolBase + newElementIndex * poolAlloc->elementSize;

        bool success = atomic_compare_exchange_weak_explicit(&poolAlloc->nextUnusedFreeIndex,
                                                             &newElementIndex,
                                                             newElementIndex + 1U,
                                                             memory_order_acq_rel,
                                                             memory_order_acquire
        );

        if (success) { return (void*)newElement; }
    }

    return NULL;
}


void PoolAllocator_free(PoolAllocator_t* poolAlloc, void* element) {
    *(uint32_t*)element = poolAlloc->nextFreeIndex;
    poolAlloc->nextFreeIndex = ((uintptr_t)element - (uintptr_t)poolAlloc->poolBase) / poolAlloc->elementSize;
}


void PoolAllocator_free_atomic(PoolAllocator_Atomic_t* poolAlloc, void* element) {
    bool success;
    uint32_t nextFreeIndex = atomic_load_explicit(&poolAlloc->nextFreeIndex, memory_order_acquire);
    uint32_t elementIndex = ((uintptr_t)element - (uintptr_t)poolAlloc->poolBase) / poolAlloc->elementSize;

    do {
        *(uint32_t*)element = nextFreeIndex;

        success = atomic_compare_exchange_weak_explicit(&poolAlloc->nextFreeIndex,
                                                        &nextFreeIndex,
                                                        elementIndex,
                                                        memory_order_acq_rel,
                                                        memory_order_acquire
        );
    } while (!success);
}


void PoolAllocator_clear(PoolAllocator_t* poolAlloc) {
    poolAlloc->nextFreeIndex = poolAlloc->elementCount;
    poolAlloc->nextUnusedFreeIndex = 0U;
}



//- DYNAMIC POOL ALLOCATOR

//- - DEFINITIONS

struct DynamicPoolAllocator_Pool {
    LinkedList_DoubleLink_t link;
    uint32_t freeElementCount;
    uint32_t nextFreeIndex;
    uint32_t nextUnusedFreeIndex;
    alignas(max_align_t) unsigned char pool[];
};



//- <

static void DynamicPoolAllocator_addPool(DynamicPoolAllocator_t* dynamicPoolAlloc) {
    size_t poolSize = sizeof(DynamicPoolAllocator_Pool_ot) + dynamicPoolAlloc->elementCount * dynamicPoolAlloc->elementSize;
    DynamicPoolAllocator_Pool_ot* newPool = CORAL_malloc(poolSize);
    CORAL_ASSERT(newPool, "Failed to allocate a dynamic pool.");

    newPool->freeElementCount = dynamicPoolAlloc->elementCount;
    newPool->nextFreeIndex = dynamicPoolAlloc->elementCount;
    newPool->nextUnusedFreeIndex = 0U;

    LinkedList_push_double(&dynamicPoolAlloc->poolList, &newPool->link);
};


void DynamicPoolAllocator_init(DynamicPoolAllocator_t* dynamicPoolAlloc, uint32_t elementCount, uint32_t elementSize) {
    PoolAllocator_checkElementSize(elementSize);

    *dynamicPoolAlloc = ((DynamicPoolAllocator_t){{NULL, NULL}, elementCount, elementSize});
    DynamicPoolAllocator_addPool(dynamicPoolAlloc);
}


void DynamicPoolAllocator_destr(DynamicPoolAllocator_t* dynamicPoolAlloc) {
    LinkedList_DoubleLink_t* poolLink = dynamicPoolAlloc->poolList.headLink.doubleLink;

    while (poolLink) {
        DynamicPoolAllocator_Pool_ot* pool = CORAL_TO_PARENT_PTR(DynamicPoolAllocator_Pool_ot, link, poolLink);
        poolLink = poolLink->next;
        CORAL_free(pool);
    }

    dynamicPoolAlloc->poolList = ((LinkedList_t){NULL, NULL});
}


void* DynamicPoolAllocator_alloc(DynamicPoolAllocator_t* dynamicPoolAlloc) {
    LinkedList_DoubleLink_t* poolLink = dynamicPoolAlloc->poolList.headLink.doubleLink;
    DynamicPoolAllocator_Pool_ot* pool;

    if (!poolLink || !(pool = CORAL_TO_PARENT_PTR(DynamicPoolAllocator_Pool_ot, link, poolLink))->freeElementCount) {
        DynamicPoolAllocator_addPool(dynamicPoolAlloc);
        poolLink = dynamicPoolAlloc->poolList.headLink.doubleLink;
        pool = CORAL_TO_PARENT_PTR(DynamicPoolAllocator_Pool_ot, link, poolLink);
    }

    PoolAllocator_t tempPoolAlloc = ((PoolAllocator_t){pool->pool,
                                                       dynamicPoolAlloc->elementCount,
                                                       dynamicPoolAlloc->elementSize,
                                                       pool->nextFreeIndex,
                                                       pool->nextUnusedFreeIndex
    });

    void* element = PoolAllocator_alloc(&tempPoolAlloc);

    pool->nextFreeIndex = tempPoolAlloc.nextFreeIndex;
    pool->nextUnusedFreeIndex = tempPoolAlloc.nextUnusedFreeIndex;

    if (!(--pool->freeElementCount)) {
        LinkedList_remove_double(&dynamicPoolAlloc->poolList, poolLink);
        LinkedList_append_double(&dynamicPoolAlloc->poolList, poolLink);
    }

    return element;
}


void DynamicPoolAllocator_free(DynamicPoolAllocator_t* dynamicPoolAlloc, void* element) {
    size_t poolSize = dynamicPoolAlloc->elementCount * dynamicPoolAlloc->elementSize;
    LinkedList_DoubleLink_t* poolLink = dynamicPoolAlloc->poolList.headLink.doubleLink;

    while (poolLink) {
        DynamicPoolAllocator_Pool_ot* pool = CORAL_TO_PARENT_PTR(DynamicPoolAllocator_Pool_ot, link, poolLink);

        if ((uintptr_t)element - (uintptr_t)pool->pool >= poolSize) {
            poolLink = poolLink->next;
            continue;
        }

        if (++pool->freeElementCount == dynamicPoolAlloc->elementCount) {
            LinkedList_remove_double(&dynamicPoolAlloc->poolList, poolLink);
            CORAL_free(pool);
            return;
        }

        PoolAllocator_t tempPoolAlloc = ((PoolAllocator_t){pool->pool,
                                                           dynamicPoolAlloc->elementCount,
                                                           dynamicPoolAlloc->elementSize,
                                                           pool->nextFreeIndex,
                                                           pool->nextUnusedFreeIndex
        });

        PoolAllocator_free(&tempPoolAlloc, element);

        pool->nextFreeIndex = tempPoolAlloc.nextFreeIndex;
        pool->nextUnusedFreeIndex = tempPoolAlloc.nextUnusedFreeIndex;

        if (pool->freeElementCount == 1U) {
            LinkedList_remove_double(&dynamicPoolAlloc->poolList, poolLink);
            LinkedList_push_double(&dynamicPoolAlloc->poolList, poolLink);
        }

        return;
    }

    CORAL_ASSERT(0, "Failed to find the pool which allocated the element to be freed.");
}


void DynamicPoolAllocator_clear(DynamicPoolAllocator_t* dynamicPoolAlloc) {
    LinkedList_DoubleLink_t* poolLink = dynamicPoolAlloc->poolList.headLink.doubleLink;
    
    while (poolLink) {
        DynamicPoolAllocator_Pool_ot* pool = CORAL_TO_PARENT_PTR(DynamicPoolAllocator_Pool_ot, link, poolLink);
        pool->freeElementCount = dynamicPoolAlloc->elementCount;
        pool->nextFreeIndex = dynamicPoolAlloc->elementCount;
        pool->nextUnusedFreeIndex = 0U;

        poolLink = poolLink->next;
    }
}
