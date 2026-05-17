#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "unity.h"
#include "common/utils.h"
#include "containers/containers.h"



// DEFINITIONS

#define CAPACITY_COUNT (32U)

typedef struct {
    union {
        LinkedList_SingleLink_t singleLink;
        LinkedList_DoubleLink_t doubleLink;
    };
    size_t data;
} Test_LinkContainer_t;


// GLOBALS

static size_t initValues[CAPACITY_COUNT];
static Test_LinkContainer_t linkContainers[CAPACITY_COUNT];



// TESTS

//- STRING

#define DYNAMIC_STRING_SOURCE_REPITION_COUNT (16U)


void test_string_static(void) {
    const char* rawSourceStr = "Hello,";
    const char* rawConstStr = " World!";
    char rawMutableStr[16U] = {'\0'};
    size_t rawSourceStrLength = strlen(rawSourceStr);

    String_t stringA;
    String_t stringB;

    {
        if (String_init_static(&stringA, &rawMutableStr[0U], 0U, sizeof(rawMutableStr), false)) { goto fail_init; }
        if (String_init_static(&stringB, rawConstStr, 0U, 0U, true)) { goto fail_init; }
    }

    {
        if (String_copy(&stringA, &stringB)) { goto fail_copy; }
        if (strcmp(&rawConstStr[0U], stringA.rawStr.mutableStr)) { goto fail_copy_res; }
    }

    {
        memcpy(stringA.rawStr.mutableStr, rawSourceStr, rawSourceStrLength + 1U);
        stringA.used = rawSourceStrLength;

        if (String__copy(&stringA, &stringB, 4U, 0U, stringB.used)) { goto fail_copy; }
        if (strcmp("Hell World!", stringA.rawStr.mutableStr)) { goto fail_copy_res; }
    }

    {
        memcpy(stringA.rawStr.mutableStr, rawSourceStr, rawSourceStrLength + 1U);
        stringA.used = rawSourceStrLength;

        if (String_concat(&stringA, &stringB)) { goto fail_concat; }
        if (strcmp("Hello, World!", stringA.rawStr.mutableStr)) { goto fail_concat_res; }
    }

    {
        memcpy(stringA.rawStr.mutableStr, rawSourceStr, rawSourceStrLength + 1U);
        stringA.used = rawSourceStrLength;

        if (String__concat(&stringA, &stringB, 4U, 0U, stringB.used)) { goto fail_concat; }
        if (strcmp("Hell World!o,", stringA.rawStr.mutableStr)) { goto fail_concat_res; }
    }

    return;

fail_init:
        TEST_FAIL_MESSAGE("String initialization failed.");

fail_copy:
        TEST_FAIL_MESSAGE("String copying failed.");

fail_copy_res:
        TEST_FAIL_MESSAGE("String copying produced a wrong result.");

fail_concat:
        TEST_FAIL_MESSAGE("String concatination failed.");

fail_concat_res:
        TEST_FAIL_MESSAGE("String concatination produced a wrong result.");
}


void test_string_dynamic(void) {
    const char* rawSourceStr = "Hello, World!";
    const size_t rawSourceStrLength = strlen(rawSourceStr);

    String_t dynamicString;
    String_t staticString;

    Error_t error = CORAL_ERROR_NONE;

    {
        if (String_init_static(&staticString, rawSourceStr, rawSourceStrLength, rawSourceStrLength + 1U, true)) { TEST_FAIL_MESSAGE("Failed to initialize static string."); }

        if ((error = String_init(&dynamicString, rawSourceStrLength + 1U))) { goto fail; }
    }

    {
        for (size_t i = 0U; i < DYNAMIC_STRING_SOURCE_REPITION_COUNT; i++) {
            if ((error = String_concat(&dynamicString, &staticString))) { goto fail; }
        }

        for (size_t i = 0U; i < DYNAMIC_STRING_SOURCE_REPITION_COUNT; i++) {
            if (strncmp(dynamicString.rawStr.mutableStr + (rawSourceStrLength) * i, staticString.rawStr.constStr, rawSourceStrLength)) {
                TEST_MESSAGE(dynamicString.rawStr.mutableStr);
                TEST_FAIL_MESSAGE("Failed to concatenate strings.");
            }
        }
    }


fail:
    switch(error) {
        case CORAL_ERROR_NONE:
            TEST_PASS();

        case CORAL_ERROR_INVALID_ARGS:
            TEST_FAIL_MESSAGE("Passed incorrect arguments to string function.");

        case CORAL_ERROR_OOM:
            TEST_FAIL_MESSAGE("Failed to allocate space for dynamic string.");
    }
}



//- BUFFER

