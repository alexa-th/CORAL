#include "concurrency/concurrency_implementation.h"
#ifdef CORAL_CONCURRENCY_IMPL_LINUX
#define _GNU_SOURCE

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include "common/atomic.h"
#include "common/noreturn.h"
#include "common/utils.h"
#include "concurrency/signal.h"
#include "concurrency/threadDefinition.h"
#include "concurrency/thread.h"



// THREAD

//- PLATFORM DEPENDENT

//- - DEFINITIONS

typedef struct {
    Thread_MainFunc_t* mainFunc;
    void* mainArg;
    pid_t id; /*!< \brief Will be set by the started thread. */
    volatile CORAL_ATOMIC(uint32_t) hasReceivedStartInfo;
} Thread_StartInfo_t;



//- - THREAD FUNCTIONS

ThreadID_t Thread_getCurrentThread(void) {
    return gettid();
}


ThreadID_t Thread_getID(const Thread_ot* thread) {
    return thread->id;
}


static void* Thread_start_base(void* threadStartInfo) {
    Thread_StartInfo_t localStartInfo = *(Thread_StartInfo_t*)threadStartInfo;

    ((Thread_StartInfo_t*)threadStartInfo)->id = Thread_getCurrentThread();
    atomic_store_explicit(&((Thread_StartInfo_t*)threadStartInfo)->hasReceivedStartInfo, 1U, memory_order_release);
    Signal_send(&((Thread_StartInfo_t*)threadStartInfo)->hasReceivedStartInfo, false);

    localStartInfo.mainFunc(localStartInfo.mainArg);
    return NULL;
}


ThreadID_t Thread_start(Thread_ot* thread, Thread_MainFunc_t* mainFunc, void* mainArg) {
    Thread_StartInfo_t threadStartInfo = {mainFunc, mainArg, 0, 0U};

    if (pthread_create(&thread->pthread, NULL, &Thread_start_base, &threadStartInfo)) { return 0; }

    do {
        Signal_await(&threadStartInfo.hasReceivedStartInfo, 0U);
    } while (!atomic_load_explicit(&threadStartInfo.hasReceivedStartInfo, memory_order_acquire));

    return (thread->id = threadStartInfo.id);
}


CORAL_NORETURN void Thread_exit(void) {
    pthread_exit(NULL);
}


void Thread_join(const Thread_ot* thread) {
    CORAL_ASSERT(!pthread_join(thread->pthread, NULL), "Joining failed.");
}


void Thread__join(const Thread_ot threads[], size_t threadCount) {
    for (size_t i = 0U; i < threadCount; i++) {
        CORAL_ASSERT(!pthread_join(threads[i].pthread, NULL), "Joining failed.");
    }
}

#endif
