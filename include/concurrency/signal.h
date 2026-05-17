#ifndef CORAL_CONCURRENCY_SIGNAL
#define CORAL_CONCURRENCY_SIGNAL

/*! \file */

#include <stdint.h>
#include <stdbool.h>
#include "../common/api.h"
#include "../common/atomic.h"

/*! \defgroup CORAL_CONCURRENCY_SIGNAL Signals
 *  \ingroup CORAL_CONCURRENCY
 *  \brief Signals can be used to communicate between threads.
 *  \addtogroup CORAL_CONCURRENCY_SIGNAL
 *  @{
 */



// SIGNALS

//- FUNCTIONS

/*! \brief Wakes threads currently awaiting a signal from \p signalAddress.
 *  \param[in]  sendToAll   If \c true, will wake all threads currently awaiting a signal from \p signalAddress;
 *                          otherwise, will wake one thread awaiting a signal from \p signalAddress.
 *
 *  \note
 *      If \p sendToAll is \c false and multiple threads are awaiting a signal from \p signalAddress, it can
 *      can not be determined which of those threads will wake.
 */
CORAL_API void Signal_send(volatile CORAL_ATOMIC(uint32_t)* signalAddress, bool sendToAll);


/*! \brief Will await a signal from \p signalAddress if the value at \p signalAddress is equal to \p lastValue. */
CORAL_API void Signal_await(volatile CORAL_ATOMIC(uint32_t)* signalAddress, uint32_t lastValue);

/*! @} */

#endif
