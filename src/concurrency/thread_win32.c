#include "concurrency/concurrency_implementation.h"
#ifdef CORAL_CONCURRENCY_IMPL_WIN32

#include <stddef.h>
#include <stdint.h>
#include <windows.h>
#include <process.h>
#include <processthreadsapi.h>
#include "common/atomic.h"
#include "common/noreturn.h"
#include "common/utils.h"
#include "concurrency/signal.h"
#include "concurrency/threadDefinition.h"
#include "concurrency/thread.h"



// THREAD

//- DEFINITIONS

typedef struct {
    Thread_MainFunc_t* mainFunc;
    void* mainArg;
    volatile CORAL_ATOMIC(uint32_t) hasReceivedStartInfo;
} Thread_StartInfo_t;



//- FUNCTIONS

ThreadID_t Thread_getCurrentThread(void) {
    return GetCurrentThreadId();
}


ThreadID_t Thread_getID(const Thread_ot* thread) {
    return GetThreadId((HANDLE)thread->handle);
}


static unsigned WINAPI Thread_start_base(void* threadStartInfo) {
    Thread_StartInfo_t localStartInfo = *(Thread_StartInfo_t*)threadStartInfo;

    atomic_store_explicit(&((Thread_StartInfo_t*)threadStartInfo)->hasReceivedStartInfo, 1U, memory_order_release);
    Signal_send(&((Thread_StartInfo_t*)threadStartInfo)->hasReceivedStartInfo, false);

    localStartInfo.mainFunc(localStartInfo.mainArg);
    return EXIT_SUCCESS;
}


ThreadID_t Thread_start(Thread_ot* thread, Thread_MainFunc_t* mainFunc, void* mainArg) {
    Thread_StartInfo_t threadStartInfo = {mainFunc, mainArg, 0U};
    unsigned newThreadID;
    uintptr_t newThreadHandle = _beginthreadex(NULL, 0U, &Thread_start_base, &threadStartInfo, 0U, &newThreadID);

    if (!newThreadHandle) { return 0U; }
    thread->handle = newThreadHandle;

    do {
        Signal_await(&threadStartInfo.hasReceivedStartInfo, 0U);
    } while (!atomic_load_explicit(&threadStartInfo.hasReceivedStartInfo, memory_order_acquire));

    return newThreadID;
}


CORAL_NORETURN void Thread_exit(void) {
    _endthreadex(EXIT_SUCCESS);
}


void Thread_join(const Thread_ot* thread) {
    CORAL_ASSERT(WaitForSingleObject((HANDLE)thread->handle, INFINITE) == WAIT_OBJECT_0, "Joining a thread failed.");
    CORAL_ASSERT(CloseHandle((HANDLE)thread->handle), "Closing a thread-handle failed.");
}


void Thread__join(const Thread_ot threads[], size_t threadCount) {
    if (threadCount == 1U) {
        Thread_join(&threads[0]);
        return;
    }

    CORAL_ASSERT(threadCount <= MAXIMUM_WAIT_OBJECTS, "Thread__join received more threads than it can handle (-> more than MAXIMUM_WAIT_OBJECTS).");

    HANDLE threadHandles[MAXIMUM_WAIT_OBJECTS];
    for (size_t i = 0U; i < threadCount; i++) { threadHandles[i] = (HANDLE)threads[i].handle; }

    (void)WaitForMultipleObjects((DWORD)threadCount, &(threadHandles[0]), true, INFINITE);

    for (size_t i = 0U; i < threadCount; i++) { CORAL_ASSERT(CloseHandle(threadHandles[i]), "Closing a thread-handle failed."); }
}

#endif
