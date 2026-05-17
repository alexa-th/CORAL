#ifndef CORAL_ATOMIC

/*! \file */



// ATOMIC TYPES

#ifdef __cplusplus

extern "C++" {
#include <atomic>
#define CORAL_ATOMIC(T) std::atomic<T>
}

#else
#   if defined(__STDC_NO_ATOMICS__) && !defined(CORAL_CONCURRENCY_ATOMIC_FORCE_INCLUDE)
#       error "Missing required header: <stdatomic.h>"
#   else
#       include <stdatomic.h>
#       define CORAL_ATOMIC(T) _Atomic(T)
#   endif
#endif

#endif
