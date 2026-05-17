#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "common/error.h"
#include "common/utils.h"
#include "memory/dynalloc.h"
#include "containers/buffer.h"
#include "filesystem/error.h"
#include "filesystem/file.h"
#include "filesystem/io.h"



// IO OPERATIONS

//- SYNCHRONOUS

Error_t IO_read(File_t* file, size_t size, size_t* read, bool stopBeforeResize) {
    const size_t initialUsedSize = file->readBuffer.used;
    const size_t readBufferUseLimit = (size) ? (size + initialUsedSize) : (CORAL_FILESYSTEM_FILE_MAX_FILE_SIZE); // Used to implement a complete read
    Error_t fileError = CORAL_ERROR_NONE;

    while(true) {
        if (size && file->readBuffer.used - initialUsedSize == size) { break; }

        size_t bytesToRead = CORAL_MIN(readBufferUseLimit - file->readBuffer.used, CORAL_FILESYSTEM_IO_OS_BUFFER_SIZE);

        // Resizes if bytesToRead is 0U, stopBeforeResize is true and no read has been performed (see fn documentation)
        if (file->readBuffer.capacity < file->readBuffer.used + bytesToRead || !bytesToRead) {
            if (stopBeforeResize && file->readBuffer.used != initialUsedSize) { return CORAL_FILESYSTEM_ERROR_UNFINISHED; }

            fileError = Buffer__reserveCapacity(&file->readBuffer, file->readBuffer.capacity * 2U, CORAL_FILESYSTEM_FILE_MAX_FILE_SIZE, true);
            if (fileError != CORAL_ERROR_NONE) { break; }

            if (!bytesToRead) { continue; } // Needs to be reevaluated
        }

        size_t bytesRead = fread((void*)((uintptr_t)file->readBuffer.bufferPtr + file->readBuffer.used), 1U, bytesToRead, file->filePtr);
        file->readBuffer.used += bytesRead;

        if (bytesRead != bytesToRead || !bytesRead) {
            if (feof(file->filePtr)) { fileError =  CORAL_FILESYSTEM_ERROR_EOF; }
            else { fileError = CORAL_ERROR_OTHER; }
            break;
        }

        if (file->readBuffer.used == CORAL_FILESYSTEM_FILE_MAX_FILE_SIZE) {
            fileError = CORAL_FILESYSTEM_ERROR_TOO_LARGE;
            break;
        }
    }

    if (read) { *read = file->readBuffer.used - initialUsedSize; }
    return fileError;
}


Error_t IO_readComplete(File_t* file) {
    if (!CORAL_FILESYSTEM_FILE_HAS_MODE(file->filePath.pathStr.additionalData[1U], CORAL_FILESYSTEM_FILE_MODE_READ)) { return CORAL_ERROR_INVALID_ARGS; }

    file->readBuffer.used = 0U;

    if (!file->readBuffer.bufferPtr) {
        file->readBuffer.capacity = CORAL_CONTAINERS_BUFFER_CHUNK_SIZE;
        file->readBuffer.bufferPtr = CORAL_malloc(file->readBuffer.capacity);

        if (!file->readBuffer.bufferPtr) { return CORAL_ERROR_OOM; }
    }

    rewind(file->filePtr);
    Error_t fileError = IO_read(file, 0U, NULL, false);

    if (fileError != CORAL_FILESYSTEM_ERROR_EOF) { return fileError; }
    return CORAL_ERROR_NONE;
}


Error_t IO_write(File_t* file, const void* source, size_t size, size_t* written) {
    size_t writtenBytes = 0U;
    Error_t fileError = CORAL_ERROR_NONE;

    if (!CORAL_FILESYSTEM_FILE_HAS_MODE(file->filePath.pathStr.additionalData[1U], CORAL_FILESYSTEM_FILE_MODE_WRITE_BASE | CORAL_FILESYSTEM_FILE_MODE_BINARY)) {
        fileError = CORAL_ERROR_INVALID_ARGS;
        goto end;
    }

    size_t writePosition = ftell(file->filePtr);
    if (writePosition + size > CORAL_FILESYSTEM_FILE_MAX_FILE_SIZE) {
        fileError = CORAL_FILESYSTEM_ERROR_TOO_LARGE;
        goto end;
    }

    bool replicateWrite = file->readBuffer.bufferPtr &&
                          !CORAL_FILESYSTEM_FILE_HAS_MODE(file->filePath.pathStr.additionalData[1U], CORAL_FILESYSTEM_FILE_MODE_NO_REPLICATE_ON_WRITE);

    if (replicateWrite) {
        if ((fileError = Buffer_reserveExactCapacity(&file->readBuffer, writePosition + size, false)) != CORAL_ERROR_NONE) { goto end; }
    }

    // Must call fseek if file is also open in read mode (see "fopen # File access flags")
    if (CORAL_FILESYSTEM_FILE_HAS_MODE(file->filePath.pathStr.additionalData[1U], CORAL_FILESYSTEM_FILE_MODE_READ)) { fseek(file->filePtr, 0, SEEK_CUR); }

    if ((writtenBytes = fwrite(source, 1U, size, file->filePtr)) != size) { fileError = CORAL_ERROR_OTHER; }

    if (replicateWrite && writtenBytes) { memcpy((void*)((uintptr_t)file->readBuffer.bufferPtr + writePosition), source, size); }

end:
    if (written) { *written = writtenBytes; }
    return fileError;
}
