#ifndef CORAL_CONCURRENCY_THREAD_DEFINITION_WIN32
#define CORAL_CONCURRENCY_THREAD_DEFINITION_WIN32

/*! \file */

#include <stdint.h>



// THREAD DEFINITION

struct Thread {
    uintptr_t handle;
};


typedef unsigned ThreadID_t;

#endif
