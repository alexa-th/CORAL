#ifndef CORAL_CONCURRENCY_MUTEX
#define CORAL_CONCURRENCY_MUTEX

/*! \file */

#include <stdint.h>
#include <stdbool.h>
#include "../common/api.h"
#include "../common/atomic.h"
#include "./threadDefinition.h"

/*! \defgroup CORAL_CONCURRENCY_MUTEX Mutexes
 *  \ingroup CORAL_CONCURRENCY
 *  \brief Mutexes can be used to synchronize access to common resources.
 *  \addtogroup CORAL_CONCURRENCY_MUTEX
 *  @{
 */



// MUTEXES

//- DEFINITIONS

/*! \brief Can be used by multiple threads to mutually exclude (->mutex) each other from accessing a common resource. */
typedef struct {

    /*! \brief The thread currently \"owning\" the common resource. */
    ThreadID_t owner;

    /*! \brief The amount of times the owning thread has locked the mutex.
     *
     *  Used in order to prevent a thread from softlocking itself and to prevent a call to
     *  \p Mutex_unlock() from unlocking a mutex that is still required to be locked further up the call stack.
     */
    volatile CORAL_ATOMIC(uint32_t) lockCount;
} Mutex_t;


/*! \brief A mutex that can both be locked by multiple threads collectively and by one singular thread exclusively.
 *
 *  Used if multiple threads should be able read from a common resource simultaneously but if a thread needs
 *  to write to said resource, it must be able to ensure that it owns the resource exclusively.
 */
typedef struct {

    /*! \brief Mutex to be used when a thread needs to own the common resource exclusively. */
    Mutex_t singleOwnerMutex;

    /*! \brief Amount of threads that collectively own the common resource.
     *
     *  Additionally used like \p Mutex_t::relockCount to prevent softlocks.
     */
    volatile CORAL_ATOMIC(uint32_t) ownerCount;
} MultiOwnedMutex_t;



//- FUNCTIONS

/*! \name Singly-owned Mutex
 *  @{
 */

/*! \brief Initializes the <em>dynamically allocated</em> \p mutex. */
CORAL_API void Mutex_init(Mutex_t* mutex);


/*! \brief Tries to lock \p mutex.
 *  \returns \c true if \p mutex is now locked, \c false if not.
 */
CORAL_API bool Mutex_tryLock(Mutex_t* mutex);


/*! \note Blocks the calling thread until \p mutex can be locked. */
CORAL_API void Mutex_lock(Mutex_t* mutex);


/*! \note \p mutex must have been locked by the calling thread prior to calling \p Mutex_unlock(). */
CORAL_API void Mutex_unlock(Mutex_t* mutex);

/*! @} */



/*! \name Multi-owned Mutex
 *  @{
 */

/*! \brief Initializes the <em>dynically allocated</em> \p multiMutex. */
CORAL_API void MultiOwnedMutex_init(MultiOwnedMutex_t* multiMutex);


/*! \brief Tries to atomically increase \mlink{MultiOwnedMutex_t,multiMutex->,ownerCount} by \c 1U.
 *  \returns \c true if the increase succeeded, \c false if not.
 *  \remarks
 *      Increasing \mlink{MultiOwnedMutex_t,multiMutex->,ownerCount} will only fail
 *      if \p multiMutex was exclusively locked before or during the operation.
 */
CORAL_API bool MultiOwnedMutex_tryLock(MultiOwnedMutex_t* multiMutex);


/*! \brief Atomically increases \mlink{MultiOwnedMutex_t,multiMutex->,ownerCount} by \c 1U. */
CORAL_API void MultiOwnedMutex_lock(MultiOwnedMutex_t* multiMutex);


/*! \brief Atomically decreases \mlink{MultiOwnedMutex_t,multiMutex->,ownerCount} by \c 1U.
 *  \note
 *      \mlink{MultiOwnedMutex_t,multiMutex->,ownerCount} must not be \c 0U
 *      and \p multiMutex must not be exclusively locked.
 */
CORAL_API void MultiOwnedMutex_unlock(MultiOwnedMutex_t* multiMutex);


/*! \brief Tries to exclusively lock \p multiMutex.
 *  \returns \c true if \p multiMutex is now exclusively locked, \c false if not.
 *  \remarks Behaves similar to \p Mutex_tryLock().
 */
CORAL_API bool MultiOwnedMutex_tryLock_single(MultiOwnedMutex_t* multiMutex);


/*! \remarks Behaves similar to \p Mutex_lock(). */
CORAL_API void MultiOwnedMutex_lock_single(MultiOwnedMutex_t* multiMutex);


/*! \param[in]  multiMutex  Must already be collectively locked by the calling thread.
 *  \copydoc MultiOwnedMutex_lock_single
 *  \warning
 *      No guarantee can be made that \p multiMutex was not locked by another thread and
 *      that the common resource guarded by \p multiMutex was not written to.
 *
 *  \warning
 *      If \p multiMutex has been collectively locked multiple times by the calling thread,
 *      a call to \p MultiOwnedMutex_lock__single() will block indefinetely.
 */
CORAL_API void MultiOwnedMutex_lock__single(MultiOwnedMutex_t* multiMutex);


/*! \remarks Behaves similar to \p Mutex_unlock(). */
CORAL_API void MultiOwnedMutex_unlock_single(MultiOwnedMutex_t* multiMutex);


/*! \brief Unlocks \p multiMutex such that the calling thread retains a collective lock.
 *  \returns
 *      \c true if \p multiMutex is no longer locked exclusively, otherwise \c false, which may occur
 *      if \mlink{Mutex_t,multiMutex->singleOwnerMutex.,lockCount} is greater than \c 1U.
 *  \copydoc MultiOwnedMutex_unlock_single
 */
CORAL_API bool MultiOwnedMutex_unlock__single(MultiOwnedMutex_t* multiMutex);

/*! @} */

/*! @} */

#endif
