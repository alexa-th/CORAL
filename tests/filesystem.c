#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "unity.h"
#include "common/error.h"
#include "filesystem/filesystem.h"



// DEFINITIONS

#define WRITE_BUFFER_SIZE       ((size_t)1024U)
#define WRITE_BUFFER_STR_SIZE   ((size_t)128U)

#define FILE_ERROR_HEX_DIGIT_COUNT  (sizeof(Error_t) * 2U)
#define FILE_ERROR_STRING           ("File error: %#0*X")
#define FILE_ERROR_STRING_SIZE      (sizeof(FILE_ERROR_STRING) + FILE_ERROR_HEX_DIGIT_COUNT)




// GLOBALS

static char writeBuffer[WRITE_BUFFER_SIZE];
static char writeBufferStr[WRITE_BUFFER_STR_SIZE];
static const char alphabet[] = "abcdefghijklmnopqrstuvwxyz1234567890";
static File_t testFile;
static bool openedTestFile = false;



// ERROR HELPER

const char* getErrorMessage(Error_t fileError) {
    static char errorMessage[FILE_ERROR_STRING_SIZE];
    sprintf(errorMessage, FILE_ERROR_STRING, (int)FILE_ERROR_HEX_DIGIT_COUNT, fileError);
    return errorMessage;
}



// TESTS

void test_openFile(void) {
    Error_t fileError = File_init_tempFile(&testFile,
                                           CORAL_FILESYSTEM_FILE_MODE_READ_WRITE_CREATE |
                                           CORAL_FILESYSTEM_FILE_MODE_BINARY
    );

    if (fileError != CORAL_ERROR_NONE) {
        const char* errorMessage = getErrorMessage(fileError);
        TEST_FAIL_MESSAGE(errorMessage);
    }

    openedTestFile = true;
}


void test_writeToFile(void) {
    Error_t fileError;

    for (size_t i = 0U; i < 256U * 1024U; i += WRITE_BUFFER_SIZE) {
        if ((fileError = IO_write(&testFile, &writeBuffer[0U], WRITE_BUFFER_SIZE, NULL)) != CORAL_ERROR_NONE) {
            const char* errorMessage = getErrorMessage(fileError);
            TEST_FAIL_MESSAGE(errorMessage);
        }
    }
}

void test_readCompleteFile(void) {
    Error_t fileError = IO_readComplete(&testFile);

    if (fileError != CORAL_ERROR_NONE) {
        const char* errorMessage = getErrorMessage(fileError);
        TEST_FAIL_MESSAGE(errorMessage);
    }
}


void test_changeFileMode(void) {
    Error_t fileError = File_changeMode(&testFile,
                                        CORAL_FILESYSTEM_FILE_MODE_READ |
                                        CORAL_FILESYSTEM_FILE_MODE_WRITE |
                                        CORAL_FILESYSTEM_FILE_MODE_BINARY
    );

    if (fileError != CORAL_ERROR_NONE) {
        const char* errorMessage = getErrorMessage(fileError);
        TEST_FAIL_MESSAGE(errorMessage);
    }
}



// UNITY

void setUp(void) { }
void tearDown(void) { }



// MAIN

int main(void) {
    memset(&writeBuffer[0U], 'X', WRITE_BUFFER_SIZE);

    for (size_t usedSize = 0U; usedSize + sizeof(alphabet) < WRITE_BUFFER_SIZE; usedSize += sizeof(alphabet) - 1U){
        strncpy(&writeBuffer[usedSize], &alphabet[0], sizeof(alphabet) - 1U);
    }

    UNITY_BEGIN();

    if (TEST_PROTECT()) {
        test_openFile();
        test_writeToFile();
        test_readCompleteFile();
        test_changeFileMode();
    }

    if (openedTestFile) {
        File_destr(&testFile, true);
    }

    return UNITY_END();
}
