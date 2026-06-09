#include <string.h>
#include "common/error.h"
#include "common/utils.h"
#include "containers/string.h"
#include "filesystem/path.h"



// PATH

//- DEFINITIONS

#define CORAL_FILESYSTEM_PATH_SEPERATOR_WINDOWS ('\\')
#define CORAL_FILESYSTEM_PATH_SEPERATOR_UNIX    ('/')



//- FUNCTIONS

static Error_t Path_parse(Path_t* path) {
    PathType_e pathType = 0;
    size_t extensionLength = 0U;

    const char* const startCharacter = path->pathStr.rawStr.constStr + path->pathStr.used;  // startCharacter is actually the end of the string
    const char* const endCharacter = path->pathStr.rawStr.constStr;                         // endCharacter is actually the start of the string
    const char* currentCharacter = startCharacter;

    for (; currentCharacter >= endCharacter; currentCharacter--) {
        switch (*currentCharacter) {
            case '.':
                if (!extensionLength) { extensionLength = startCharacter - currentCharacter; }
                break;

            case CORAL_FILESYSTEM_PATH_SEPERATOR_WINDOWS:
                pathType = CORAL_FILESYSTEM_PATH_TYPE_WINDOWS;
                goto end;

            case CORAL_FILESYSTEM_PATH_SEPERATOR_UNIX:
                pathType = CORAL_FILESYSTEM_PATH_TYPE_UNIX;
                goto end;

        }
    }

end:
    if (!pathType) { return CORAL_ERROR_INVALID_ARGS; }

    path->extensionLength = extensionLength;
    path->directoryLength = currentCharacter - endCharacter + 1U;
    path->pathStr.additionalData[0U] = pathType;

    return CORAL_ERROR_NONE;
}


Error_t Path_init(Path_t* path, const char* rawPathStr, size_t rawPathStrUsed, size_t rawPathStrCapacity, bool makeDynamic, bool isConst) {
    if (!rawPathStrUsed) { rawPathStrUsed = strlen(rawPathStr); }

    if (!rawPathStrCapacity) { rawPathStrCapacity = rawPathStrUsed + 1U; }
    else if (rawPathStrCapacity <= rawPathStrUsed) { return CORAL_ERROR_INVALID_ARGS; } 

    if (rawPathStrUsed + 1U > CORAL_FILESYSTEM_PATH_MAX_LENGTH) { return CORAL_ERROR_INVALID_ARGS; }

    if (makeDynamic) {
        if (String_init(&path->pathStr, rawPathStrCapacity) == CORAL_ERROR_OOM) { return CORAL_ERROR_OOM; }

        memcpy(path->pathStr.rawStr.mutableStr, rawPathStr, rawPathStrUsed + 1U);
    }
    else {
        Error_t error = String_init_static(&path->pathStr, rawPathStr, rawPathStrUsed, rawPathStrCapacity, isConst);
        CORAL_ASSERT(error == CORAL_ERROR_NONE, "Unexpected static string initialization error.");
    }

    return Path_parse(path);
}


Error_t Path_init_relative(Path_t* path, Path_t* baseDirectory, const char* rawPathStr, size_t rawPathStrUsed, size_t rawPathStrCapacity, bool makeDynamic) {
    if (baseDirectory->directoryLength != baseDirectory->pathStr.used) { return CORAL_ERROR_INVALID_ARGS; }

    if (!rawPathStrUsed) { rawPathStrUsed = strlen(rawPathStr); }

    if (!rawPathStrCapacity) { rawPathStrCapacity = rawPathStrUsed + 1U; }
    else if (rawPathStrCapacity <= rawPathStrUsed) { return CORAL_ERROR_INVALID_ARGS; }

    size_t absolutePathStrCapacity = rawPathStrUsed + baseDirectory->pathStr.used + 1U;

    if (absolutePathStrCapacity > CORAL_FILESYSTEM_PATH_MAX_LENGTH) { return CORAL_ERROR_INVALID_ARGS; }

    if (makeDynamic) {
        if (String_init(&path->pathStr, absolutePathStrCapacity) == CORAL_ERROR_OOM) { return CORAL_ERROR_OOM; }

        memcpy(path->pathStr.rawStr.mutableStr, baseDirectory->pathStr.rawStr.constStr, baseDirectory->pathStr.used);
        memcpy(path->pathStr.rawStr.mutableStr + path->pathStr.used, rawPathStr, rawPathStrUsed + 1U);
    }
    else {
        if (absolutePathStrCapacity > rawPathStrCapacity) { return CORAL_ERROR_OOM; }

        Error_t error = String_init_static(&path->pathStr, rawPathStr, rawPathStrUsed, rawPathStrCapacity, false);
        CORAL_ASSERT(error == CORAL_ERROR_NONE, "Unexpected static string initialization error.");

        error = String__concat(&path->pathStr, &baseDirectory->pathStr, 0U, 0U, baseDirectory->pathStr.used);
        CORAL_ASSERT(error == CORAL_ERROR_NONE, "Unexpected string concatination error.");
    }

    return Path_parse(path);
}


void Path_destr(Path_t* path) {
    String_destr(&path->pathStr);
}
