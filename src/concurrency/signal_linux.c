#include "concurrency/concurrency_implementation.h"
#ifdef CORAL_CONCURRENCY_IMPL_LINUX
#define _GNU_SOURCE

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <unistd.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include "common/atomic.h"
#include "concurrency/signal.h"



// SIGNALS

//- FUNCTIONS

void Signal_send(volatile CORAL_ATOMIC(uint32_t)* signalAddress, bool sendToAll) {
    syscall(SYS_futex, (uint32_t*)signalAddress, FUTEX_WAKE_PRIVATE, (sendToAll) ? (INT_MAX) : (1));
}


void Signal_await(volatile CORAL_ATOMIC(uint32_t)* signalAddress, uint32_t lastValue) {
    syscall(SYS_futex, (uint32_t*)signalAddress, FUTEX_WAIT_PRIVATE, lastValue, NULL);
}

#endif
