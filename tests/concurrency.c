#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "unity.h"
#include "concurrency/concurrency.h"



// DEFINITIONS

#define THREAD_COUNT            (8U)
#define COUNT_PER_THREAD        (100U)
#define THREAD_POOL_TASK_COUNT  (THREAD_COUNT * 8U)



// GLOBALS

volatile unsigned counter = 0U;
Mutex_t counterMutex;

static Thread_ot threads[THREAD_COUNT];
static size_t threadArgs[THREAD_COUNT];

static ThreadPool_t threadPool;
static ThreadPool_TaskParameter_t taskParameters[THREAD_POOL_TASK_COUNT];



// THREAD FUNCTIONS

void threadFunc_helloWorld(void* arg) {
    printf("Hello World!\nThread identifier: %ju\nGiven arg (size_t): %zu\n\n", (intmax_t)Thread_getCurrentThread(), *(size_t*)arg);
}


void threadFunc_mutexCount(void* arg) {
    Mutex_lock(&counterMutex);

    for (size_t i = 0U; i < COUNT_PER_THREAD; i++) { counter++; }

    Mutex_unlock(&counterMutex);
}


uintptr_t threadPoolFunc_returnArg(uintptr_t arg) {
    printf("Received %ju from thread pool.\n", (uintmax_t)arg);
    return arg;
}



// THREAD HELPERS

void startThreads(Thread_MainFunc_t* startFunc) {
    size_t successfulThreadStarts = 0U;

    while (successfulThreadStarts < THREAD_COUNT) {
        if (Thread_start(&threads[successfulThreadStarts], startFunc, (void*)&threadArgs[successfulThreadStarts])) { successfulThreadStarts++; }
        else { break; }
    }

    if (successfulThreadStarts) { Thread__join(threads, successfulThreadStarts); }

    TEST_ASSERT_EQUAL_MESSAGE(THREAD_COUNT, successfulThreadStarts, "Failed to start some threads.");
}



// TESTS

void test_helloWorld(void) {
    startThreads(&threadFunc_helloWorld);
}


void test_mutexCount(void) {
    startThreads(&threadFunc_mutexCount);

    TEST_ASSERT_EQUAL_MESSAGE(THREAD_COUNT * COUNT_PER_THREAD, counter, "Mutex failed.");
}


void test_threadPool(void) {
    ThreadPool_init(&threadPool, threads, THREAD_COUNT);

    {
        for (size_t i = 0U; i < THREAD_POOL_TASK_COUNT; i++) { taskParameters[i].arg = i; }
        ThreadPool_startJob(&threadPool, &threadPoolFunc_returnArg, taskParameters, THREAD_POOL_TASK_COUNT);
        ThreadPool_waitOnFinish(&threadPool);
        for (size_t i = 0U; i < THREAD_POOL_TASK_COUNT; i++) { TEST_ASSERT_EQUAL(taskParameters[i].arg, taskParameters[i].returnValue); }
    }

    printf("\n\n");

    {
        ThreadPool_startJob(&threadPool, &threadPoolFunc_returnArg, NULL, THREAD_POOL_TASK_COUNT);
        ThreadPool_waitOnFinish(&threadPool);
    }

    ThreadPool_destr(&threadPool);
}



// UNITY

void setUp(void) {}
void tearDown(void) {}



// MAIN

int main(void) {
    for (size_t i = 0U; i < THREAD_COUNT; i++) { threadArgs[i] = i; }

    UNITY_BEGIN();

    RUN_TEST(test_helloWorld);
    RUN_TEST(test_mutexCount);
    RUN_TEST(test_threadPool);

    return UNITY_END();
}
