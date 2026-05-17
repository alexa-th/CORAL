#include <stdbool.h>
#include <string.h>
#include "common/error.h"
#include "memory/dynalloc.h"
#include "containers/string.h"



// STRINGS

//- DEFINITIONS

#define CORAL_CONTAINERS_STRING_INIT_DYNAMIC_CAPACITY           (16U)
#define CORAL_CONTAINERS_STRING_DYNAMIC_CAPACITY_GROWTH_FACTOR  (2U)



//- FUNCTIONS

//- - HELPERS

static Error_t String_verifyArgs(const String_t* restrict destString, const String_t* restrict srcString, size_t destStart, size_t srcStart, size_t minDestCapacity) {
    if ((destStart > destString->used) || (srcStart > srcString->used) || (!srcString->used) || (destString->isConst)) { return CORAL_ERROR_INVALID_ARGS; }
    if (destString->isStatic && destString->capacity < minDestCapacity) { return CORAL_ERROR_OOM; }

    return CORAL_ERROR_NONE;
}


static Error_t String_reserveDynamicCapacity(String_t* string, size_t newMinCapacity) {
    size_t newCapacity = (string->capacity) ? (string->capacity) : (CORAL_CONTAINERS_STRING_INIT_DYNAMIC_CAPACITY);
    while (newCapacity < newMinCapacity) { newCapacity *= CORAL_CONTAINERS_STRING_DYNAMIC_CAPACITY_GROWTH_FACTOR; }

    char* newRawStr = CORAL_realloc(string->rawStr.mutableStr, newCapacity);
    if (!newRawStr) { return CORAL_ERROR_OOM; }

    string->rawStr.mutableStr = newRawStr;
    string->capacity = newCapacity;

    return CORAL_ERROR_NONE;
}



//- <

Error_t String_init(String_t* string, size_t initCapacity) {
    char* rawStr = NULL;

    if (initCapacity) {
        if (!(rawStr = CORAL_malloc(initCapacity))) { return CORAL_ERROR_OOM; }
    }

    *string = ((String_t){{.mutableStr = rawStr}, 0U, initCapacity, false, false});
    return CORAL_ERROR_NONE;
}


Error_t String_init_static(String_t* string, const char* rawStr, size_t used, size_t capacity, bool isConst) {
    if (!used) { used = strlen(rawStr); }

    if (!capacity) { capacity = used + 1U; }
    else if (capacity <= used) { return CORAL_ERROR_INVALID_ARGS; }

    *string = ((String_t){{.constStr = rawStr}, used, capacity, true, isConst});
    return CORAL_ERROR_NONE;
}


void String_destr(String_t* string) {
    if (!string->isStatic && string->rawStr.mutableStr) { CORAL_free(string->rawStr.mutableStr); }
}


Error_t String_copy(String_t* restrict destString, const String_t* restrict srcString) {
    return String__copy(destString, srcString, 0U, 0U, srcString->used);
}


Error_t String__copy(String_t* restrict destString, const String_t* restrict srcString, size_t destStart, size_t srcStart, size_t count) {
    Error_t error;
    const size_t newUsed = destStart + count;

    if ((error = String_verifyArgs(destString, srcString, destStart, srcStart, newUsed + 1U))) { return error; }

    if (!destString->isStatic && destString->capacity <= newUsed) {
        if ((error = String_reserveDynamicCapacity(destString, newUsed + 1U))) { return error; }
    }

    if (count) { memcpy(destString->rawStr.mutableStr + destStart, srcString->rawStr.constStr + srcStart, count); }

    destString->rawStr.mutableStr[newUsed] = '\0';
    destString->used = newUsed;

    return CORAL_ERROR_NONE;
}


Error_t String_concat(String_t* restrict destString, const String_t* restrict srcString) {
    return String__concat(destString, srcString, destString->used, 0U, srcString->used);
}


Error_t String__concat(String_t* restrict destString, const String_t* restrict srcString, size_t destStart, size_t srcStart, size_t count) {
    Error_t error;
    const size_t newUsed = destString->used + count;

    if ((error = String_verifyArgs(destString, srcString, destStart, srcStart, newUsed + 1U))) { return error; }

    if (!destString->isStatic && destString->capacity <= newUsed) {
        if ((error = String_reserveDynamicCapacity(destString, newUsed + 1U))) { return error; }
    }

    if (destString->used) {
        size_t shiftCount = destString->used - destStart;
        if (shiftCount) { memcpy(destString->rawStr.mutableStr + destStart + count, destString->rawStr.mutableStr + destStart, shiftCount); }
    }

    if (count) { memcpy(destString->rawStr.mutableStr + destStart, srcString->rawStr.constStr + srcStart, count); }

    destString->rawStr.mutableStr[newUsed] = '\0';
    destString->used = newUsed;

    return CORAL_ERROR_NONE;
}
