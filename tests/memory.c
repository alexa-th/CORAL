#include "unity.h"
#include "common/atomic.h"
#include "common/utils.h"
#include "memory/memory.h"
#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>



// DEFINITIONS

#define ALLOC_BUFFER_SIZE ((size_t)32U)



// GLOBALS

static unsigned char allocBuffer[ALLOC_BUFFER_SIZE];



// TESTS

void test_stackAlloc(void) {
    StackAllocator_t stackAlloc = {&allocBuffer[0U], &allocBuffer[0U], ALLOC_BUFFER_SIZE};

    {
        int* a = StackAllocator_alloc(&stackAlloc, sizeof(int));

        TEST_ASSERT_EQUAL(&allocBuffer[0U], a);
    }

    {
        (void)StackAllocator_alloc(&stackAlloc, 1U);
        int* b = StackAllocator_alignedAlloc(&stackAlloc, sizeof(int), alignof(int));

        TEST_ASSERT_MESSAGE(!((uintptr_t)b % alignof(int)), "Pointer was not properly aligned.");
    }

    {
        StackAllocator_toNextAlignmentBarrier(&stackAlloc, alignof(max_align_t));

        TEST_ASSERT_MESSAGE(!((uintptr_t)stackAlloc.stackPtr % alignof(max_align_t)), "Stack pointer was not properly aligned.");
    }
}


void test_poolAlloc(void) {
    PoolAllocator_t poolAlloc;
    PoolAllocator_init(&poolAlloc, allocBuffer, ALLOC_BUFFER_SIZE / sizeof(uintptr_t), sizeof(uintptr_t));

    {
        uintptr_t* a = PoolAllocator_alloc(&poolAlloc);

        TEST_ASSERT_EQUAL(allocBuffer, a);
    }

    {
        uintptr_t* a = PoolAllocator_alloc(&poolAlloc);
        PoolAllocator_free(&poolAlloc, a);

        TEST_ASSERT_EQUAL(1U, poolAlloc.nextFreeIndex);
        TEST_ASSERT_EQUAL(2U, poolAlloc.nextUnusedFreeIndex);
    }

    {
        PoolAllocator_clear(&poolAlloc);

        TEST_ASSERT_EQUAL(poolAlloc.elementCount, poolAlloc.nextFreeIndex);
        TEST_ASSERT_EQUAL(0U, poolAlloc.nextUnusedFreeIndex);
    }
}


void test_atomicPoolAlloc(void) {
    PoolAllocator_Atomic_t atomicPoolAlloc;
    PoolAllocator_init_atomic(&atomicPoolAlloc, allocBuffer, ALLOC_BUFFER_SIZE / sizeof(uintptr_t), sizeof(uintptr_t));

    {
        uintptr_t* a = PoolAllocator_alloc_atomic(&atomicPoolAlloc);

        TEST_ASSERT_EQUAL(allocBuffer, a);
    }

    {
        uintptr_t* a = PoolAllocator_alloc_atomic(&atomicPoolAlloc);
        PoolAllocator_free_atomic(&atomicPoolAlloc, a);

        TEST_ASSERT_EQUAL(1U, atomic_load_explicit(&atomicPoolAlloc.nextFreeIndex, memory_order_relaxed));
        TEST_ASSERT_EQUAL(2U, atomic_load_explicit(&atomicPoolAlloc.nextUnusedFreeIndex, memory_order_relaxed));
    }
}


void test_dynamicPoolAlloc(void) {
    DynamicPoolAllocator_t dynamicPoolAlloc;
    DynamicPoolAllocator_init(&dynamicPoolAlloc, ALLOC_BUFFER_SIZE / sizeof(uintptr_t), sizeof(uintptr_t));

    uintptr_t* a[ALLOC_BUFFER_SIZE / sizeof(uintptr_t)];
    uintptr_t* b;


    {
        for (size_t i = 0U; i < CORAL_ARRAY_LENGTH(a); i++) {
            a[i] = DynamicPoolAllocator_alloc(&dynamicPoolAlloc);
        }

        TEST_ASSERT_EQUAL_MESSAGE(dynamicPoolAlloc.poolList.headLink.doubleLink,
                                  dynamicPoolAlloc.poolList.tailLink.doubleLink,
                                  "Head and tail of poolList were not equal"
        );

        b = DynamicPoolAllocator_alloc(&dynamicPoolAlloc);

        TEST_ASSERT_EQUAL_MESSAGE(dynamicPoolAlloc.poolList.headLink.doubleLink->next,
                                  dynamicPoolAlloc.poolList.tailLink.doubleLink,
                                  "Head and tail of poolList were not immediately connected"
        );
    }

    {
        DynamicPoolAllocator_free(&dynamicPoolAlloc, b);

        for (size_t i = 0U; i < CORAL_ARRAY_LENGTH(a); i++) {
            DynamicPoolAllocator_free(&dynamicPoolAlloc, a[i]);
        }

        TEST_ASSERT_EQUAL_MESSAGE(NULL,
                                  dynamicPoolAlloc.poolList.headLink.doubleLink,
                                  "poolList was not completely deallocated"
        );
    }
}



// UNITY

void setUp(void) {}
void tearDown(void) {}



// MAIN

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_stackAlloc);
    RUN_TEST(test_poolAlloc);
    RUN_TEST(test_atomicPoolAlloc);
    RUN_TEST(test_dynamicPoolAlloc);

    return UNITY_END();
}
