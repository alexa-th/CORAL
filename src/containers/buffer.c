#include <stddef.h>
#include <stdbool.h>
#include "common/error.h"
#include "common/utils.h"
#include "memory/dynalloc.h"
#include "containers/buffer.h"



// BUFFER

Error_t Buffer_init(Buffer_t* buffer, size_t initCapacity) {
    void* bufferPtr = NULL;

    if (initCapacity) {
        if (!(bufferPtr = CORAL_malloc(initCapacity))) { return CORAL_ERROR_OOM; }
    }

    *buffer = ((Buffer_t){bufferPtr, initCapacity, 0U});
    return CORAL_ERROR_NONE;
}


void Buffer_destr(Buffer_t* buffer) {
    if (buffer->bufferPtr) { CORAL_free(buffer->bufferPtr); }
    *buffer = ((Buffer_t){NULL, 0U, 0U});
}


Error_t Buffer_reserveNewBufferChunk(Buffer_t* buffer, bool freeOnOOMFailure) {
    size_t newCapacity = CORAL_TO_ALIGNMENT(buffer->capacity, CORAL_CONTAINERS_BUFFER_CHUNK_SIZE);

    return Buffer_reserveExactCapacity(buffer, newCapacity, freeOnOOMFailure);
}


Error_t Buffer__reserveNewBufferChunk(Buffer_t* buffer, size_t maxCapacity, bool freeOnOOMFailure) {
    size_t newCapacity = CORAL_TO_ALIGNMENT(buffer->capacity, CORAL_CONTAINERS_BUFFER_CHUNK_SIZE);
    if (maxCapacity) { newCapacity = CORAL_MIN(newCapacity, maxCapacity); }

    return Buffer_reserveExactCapacity(buffer, newCapacity, freeOnOOMFailure);
}


Error_t Buffer_reserveCapacity(Buffer_t* buffer, size_t newCapacity, bool freeOnOOMFailure) {
    newCapacity = CORAL_TO_ALIGNMENT(newCapacity, CORAL_CONTAINERS_BUFFER_CHUNK_SIZE);

    return Buffer_reserveExactCapacity(buffer, newCapacity, freeOnOOMFailure);
}


Error_t Buffer__reserveCapacity(Buffer_t* buffer, size_t newCapacity, size_t maxCapacity, bool freeOnOOMFailure) {
    newCapacity = CORAL_TO_ALIGNMENT(newCapacity, CORAL_CONTAINERS_BUFFER_CHUNK_SIZE);
    if (maxCapacity) { newCapacity = CORAL_MIN(newCapacity, maxCapacity); }

    return Buffer_reserveExactCapacity(buffer, newCapacity, freeOnOOMFailure);
}


Error_t Buffer_reserveExactCapacity(Buffer_t* buffer, size_t newExactCapacity, bool freeOnOOMFailure) {
    if (buffer->capacity >= newExactCapacity) { return CORAL_ERROR_NONE; }

    void* newBufferPtr = CORAL_realloc(buffer->bufferPtr, newExactCapacity);

    if (!newBufferPtr) {
        if (freeOnOOMFailure) {
            CORAL_free(buffer->bufferPtr);
            buffer->bufferPtr = NULL;
            buffer->capacity = 0U;
            buffer->used = 0U;
        }

        return CORAL_ERROR_OOM;
    }

    buffer->bufferPtr = newBufferPtr;
    buffer->capacity = newExactCapacity;
    return CORAL_ERROR_NONE;
}


void Buffer_trimToUsedSize(Buffer_t* buffer) {
    if (buffer->capacity == buffer->used) { return; }

    if ((buffer->capacity = buffer->used)) {
        buffer->bufferPtr = CORAL_realloc(buffer->bufferPtr, buffer->capacity);
        CORAL_ASSERT(buffer->bufferPtr, "Realloc failed.");
    }
    else {
        CORAL_free(buffer->bufferPtr);
        buffer->bufferPtr = NULL;
    }
}
