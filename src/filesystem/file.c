#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "common/error.h"
#include "common/utils.h"
#include "concurrency/mutex.h"
#include "filesystem/error.h"
#include "filesystem/path.h"
#include "filesystem/file.h"



// FILE

//- DEFINITIONS

#define CORAL_FILESYSTEM_FILE_MODE_MAX_STR_LENGTH sizeof("wb+x")



//- GLOBALS

static Mutex_t g_tmpnamMutex;



//- FUNCTIONS

Error_t File_getModeStr(char (*modeStrBuffer)[CORAL_FILESYSTEM_FILE_MODE_MAX_STR_LENGTH], uint8_t fileMode) {
    // TODO: Log errors

    // These access modes are mutually exclusive
    if (CORAL_FILESYSTEM_FILE_HAS_MODE(fileMode, CORAL_FILESYSTEM_FILE_MODE_WRITE | CORAL_FILESYSTEM_FILE_MODE_APPEND)) {
        return CORAL_ERROR_INVALID_ARGS;
    }

    char* currModeChar = &(*modeStrBuffer)[0U];

    if (CORAL_FILESYSTEM_FILE_HAS_MODE(fileMode, CORAL_FILESYSTEM_FILE_MODE_READ)
        && !CORAL_FILESYSTEM_FILE_HAS_MODE(fileMode, CORAL_FILESYSTEM_FILE_MODE_READ_WRITE_CREATE)) { *currModeChar = 'r'; }
    else if (CORAL_FILESYSTEM_FILE_HAS_MODE(fileMode, CORAL_FILESYSTEM_FILE_MODE_WRITE) ) { *currModeChar = 'w'; }
    else if (CORAL_FILESYSTEM_FILE_HAS_MODE(fileMode, CORAL_FILESYSTEM_FILE_MODE_APPEND)) { *currModeChar = 'a'; }
    else { return CORAL_ERROR_INVALID_ARGS; }

    currModeChar++;

    if (CORAL_FILESYSTEM_FILE_HAS_MODE(fileMode, CORAL_FILESYSTEM_FILE_MODE_BINARY)) {
        *(currModeChar++) = 'b';
    }

    if (CORAL_FILESYSTEM_FILE_HAS_MODE(fileMode, CORAL_FILESYSTEM_FILE_MODE_READ | CORAL_FILESYSTEM_FILE_MODE_WRITE)) {
        *(currModeChar++) = '+';
    }

    if (CORAL_FILESYSTEM_FILE_HAS_MODE(fileMode, CORAL_FILESYSTEM_FILE_MODE_WRITE | CORAL_FILESYSTEM_FILE_MODE_NO_OVERWRITE)) {
        *(currModeChar++) = 'x';
    }

    CORAL_ASSERT((size_t)(currModeChar - &(*modeStrBuffer)[0]) < sizeof(*modeStrBuffer), "modeStrBuffer should be increased.");

    *currModeChar = '\0';
    return CORAL_ERROR_NONE;
}


Error_t File_init(File_t* file, uint8_t fileMode) {
    if (file->filePath.directoryLength == file->filePath.pathStr.used) { return CORAL_ERROR_INVALID_ARGS; }

    Error_t fileError;
    char modeStr[CORAL_FILESYSTEM_FILE_MODE_MAX_STR_LENGTH];
    if ((fileError = File_getModeStr(&modeStr, fileMode)) != CORAL_ERROR_NONE) { return fileError; }

    FILE* filePtr = fopen(file->filePath.pathStr.rawStr.constStr, &modeStr[0]);
    if (!filePtr) { return CORAL_ERROR_UNKOWN; }

    file->filePtr = filePtr;
    file->filePath.pathStr.additionalData[1U] = fileMode;

    return CORAL_ERROR_NONE;
}


Error_t File_init_tempFile(File_t* file, uint8_t fileMode) {
    Mutex_lock(&g_tmpnamMutex);
    const char* tmpFilePathStr = tmpnam(NULL); // tmpnam() is not thread-safe
    Mutex_unlock(&g_tmpnamMutex);

    if (!tmpFilePathStr) { return CORAL_FILESYSTEM_ERROR_TMP_FILE_LIMIT; }

    Path_init(&file->filePath, tmpFilePathStr, 0U, 0U, false, false);
    return File_init(file, fileMode);
}


void File_destr(File_t* file, bool deleteFile) {
    if (file->readBuffer.bufferPtr) { Buffer_destr(&file->readBuffer); }
    if (file->filePath.pathStr.rawStr.constStr) { Path_destr(&file->filePath); }
    if (file->filePtr) { fclose(file->filePtr); }
    if (deleteFile) { CORAL_ASSERT(!remove(file->filePath.pathStr.rawStr.constStr), "Failed to remove file."); }
}


Error_t File_changeMode(File_t* file, uint8_t newFileMode) {
    Error_t fileError;
    char modeStr[CORAL_FILESYSTEM_FILE_MODE_MAX_STR_LENGTH];
    if((fileError = File_getModeStr(&modeStr, newFileMode)) != CORAL_ERROR_NONE) { return fileError; }

    file->filePtr = freopen(file->filePath.pathStr.rawStr.constStr, &modeStr[0U], file->filePtr);

    if (!file->filePtr) { return CORAL_ERROR_UNKOWN; }
    else { file->filePath.pathStr.additionalData[1U] = newFileMode; }

    return CORAL_ERROR_NONE;
}
