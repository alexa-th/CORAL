#ifndef CORAL_CONCURRENCY_THREAD
#define CORAL_CONCURRENCY_THREAD

/*! \file */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "../common/api.h"
#include "../common/compilers.h"
#include "../common/noreturn.h"
#include "./threadDefinition.h"



// THREAD

/*! \addtogroup CORAL_CONCURRENCY_THREAD
 *  @{
 */



//- DEFINITIONS

/*! \def CORAL_THREAD_LOCAL
 *  \brief Variables declared with \p CORAL_THREAD_LOCAL have \e thread storage duration.
 *
 *  Each thread will have its own copy of a variable declared with \p CORAL_THREAD_LOCAL residing in thread-local storage.
 *  This copy will be initialized upon thread entry and will exist until thread exit.
 *
 *  \warning Accessing another thread's thread-local variables is implementation-defined behaviour and therefore heavily discouraged.
 */

#if CORAL_IS_COMPILER(CORAL_COMPILER_GCC) || CORAL_IS_COMPILER(CORAL_COMPILER_CLANG)
#   define CORAL_THREAD_LOCAL __thread
#elif CORAL_IS_COMPILER(CORAL_COMPILER_MSVC)
#   define CORAL_THREAD_LOCAL __declspec(thread)
#else
#   define CORAL_THREAD_LOCAL
#endif


/*! \brief A function that will be called as a created thread's \c main function.
 *  \param[in]  mainArg     Argument provided by the thread that called \p Thread_start().
 */
typedef void (Thread_MainFunc_t)(void* mainArg);



//- FUNCTIONS

/*! \returns The integer representation of the thread identifier of the calling thread. */
CORAL_API ThreadID_t Thread_getCurrentThread(void);


/*! \returns The integer representation of \p thread. */
CORAL_API ThreadID_t Thread_getID(const Thread_ot* thread);


/*! \brief Starts a new thread.
 *
 *  After initializing, the created thread will call \p mainFunc with \p startArg.
 *
 *  \param[out] thread  Will contain the thread identifier of the created thread.
 *  \returns The integer representation of the thread identifier of the created thread or \c 0 if thread creation failed.
 */
CORAL_API ThreadID_t Thread_start(Thread_ot* thread, Thread_MainFunc_t* mainFunc, void* mainArg);


/*! \brief Terminates the calling thread.
 *  \warning
 *      To be called only from within a thread created by \p Thread_start().
 *      As \p Thread_exit() does not ensure the correct shutdown of all submodules,
 *      terminating the main thread with \p Thread_exit() will lead to major errors.
 */
CORAL_API CORAL_NORETURN void Thread_exit(void);


/*! \brief Waits for \p thread to exit. */
CORAL_API void Thread_join(const Thread_ot* thread);


/*! \brief Waits for the all threads in \p threads to exit. */
CORAL_API void Thread__join(const Thread_ot threads[], size_t threadCount);

/*! @} */

#endif
