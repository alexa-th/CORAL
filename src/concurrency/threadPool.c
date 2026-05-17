#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "common/atomic.h"
#include "concurrency/signal.h"
#include "concurrency/thread.h"
#include "concurrency/threadPool.h"



// THREAD POOL

//- DEFINITIONS

#define CORAL_CONCURRENCY_THREAD_POOL_STATUS_JOB_AVAILABLE  ((uint32_t)0x1)
#define CORAL_CONCURRENCY_THREAD_POOL_STATUS_TERMINATE      ((uint32_t)0x2)



//<

static void ThreadPool_workerThread_main(void* arg) {
    ThreadPool_t* const threadPool = arg;
    uint32_t currentStatus;

    do {
        while (!(currentStatus = atomic_load_explicit(&threadPool->currentStatus, memory_order_acquire))) {
            Signal_await(&threadPool->currentStatus, currentStatus);
        }

        if (!(currentStatus & CORAL_CONCURRENCY_THREAD_POOL_STATUS_JOB_AVAILABLE)) { continue; }

        atomic_fetch_add_explicit(&threadPool->busyThreadCount, 1U, memory_order_acq_rel);

        while (true) {
            uint32_t remainingTaskCount = atomic_load_explicit(&threadPool->remainingTaskCount, memory_order_acquire);
            bool success;

            do {
                if (!remainingTaskCount) { goto finishedJob; }
                success = atomic_compare_exchange_weak_explicit(&threadPool->remainingTaskCount,
                                                                &remainingTaskCount,
                                                                remainingTaskCount - 1U,
                                                                memory_order_acq_rel,
                                                                memory_order_acquire
                );
            } while (!success);

            uint32_t taskParameterIndex = remainingTaskCount - 1U;

            if (threadPool->taskParameters) {
                ThreadPool_TaskParameter_t* currentTaskParameter = &threadPool->taskParameters[taskParameterIndex];
                currentTaskParameter->returnValue = threadPool->taskFunc(currentTaskParameter->arg);
            }
            else {
                (void)threadPool->taskFunc(taskParameterIndex);
            }

            // This worker finished the last task, must do some cleanup
            if (!taskParameterIndex) {
                (void)atomic_fetch_and_explicit(&threadPool->currentStatus,
                                                ~CORAL_CONCURRENCY_THREAD_POOL_STATUS_JOB_AVAILABLE,
                                                memory_order_acq_rel
                );
                goto finishedJob;
            }
        }

finishedJob:
        // This worker was the last to be busy, must do some cleanup
        if (atomic_fetch_sub_explicit(&threadPool->busyThreadCount, 1U, memory_order_acq_rel) == 1U) {
            Signal_send(&threadPool->busyThreadCount, true);
        }
    } while (!(currentStatus & CORAL_CONCURRENCY_THREAD_POOL_STATUS_TERMINATE));
}


bool ThreadPool_init(ThreadPool_t* threadPool, Thread_ot threads[], uint32_t threadCount) {
    *threadPool = ((ThreadPool_t){&threads[0U], NULL, NULL, threadCount, 0U, 0U, 0U});

    for (size_t i = 0U; i < threadCount; i++) {
        if (!Thread_start(&threads[i], &ThreadPool_workerThread_main, threadPool)) {
            if (i) {
                threadPool->threadCount = i;
                ThreadPool_destr(threadPool);
            }

            return false;
        }
    }

    return true;
}


void ThreadPool_destr(ThreadPool_t* threadPool) {
    (void)atomic_fetch_or_explicit(&threadPool->currentStatus,
                                   CORAL_CONCURRENCY_THREAD_POOL_STATUS_TERMINATE,
                                   memory_order_acq_rel
    );

    Signal_send(&threadPool->currentStatus, true);
    Thread__join(threadPool->threads, threadPool->threadCount);
    threadPool->threads = NULL;
}


void ThreadPool_startJob(ThreadPool_t* threadPool,
                         ThreadPool_TaskFunc_t* taskFunc,
                         ThreadPool_TaskParameter_t taskParameters[],
                         uint32_t taskCount) {
    ThreadPool_waitOnFinish(threadPool);

    threadPool->taskFunc = taskFunc;
    threadPool->taskParameters = taskParameters;

    atomic_store_explicit(&threadPool->remainingTaskCount, taskCount, memory_order_relaxed);
    atomic_store_explicit(&threadPool->currentStatus,
                          CORAL_CONCURRENCY_THREAD_POOL_STATUS_JOB_AVAILABLE,
                          memory_order_release
    );

    Signal_send(&threadPool->currentStatus, true);
}


void ThreadPool_waitOnFinish(ThreadPool_t* threadPool) {
    uint32_t busyThreadCount;

    while ((busyThreadCount = atomic_load_explicit(&threadPool->busyThreadCount, memory_order_acquire)) ||
           (atomic_load_explicit(&threadPool->currentStatus, memory_order_acquire) &
            CORAL_CONCURRENCY_THREAD_POOL_STATUS_JOB_AVAILABLE
           )
    ) {
        Signal_await(&threadPool->busyThreadCount, busyThreadCount);
    }
}
