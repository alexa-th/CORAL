#include "concurrency/concurrency_implementation.h"
#ifdef CORAL_CONCURRENCY_IMPL_WIN32

#include <stdint.h>
#include <stdbool.h>
#include <windows.h>
#include <synchapi.h>
#include "common/atomic.h"
#include "common/utils.h"
#include "concurrency/signal.h"



// SIGNALS

//- FUNCTIONS

void Signal_send(volatile CORAL_ATOMIC(uint32_t)* signalAddress, bool sendToAll) {
    if (sendToAll) { WakeByAddressAll((PVOID)signalAddress); }
    else { WakeByAddressSingle((PVOID)signalAddress); }
}


void Signal_await(volatile CORAL_ATOMIC(uint32_t)* signalAddress, uint32_t lastValue) {
    CORAL_ASSERT(WaitOnAddress((volatile VOID*)signalAddress, &lastValue, sizeof(uint32_t), INFINITE),
                 "Waiting on address failed."
    );
}

#endif
