#ifndef CORAL_CONTAINERS_LINKED_LIST
#define CORAL_CONTAINERS_LINKED_LIST

/*! \file */

#include <stddef.h>
#include <stdbool.h>
#include "../common/api.h"



// LINKED LISTS

/*! \defgroup CORAL_CONTAINERS_LINKED_LIST LinkedList_t
 *  \ingroup CORAL_CONTAINERS
 *  \brief Types representing different links and linked lists and functions operating on said types.
 *  \addtogroup CORAL_CONTAINERS_LINKED_LIST
 *  @{
 */



//- DEFINITIONS

//- - LINK TYPES

/*! \name Link types
 *  @{
 */

/*! \note
 *      Must be a member of a \c struct if a link is supposed to carry additional information.
 *      Pointers to links can then be upcast to pointers to their parents \c structs using \p CORAL_TO_PARENT_PTR().
 */
typedef struct LinkedList_SingleLink {
    struct LinkedList_SingleLink* next;
} LinkedList_SingleLink_t;


/*! \copydoc LinkedList_SingleLink_t */
typedef struct LinkedList_DoubleLink {
    struct LinkedList_DoubleLink* next;
    struct LinkedList_DoubleLink* previous;
} LinkedList_DoubleLink_t;


/*! \copydoc LinkedList_SingleLink_t */
typedef union {
    LinkedList_SingleLink_t* singleLink;
    LinkedList_DoubleLink_t* doubleLink;
} LinkedList_LinkPtr_u;

/*! @} */



//- - LIST TYPES

/*! \name List types
 *  @{
 */

/*! \brief Used both as a singly and doubly linked list.
 *  \see \p LinkedList_LinkPtr_u.
 */
typedef struct {
    LinkedList_LinkPtr_u headLink;
    LinkedList_LinkPtr_u tailLink;
} LinkedList_t;


/*! @} */



//- FUNCTIONS

/*! \name LinkedList_push
 *  @{
 */

/*! \brief Adds \p link at the front of \p list. */
CORAL_API void LinkedList_push_single(LinkedList_t* list, LinkedList_SingleLink_t* link);
CORAL_API void LinkedList_push_double(LinkedList_t* list, LinkedList_DoubleLink_t* link);

/*! @} */


/*! \name LinkedList_append
 *  @{
 */

/*! \brief Adds \p link at the back of \p list. */
CORAL_API void LinkedList_append_single(LinkedList_t* list, LinkedList_SingleLink_t* link);
CORAL_API void LinkedList_append_double(LinkedList_t* list, LinkedList_DoubleLink_t* link);

/*! @} */


/*! \name LinkedList_insert
 *  @{
 */

/*! \brief Inserts \p link into \p list at \p insertIndex. */
CORAL_API void LinkedList_insert_single(LinkedList_t* list, LinkedList_SingleLink_t* link, size_t insertIndex);
CORAL_API void LinkedList_insert_double(LinkedList_t* list, LinkedList_DoubleLink_t* link, size_t insertIndex);


/*! \copybrief LinkedList_insert_single
 *
 *  If \p listLength is known, this function may be faster than \p LinkedList_insert_single().
 */
CORAL_API void LinkedList__insert_single(LinkedList_t* list, LinkedList_SingleLink_t* link, size_t insertIndex, size_t listLength);


/*! \copybrief LinkedList_insert_double
 *
 *  If \p listLength is known, this function may be faster than \p LinkedList_insert_double().
 */
CORAL_API void LinkedList__insert_double(LinkedList_t* list, LinkedList_DoubleLink_t* link, size_t insertIndex, size_t listLength);

/*! @} */


/*! \name LinkedList_remove
 *  @{
 */

/*! \brief Removes \p link from \p list. */
CORAL_API void LinkedList_remove_single(LinkedList_t* list, LinkedList_SingleLink_t* link);
CORAL_API void LinkedList_remove_double(LinkedList_t* list, LinkedList_DoubleLink_t* link);

/*! @} */


/*! \name LinkedList_findPrevious
 *  @{
 */

/*! \brief Finds the link preceeding \p link in the list segment begining at \p startLink.
 *  \returns
 *      A pointer to the preceeding link or \c NULL if \p link was \p startLink or if \p link could
 *      not be found after \p startLink, which most likely means that they were not in the same list.
 */
CORAL_API LinkedList_SingleLink_t* LinkedList_findPrevious_single(LinkedList_SingleLink_t* startLink, LinkedList_SingleLink_t* link);

/*! @} */

/*! @} */

#endif
