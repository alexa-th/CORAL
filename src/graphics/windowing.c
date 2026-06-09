#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "common/error.h"
#include "common/utils.h"
#include "memory/dynalloc.h"
#include "filesystem/path.h"
#include "filesystem/file.h"
#include "filesystem/io.h"
#include "math/vecTypes.h"
#include "graphics/windowing.h"
#include "graphics/windowing_internal.h"



// WINDOWING

//- FRAMEBUFFER

//- - DEFINITIONS

#define CORAL_GRAPHICS_FRAMEBUFFER_BLOCK_SIZE (2U * 1024U)



//- - FUNCTIONS

void Framebuffer__init(Framebuffer_t* framebuffer, size_t width, size_t height) {
    *framebuffer = ((Framebuffer_t){0U, 0U, 0U, 0U, NULL});
    Framebuffer__resize(framebuffer, width, height);
}


void Framebuffer__resize(Framebuffer_t* framebuffer, size_t newWidth, size_t newHeight) {
    if ((!newWidth || !newHeight) && framebuffer->pixelBuffer) {
        CORAL_free(framebuffer->pixelBuffer);
        framebuffer->pixelBuffer = NULL;
    }
    else {
        size_t oldAllocationSize = CORAL_TO_ALIGNMENT(sizeof(Vec4b_u) * framebuffer->width * framebuffer->height,
                                                      CORAL_GRAPHICS_FRAMEBUFFER_BLOCK_SIZE
        );

        size_t newAllocationSize = CORAL_TO_ALIGNMENT(sizeof(Vec4b_u) * newWidth * newHeight,
                                                      CORAL_GRAPHICS_FRAMEBUFFER_BLOCK_SIZE
        );

        if (oldAllocationSize != newAllocationSize) {
            framebuffer->pixelBuffer = CORAL_realloc(framebuffer->pixelBuffer, newAllocationSize);
            CORAL_ASSERT(framebuffer->pixelBuffer, "Failed to allocate memory for a framebuffer.");
        }
    }

    framebuffer->width = newWidth;
    framebuffer->height = newHeight;
}


Error_t Framebuffer_writeToFile(const Framebuffer_t* framebuffer, const Path_t* filePath, bool allowOverwrite) {
    if (!filePath->directoryLength || (filePath->directoryLength == filePath->pathStr.used)) {
        return CORAL_ERROR_INVALID_ARGS;
    }

    if (filePath->extensionLength) {
        const char* fileExtensionStr = filePath->pathStr.rawStr.constStr + filePath->pathStr.used - filePath->extensionLength;
        if (filePath->extensionLength != 4U || strncmp(fileExtensionStr, ".ppm", 4U)) { return CORAL_ERROR_INVALID_ARGS; }
    }


    Error_t fileError;
    File_t outputFile = {*filePath, .readBuffer = {NULL, 0U, 0U}};

    // To prevent deletion of the original filePath's string
    outputFile.filePath.pathStr.isStatic = true;
    outputFile.filePath.pathStr.isConst = true;

    uint8_t fileMode = CORAL_FILESYSTEM_FILE_MODE_WRITE | CORAL_FILESYSTEM_FILE_MODE_NO_REPLICATE_ON_WRITE;
    if (!allowOverwrite) { fileMode |= CORAL_FILESYSTEM_FILE_MODE_NO_OVERWRITE; }

    if ((fileError = File_init(&outputFile, fileMode)) == CORAL_ERROR_UNKOWN) {
        return CORAL_ERROR_UNKOWN;
    }

    CORAL_ASSERT(fileError == CORAL_ERROR_NONE, "File initialization failed due to invalid file mode.");

    fprintf(outputFile.filePtr, "P6 %zu %zu 255 ", framebuffer->width, framebuffer->height);

    fileMode = CORAL_FILESYSTEM_FILE_MODE_APPEND |
               CORAL_FILESYSTEM_FILE_MODE_BINARY |
               CORAL_FILESYSTEM_FILE_MODE_UNBUFFERED |
               CORAL_FILESYSTEM_FILE_MODE_NO_REPLICATE_ON_WRITE
    ;

    if ((fileError = File_changeMode(&outputFile, fileMode)) == CORAL_ERROR_UNKOWN) {
        goto fileFailure;
    }

    CORAL_ASSERT(fileError == CORAL_ERROR_NONE, "File mode change failed due to invalid file mode.");

    static_assert(CORAL_FILESYSTEM_IO_OS_BUFFER_SIZE / sizeof(Vec3b_u),
                  "Filebuffer is smaller than a singular pixel."
    );

    Vec3b_u pixelConversionBuffer[CORAL_FILESYSTEM_IO_OS_BUFFER_SIZE / sizeof(Vec3b_u)];

    const size_t totalPixelCount = framebuffer->width * framebuffer->height;
    size_t totalWrittenPixelCount = 0U;

    do {
        size_t currentWritePixelCount = CORAL_MIN(CORAL_ARRAY_LENGTH(pixelConversionBuffer),
                                                  totalPixelCount - totalWrittenPixelCount
        );

        for (size_t i = 0U; i < currentWritePixelCount; i++) {
            size_t currentPixelIndex = totalWrittenPixelCount + i;
            pixelConversionBuffer[i] = ((Vec3b_u){{framebuffer->pixelBuffer[currentPixelIndex].r,
                                                   framebuffer->pixelBuffer[currentPixelIndex].g,
                                                   framebuffer->pixelBuffer[currentPixelIndex].b
            }});
        }

        fileError = IO_write(&outputFile, &pixelConversionBuffer, sizeof(Vec3b_u) * currentWritePixelCount, NULL);

        if (fileError != CORAL_ERROR_NONE) {
            goto fileFailure;
        }

        totalWrittenPixelCount += currentWritePixelCount;
    } while (totalWrittenPixelCount < totalPixelCount);

    File_destr(&outputFile, false);
    return CORAL_ERROR_NONE;

fileFailure:
    File_destr(&outputFile, true);
    return CORAL_ERROR_UNKOWN;
}
