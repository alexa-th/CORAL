#include <stddef.h>
#include "common/utils.h"
#include "containers/linkedList.h"



// LINKED LISTS

//- FUNCTIONS

//- - LINK INITIALIZATION

static inline void LinkedList_configLink_single(LinkedList_SingleLink_t* link, LinkedList_SingleLink_t* next, LinkedList_SingleLink_t* previous) {
    link->next = next;

    if (previous) { previous->next = link; }
}


static inline void LinkedList_configLink_double(LinkedList_DoubleLink_t* link, LinkedList_DoubleLink_t* next, LinkedList_DoubleLink_t* previous) {
    link->next = next;
    link->previous = previous;

    if (next) { next->previous = link; }
    if (previous) { previous->next = link; }
}



//- - INDEXED RETRIEVAL

static LinkedList_SingleLink_t* LinkedList_atIndex_single(LinkedList_t* list, size_t index) {
    LinkedList_SingleLink_t* link = list->headLink.singleLink;

    CORAL_ASSERT(link, "Empty list.");

    for (size_t i = 0U; i < index; i++) {
        link = link->next;
        CORAL_ASSERT(link, "Out of range.");
    }

    return link;
}


static LinkedList_DoubleLink_t* LinkedList_atIndex_double(LinkedList_t* list, size_t index) {
    LinkedList_DoubleLink_t* link = list->headLink.doubleLink;

    CORAL_ASSERT(link, "Empty list.");

    for (size_t i = 0U; i < index; i++) {
        link = link->next;
        CORAL_ASSERT(link, "Out of range.");
    }

    return link;
}


static LinkedList_DoubleLink_t* LinkedList_atIndex__double(LinkedList_t* list, size_t reverseIndex) {
    LinkedList_DoubleLink_t* link = list->tailLink.doubleLink;

    CORAL_ASSERT(link, "Empty list.");

    for (size_t i = 0U; i < reverseIndex; i++) {
        link = link->previous;
        CORAL_ASSERT(link, "Out of range.");
    }

    return link;
}



//- <

void LinkedList_push_single(LinkedList_t* list, LinkedList_SingleLink_t* link) {
    LinkedList_configLink_single(link, list->headLink.singleLink, NULL);

    if (!list->tailLink.singleLink) { list->tailLink.singleLink = link; }
    list->headLink.singleLink = link;
}


void LinkedList_push_double(LinkedList_t* list, LinkedList_DoubleLink_t* link) {
    LinkedList_configLink_double(link, list->headLink.doubleLink, NULL);

    if (!list->tailLink.doubleLink) { list->tailLink.doubleLink = link; }
    list->headLink.doubleLink = link;
}


void LinkedList_append_single(LinkedList_t* list, LinkedList_SingleLink_t* link) {
    LinkedList_configLink_single(link, NULL, list->tailLink.singleLink);

    if (!list->headLink.singleLink) { list->headLink.singleLink = link; }
    list->tailLink.singleLink = link;
}


void LinkedList_append_double(LinkedList_t* list, LinkedList_DoubleLink_t* link) {
    LinkedList_configLink_double(link, NULL, list->tailLink.doubleLink);

    if (!list->headLink.doubleLink) { list->headLink.doubleLink = link; }
    list->tailLink.doubleLink = link;
}


void LinkedList_insert_single(LinkedList_t* list, LinkedList_SingleLink_t* link, size_t insertIndex) {
    if (!insertIndex) {
        LinkedList_push_single(list, link);
        return;
    }

    LinkedList_SingleLink_t* previousSingleLink = LinkedList_atIndex_single(list, insertIndex - 1U);

    LinkedList_configLink_single(link, previousSingleLink->next, previousSingleLink);
    if (previousSingleLink == list->tailLink.singleLink) { list->tailLink.singleLink = link; }
}


void LinkedList_insert_double(LinkedList_t* list, LinkedList_DoubleLink_t* link, size_t insertIndex) {
    if (!insertIndex) {
        LinkedList_push_double(list, link);
        return;
    }

    LinkedList_DoubleLink_t* previousDoubleLink = LinkedList_atIndex_double(list, insertIndex - 1U);

    LinkedList_configLink_double(link, previousDoubleLink->next, previousDoubleLink);
    if (previousDoubleLink == list->tailLink.doubleLink) { list->tailLink.doubleLink = link; }
}


void LinkedList__insert_single(LinkedList_t* list, LinkedList_SingleLink_t* link, size_t insertIndex, size_t listLength) {
    CORAL_ASSERT(insertIndex <= listLength, "Out of range.");

    if (insertIndex == listLength) { LinkedList_append_single(list, link); }
    else { LinkedList_insert_single(list, link, insertIndex); }
}


void LinkedList__insert_double(LinkedList_t* list, LinkedList_DoubleLink_t* link, size_t insertIndex, size_t listLength) {
    CORAL_ASSERT(insertIndex <= listLength, "Out of range.");

    if (!insertIndex) {
        LinkedList_push_double(list, link);
        return;
    }

    if (insertIndex == listLength) {
        LinkedList_append_double(list, link);
        return;
    }

    size_t reverseInsertIndex = listLength - 1U - insertIndex;

    LinkedList_DoubleLink_t* previousDoubleLink = (insertIndex - 1U <= reverseInsertIndex + 1U) ? (LinkedList_atIndex_double(list, insertIndex - 1U)) : (LinkedList_atIndex__double(list, reverseInsertIndex + 1U));

    LinkedList_configLink_double(link, previousDoubleLink->next, previousDoubleLink);
    // No need to set tailLink, already covered by the append branch
}


void LinkedList_remove_single(LinkedList_t* list, LinkedList_SingleLink_t* link) {
    CORAL_ASSERT(list->headLink.singleLink, "Empty list.");
    LinkedList_SingleLink_t* previousSingleLink = NULL;

    if (link == list->headLink.singleLink) { list->headLink.singleLink = list->headLink.singleLink->next; }
    else {
        previousSingleLink = list->headLink.singleLink;
        while (previousSingleLink->next != link) {
            previousSingleLink = previousSingleLink->next;
            CORAL_ASSERT(previousSingleLink, "Link is not in list.");
        }

        previousSingleLink->next = link->next;
    }

    if (link == list->tailLink.singleLink) { list->tailLink.singleLink = previousSingleLink; }
}


void LinkedList_remove_double(LinkedList_t* list, LinkedList_DoubleLink_t* link) {
    CORAL_ASSERT(list->headLink.doubleLink, "Empty list.");

    if (link == list->headLink.doubleLink) { list->headLink.doubleLink = list->headLink.doubleLink->next; }
    if (link == list->tailLink.doubleLink) { list->tailLink.doubleLink = list->tailLink.doubleLink->previous; }

    if (link->previous) { link->previous->next = link->next; }
    if (link->next) { link->next->previous = link->previous; }
}


LinkedList_SingleLink_t* LinkedList_findPrevious_single(LinkedList_SingleLink_t* startLink, LinkedList_SingleLink_t* link) {
    if (startLink == link) { return NULL; }

    LinkedList_SingleLink_t* requestedLink = startLink;

    while (requestedLink->next != link) {
        requestedLink = requestedLink->next;
        CORAL_ASSERT(requestedLink, "Given link was not in list.");
    }

    return requestedLink;
}
