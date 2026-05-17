#ifndef CORAL_MEMORY_DYNAMIC_ALLOC
#define CORAL_MEMORY_DYNAMIC_ALLOC

/*! \file */

#include <stdlib.h>



// DYNAMIC ALLOCATION

/*! \defgroup CORAL_MEMORY_DYNCAMIC_ALLOC Dynamic allocation
 *  \ingroup CORAL_MEMORY
 *  \brief Functions used in order to dynamically allocate and manage memory.
 *  \addtogroup CORAL_MEMORY_DYNCAMIC_ALLOC
 *  @{
 */



//- UNTRACKED ALLOCATION

/*! \name Untracked allocation
 *  @{
 */

#define CORAL_malloc    malloc
#define CORAL_calloc    calloc
#define CORAL_realloc   realloc
#define CORAL_free      free

/*! @} */

/*! @} */

#endif
