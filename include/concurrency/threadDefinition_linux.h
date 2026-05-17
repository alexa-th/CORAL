#ifndef CORAL_CONCURRENCY_THREAD_DEFINITION_POSIX
#define CORAL_CONCURRENCY_THREAD_DEFINITION_POSIX

/*! \file */

#include <stdint.h>
#include <pthread.h>
#include <sys/types.h>



// THREAD DEFINITION

struct Thread {
    pthread_t pthread;
    pid_t id;
};


typedef pid_t ThreadID_t;

#endif
