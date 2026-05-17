#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <tgmath.h>
#include <assert.h>
#include "concurrency/signal.h"
#include "concurrency/mutex.h"
#include "concurrency/thread.h"
#include "common/random.h"



// PSEUDO-RANDOM NUMBER GENERATION

//- ASSERTIONS

static_assert(INTMAX_MAX > INT_MAX,
              "Cannot convert from int to intmax_t."
);

static_assert(INTMAX_MAX >= UINT_MAX,
              "Cannot convert from unsigned to intmax_t."
);

static_assert(UINTMAX_MAX >= (uintmax_t)(-(intmax_t)INT_MIN),
              "Cannot convert from int to uintmax_t."
);

static_assert(UINTMAX_MAX > UINT_MAX,
              "Cannot convert from unsigned to uintmax_t."
);

static_assert((intmax_t)UINT_MAX + (intmax_t)INT_MIN == INT_MAX,
              "Cannot convert from unsigned to int."
);



//- DEFINITIONS

#define CORAL_RANDOM_STATUS_UNINITIALIZED   (0U)
#define CORAL_RANDOM_STATUS_INITIALIZING    (1U)
#define CORAL_RANDOM_STATUS_INITIALIZED     (2U)

#define CORAL_RANDOM_NUMBER_BUFFER_COUNT (64U)



//- GLOBALS

static Mutex_t g_randMutex;
static size_t g_randomizedBitsPerCall;
static bool g_isInitialized;

static CORAL_THREAD_LOCAL unsigned g_randomNumberBuffer[CORAL_RANDOM_NUMBER_BUFFER_COUNT];
static CORAL_THREAD_LOCAL size_t g_currentRandomNumberBufferIndex = 0U;



//- FUNCTIONS

int Random_int(void) {
    return (intmax_t)Random_unsigned() + (intmax_t)INT_MIN;
}


int Random__int(int min, int max) {
    uintmax_t rangeLength = 1U + (uintmax_t)max;

    if (min < 0) { rangeLength += -(intmax_t)min; }
    else { rangeLength -= (uintmax_t)min; }

    return (intmax_t)(Random_unsigned() % rangeLength) + (intmax_t)min;
}


unsigned Random_unsigned(void) {
    if (!(g_currentRandomNumberBufferIndex %= CORAL_RANDOM_NUMBER_BUFFER_COUNT)) {
        Mutex_lock(&g_randMutex);

        if (!g_isInitialized) {
            srand((unsigned)time(NULL));
            g_randomizedBitsPerCall = (size_t)log2(RAND_MAX);
            g_isInitialized = true;
        }

        for (size_t i = 0U; i < CORAL_RANDOM_NUMBER_BUFFER_COUNT; i++) {
            unsigned randBitMask = (1U << g_randomizedBitsPerCall) - 1U;
            unsigned newRandomNumber = 0U;

            for (size_t randomizedBits = 0U;
                randomizedBits < sizeof(unsigned) * CHAR_BIT;
                randomizedBits += g_randomizedBitsPerCall
            ) {
                newRandomNumber += ((unsigned)rand() & randBitMask) << randomizedBits;
            }

            g_randomNumberBuffer[i] = newRandomNumber;
        }

        Mutex_unlock(&g_randMutex);
    }

    return g_randomNumberBuffer[g_currentRandomNumberBufferIndex++];
}


unsigned Random__unsigned(unsigned min, unsigned max) {
    uintmax_t rangeLength = (uintmax_t)max - (uintmax_t)min + 1U;

    return (Random_unsigned() % rangeLength) + min;
}


float Random_float(void) {
    return ((long double)Random_unsigned() - (long double)UINT_MAX / 2.0l) *
           (long double)FLT_MAX / ((long double)UINT_MAX / 2.0l)
    ;
}


float Random__float(float min, float max) {
    long double rangeLength = (long double)max - (long double)min;

    return ((long double)Random_unsigned() - (long double)UINT_MAX / 2.0l) *
           rangeLength / (long double)UINT_MAX +
           ((long double)min + rangeLength / 2.0l)
    ;
}


double Random_double(void) {
    return ((long double)Random_unsigned() - (long double)UINT_MAX / 2.0l) *
           (long double)DBL_MAX / ((long double)UINT_MAX / 2.0l)
    ;
}


double Random__double(double min, double max) {
    long double rangeLength = (long double)max - (long double)min;

    return ((long double)Random_unsigned() - (long double)UINT_MAX / 2.0l) *
           rangeLength / (long double)UINT_MAX +
           ((long double)min + rangeLength / 2.0l)
    ;
}
