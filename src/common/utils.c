#include <stdio.h>
#include <stdlib.h>
#include "common/noreturn.h"
#include "common/utils.h"



// UTILS

//- ASSERT

CORAL_NORETURN void coral_assert_base(const char* message, const char* file, unsigned line) {
    fprintf(stderr, "Assertion failed at line %u in \"%s\":\n\t%s\n", line, file, message);
    abort();
}
