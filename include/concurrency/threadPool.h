#ifndef CORAL_CONCURRENCY_THREAD_POOL
#define CORAL_CONCURRENCY_THREAD_POOL

/*! \file */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "../common/api.h"
#include "../common/atomic.h"
#include "./threadDefinition.h"



// THREAD POOL

/*! \defgroup CORAL_CONCURRENCY_THREAD_POOL ThreadPool_t
 *  \ingroup CORAL_CONCURRENCY
 *  \copybrief ThreadPool_t
 *  \addtogroup CORAL_CONCURRENCY_THREAD_POOL
 *  @{
 */

/*! \brief A function that will be called from the threads of a \p ThreadPool_t. */
typedef uintptr_t (ThreadPool_TaskFunc_t)(uintptr_t arg);


/*! \brief The parameters of a single call to a \p ThreadPool_TaskFunc_t. */
typedef struct {
    uintptr_t arg;
    uintptr_t returnValue;     // TODO: Maybe implement return values for Thread_StartFunc_t
} ThreadPool_TaskParameter_t;


/*! \brief A group of threads that can be assigned to work on a job in parallel.
 *
 *  In the context of a \p ThreadPool_t, a task is a single call to a \p ThreadPool_TaskFunc_t with some
 *  argument \p arg, while a job is a collection of tasks that all call the same \p ThreadPool_TaskFunc_t.
 *
 *  \note
 *      Once a \p ThreadPool_t has been initialized using \p ThreadPool_init(), all functions that
 *      should operate on that \p ThreadPool_t must be passed the same \p ThreadPool_t* as was passed
 *      to \p ThreadPool_init() up to and including the call to \p ThreadPool_destr().
 *      This means that a \p ThreadPool_t may not be moved.
 */
typedef struct {
    const Thread_ot* threads;
    ThreadPool_TaskFunc_t* taskFunc;
    ThreadPool_TaskParameter_t* taskParameters;
    uint32_t threadCount;
    volatile CORAL_ATOMIC(uint32_t) busyThreadCount;
    volatile CORAL_ATOMIC(uint32_t) remainingTaskCount;
    volatile CORAL_ATOMIC(uint32_t) currentStatus;
} ThreadPool_t;


/*! \brief Starts \p threadCount threads.
 *
 *  The started threads will sleep until a job is started with \p ThreadPool_startJob().
 *
 *  \returns \c true if thread creation succeeded, otherwise \c false.
 */
CORAL_API bool ThreadPool_init(ThreadPool_t* threadPool, Thread_ot threads[], uint32_t threadCount);


/*! \brief Closes and destructs the threads started by \p threadPool. */
CORAL_API void ThreadPool_destr(ThreadPool_t* threadPool);


/*! \brief Starts the given job for \p threadPool.
 *  \param[in]  taskParameters  (Opt.) If provided, will call \p taskFunc with the contained \p arg and store the
 *                              returned value in \p returnValue; otherwise, will call \p taskFunc with the values
 *                              \c 0U to <tt>taskCount - 1U</tt> and discard the returned values.
 *  \note
 *      If \p ThreadPool_startJob() is called while \p threadPool is still working on a job,
 *      it will have to wait until that job is finished before issuing the next one.
 */
CORAL_API void ThreadPool_startJob(ThreadPool_t* threadPool,
                                   ThreadPool_TaskFunc_t* taskFunc,
                                   ThreadPool_TaskParameter_t taskParameters[],
                                   uint32_t taskCount
);


/*! \brief Waits until \p threadPool has finished its job. */
CORAL_API void ThreadPool_waitOnFinish(ThreadPool_t* threadPool);

/*! @} */

#endif