void test_buffer(void) {
    Buffer_t buffer;
    Error_t bufferError;

    {
        bufferError = Buffer_init(&buffer, CAPACITY_COUNT);

        TEST_ASSERT_EQUAL_MESSAGE(CORAL_ERROR_NONE, bufferError, "Error initializing the buffer.");
        TEST_ASSERT_EQUAL(CAPACITY_COUNT, buffer.capacity);
    }

    {
        bufferError = Buffer_reserveExactCapacity(&buffer, CAPACITY_COUNT * 2U, true);

        TEST_ASSERT_EQUAL(CORAL_ERROR_NONE, bufferError);
        TEST_ASSERT_EQUAL(CAPACITY_COUNT * 2U, buffer.capacity);
    }

    {
        bufferError = Buffer_reserveCapacity(&buffer, buffer.capacity, true);

        TEST_ASSERT_EQUAL(CORAL_ERROR_NONE, bufferError);
        TEST_ASSERT(!(buffer.capacity % CORAL_CONTAINERS_BUFFER_CHUNK_SIZE));
    }

    {
        bufferError = Buffer_reserveNewBufferChunk(&buffer, true);

        TEST_ASSERT_EQUAL(CORAL_ERROR_NONE, bufferError);
        TEST_ASSERT(!(buffer.capacity % CORAL_CONTAINERS_BUFFER_CHUNK_SIZE));
    }

    {
        Buffer_trimToUsedSize(&buffer);

        TEST_ASSERT(!buffer.bufferPtr);
        TEST_ASSERT(!buffer.capacity);
        TEST_ASSERT(!buffer.used);
    }
}



//- LINKED LIST
// TODO: Test the LinkedList_insert functions

#define LINKED_LIST_PREMATURE_END_STRING    ("Linked list ended prematurely.")
#define LINKED_LIST_CLEAR_ERROR_STRING      ("Linked list was not cleared correctly.")


void test_linkedList_single(void) {
    LinkedList_t list = {NULL, NULL};
    LinkedList_SingleLink_t* link;
    size_t i;

    {
        for (i = 0U; i < CAPACITY_COUNT; i++) { LinkedList_append_single(&list, &linkContainers[i].singleLink); }

        for (i = 0U, link = list.headLink.singleLink; i < CAPACITY_COUNT; i++, link = link->next) {
            TEST_ASSERT_MESSAGE(link, LINKED_LIST_PREMATURE_END_STRING);
            TEST_ASSERT_EQUAL(initValues[i], CORAL_TO_PARENT_PTR(Test_LinkContainer_t, singleLink, link)->data);
        }
    }

    {
        for (i = 0U; i < CAPACITY_COUNT; i++) { LinkedList_remove_single(&list, list.headLink.singleLink); }

        TEST_ASSERT_MESSAGE(!list.headLink.singleLink && !list.tailLink.singleLink, LINKED_LIST_CLEAR_ERROR_STRING);
    }

    {
        for (i = 0U; i < CAPACITY_COUNT; i++) { LinkedList_push_single(&list, &linkContainers[i].singleLink); }

        for (i = 0U, link = list.tailLink.singleLink; i < CAPACITY_COUNT; i++, link = LinkedList_findPrevious_single(list.headLink.singleLink, link)) {
            TEST_ASSERT_MESSAGE(link, LINKED_LIST_PREMATURE_END_STRING);
            TEST_ASSERT_EQUAL(initValues[i], CORAL_TO_PARENT_PTR(Test_LinkContainer_t, singleLink, link)->data);
        }

        list.headLink.singleLink = NULL;
        list.tailLink.singleLink = NULL;
    }
}


void test_linkedList_double(void) {
    LinkedList_t list = {NULL, NULL};
    LinkedList_DoubleLink_t* link;
    size_t i;

    {
        for (i = 0U; i < CAPACITY_COUNT; i++) { LinkedList_append_double(&list, &linkContainers[i].doubleLink); }

        for (i = 0U, link = list.headLink.doubleLink; i < CAPACITY_COUNT; i++, link = link->next) {
            TEST_ASSERT_MESSAGE(link, LINKED_LIST_PREMATURE_END_STRING);
            TEST_ASSERT_EQUAL(initValues[i], CORAL_TO_PARENT_PTR(Test_LinkContainer_t, doubleLink, link)->data);
        }
    }

    {
        for (i = 0U; i < CAPACITY_COUNT; i++) { LinkedList_remove_double(&list, list.headLink.doubleLink); }

        TEST_ASSERT_MESSAGE(!list.headLink.doubleLink && !list.tailLink.doubleLink, LINKED_LIST_CLEAR_ERROR_STRING);
    }

    {
        for (i = 0U; i < CAPACITY_COUNT; i++) { LinkedList_push_double(&list, &linkContainers[i].doubleLink); }

        for (i = 0U, link = list.tailLink.doubleLink; i < CAPACITY_COUNT; i++, link = link->previous) {
            TEST_ASSERT_MESSAGE(link, LINKED_LIST_PREMATURE_END_STRING);
            TEST_ASSERT_EQUAL(initValues[i], CORAL_TO_PARENT_PTR(Test_LinkContainer_t, doubleLink, link)->data);
        }

        list.headLink.doubleLink = NULL;
        list.tailLink.doubleLink = NULL;
    }
}



// UNITY

void setUp(void) {}
void tearDown(void) {}



// MAIN

int main(void) {
    for (size_t i = 0U; i < CAPACITY_COUNT; i++) {
        initValues[i] = i * i;
        linkContainers[i].data = initValues[i];
    }

    UNITY_BEGIN();

    RUN_TEST(test_string_static);
    RUN_TEST(test_string_dynamic);
    RUN_TEST(test_buffer);
    RUN_TEST(test_linkedList_single);
    RUN_TEST(test_linkedList_double);

    return UNITY_END();
}
