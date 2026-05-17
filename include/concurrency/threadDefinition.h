#ifndef CORAL_CONCURRENCY_THREAD_DEFINITION
#define CORAL_CONCURRENCY_THREAD_DEFINITION

/*! \file */

#include "../common/targets.h"



// THREAD DEFINITION

/*! \defgroup CORAL_CONCURRENCY_THREAD Thread
 *  \ingroup CORAL_CONCURRENCY
 *  \copybrief Thread
 *  \addtogroup CORAL_CONCURRENCY_THREAD
 *  @{
 */

/*! \struct Thread
 *  \brief Target dependent definition of a thread.
 *  \note Even though \p Thread_ot is to be treated as an opaque type, the underlying <tt>struct Thread</tt> will be defined.
 */
typedef struct Thread Thread_ot;


/*! \typedef ThreadID_t
 *  \brief Integer representation of a thread identifier.
 *  \note The integer type may be signed or unsigned; signage is implementation-specific.
 *  \note A valid thread will always have a non-zero \p ThreadID_t.
 *  \note
 *      Only the current thread's \p ThreadID_t may be compared to another thread's \p ThreadID_t.
 *      If two arbitrary threads' \p ThreadID_t are equal, these threads must not necessarily be the same thread.
 */

/*! @} */



// DEPENCENCY RESOLUTION

#if CORAL_IS_TARGET(CORAL_TARGET_WIN32)
#   include "./threadDefinition_win32.h"
#elif CORAL_IS_TARGET(CORAL_TARGET_LINUX)
#   include "./threadDefinition_linux.h"
#endif

#endif
