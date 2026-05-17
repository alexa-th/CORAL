#ifndef CORAL_MEMORY_ALLOCATORS
#define CORAL_MEMORY_ALLOCATORS

/*! \file */

#include <stddef.h>
#include <stdbool.h>
#include "../common/api.h"
#include "../common/atomic.h"
#include "../containers/linkedList.h"



// CUSTOM ALLOCATORS

/*! \defgroup CORAL_MEMORY_CUSTOM_ALLOCATORS Custom allocators
 *  \ingroup CORAL_MEMORY
 *  \brief Types and functions providing custom allocation behavior.
 *  \addtogroup CORAL_MEMORY_CUSTOM_ALLOCATORS
 *  @{
 */



//- STACK ALLOCATOR

/*! \defgroup CORAL_MEMORY_STACK_ALLOCATOR StackAllocator_t
 *  \copybrief StackAllocator_t
 *  \addtogroup CORAL_MEMORY_STACK_ALLOCATOR
 *  @{
 */


/*! \brief Permits allocation of differently sized objects which can be freed all at once.
 *  \note
 *      Objects must be freed in reverse order of allocation: If object \c A was allocated before object \c B,
 *      object \c B must be freed before object \c A. Alternatively, only object \c A can be freed, which frees
 *      both \c A and \c B.
 */
typedef struct {
    void* stackBase;
    void* stackPtr;
    size_t stackSize;
} StackAllocator_t;


/*! \returns \c NULL if \p stackAlloc is not large enough. */
CORAL_API void* StackAllocator_alloc(StackAllocator_t* stackAlloc, size_t blockSize);


/*! \returns \c NULL if \p stackAlloc is not large enough, otherwise a pointer with an alignment of \p alignment. */
CORAL_API void* StackAllocator_alignedAlloc(StackAllocator_t* stackAlloc, size_t blockSize, size_t alignment);


/*! \brief Aligns \p stackAlloc to \p alignment.
 *  \returns \c true if \p stackAlloc was large enough to align it, otherwise \c false.
 */
CORAL_API bool StackAllocator_toNextAlignmentBarrier(StackAllocator_t* stackAlloc, size_t alignment);

/*! @} */



//- POOL ALLOCATOR

/*! \defgroup CORAL_MEMORY_POOL_ALLOCATOR PoolAllocator_t
 *  \copybrief PoolAllocator_t
 *  \addtogroup CORAL_MEMORY_POOL_ALLOCATOR
 *  @{
 */


/*! \brief Permits allocation of objects of the same size which can be freed out of order. */
typedef struct {
    void* poolBase;
    uint32_t elementCount;
    uint32_t elementSize;   /*!< \note Must be at least \c sizeof(uint32_t). */

    /*! \brief Index of an element which was allocated and then freed, if no such element exists \p elementCount. */
    uint32_t nextFreeIndex;

    /*! \brief Index of an element which has never been allocated, if no such element exists \p elementCount. */
    uint32_t nextUnusedFreeIndex;
} PoolAllocator_t;


/*! \copybrief PoolAllocator_t */
typedef struct {
    void* poolBase;
    uint32_t elementCount;
    uint32_t elementSize;                                   /*!< \copydoc PoolAllocator_t::elementSize */
    volatile CORAL_ATOMIC(uint32_t) nextFreeIndex;          /*!< \copydoc PoolAllocator_t::nextFreeIndex */
    volatile CORAL_ATOMIC(uint32_t) nextUnusedFreeIndex;    /*!< \copydoc PoolAllocator_t::nextUnusedFreeIndex */
} PoolAllocator_Atomic_t;


/*! @{ */

/*! \brief Initializes \p poolAlloc. */
CORAL_API void PoolAllocator_init(PoolAllocator_t* poolAlloc, void* poolBase, uint32_t elementCount, uint32_t elementSize);
CORAL_API void PoolAllocator_init_atomic(PoolAllocator_Atomic_t* poolAlloc, void* poolBase, uint32_t elementCount, uint32_t elementSize);


/*! \brief Initializes the <em>dynamically allocated</em> \p poolAlloc. */
CORAL_API void PoolAllocator_init__atomic(PoolAllocator_Atomic_t* poolAlloc, void* poolBase, uint32_t elementCount, uint32_t elementSize);

/*! @} */


/*! @{ */

/*! \returns The new element if a free element in \p poolAlloc was available, otherwise \c NULL. */
CORAL_API void* PoolAllocator_alloc(PoolAllocator_t* poolAlloc);
CORAL_API void* PoolAllocator_alloc_atomic(PoolAllocator_Atomic_t* poolAlloc);

/*! @} */


/*! @{ */

/*! \brief Marks \p element as an available element in \p poolAlloc. */
CORAL_API void PoolAllocator_free(PoolAllocator_t* poolAlloc, void* element);
CORAL_API void PoolAllocator_free_atomic(PoolAllocator_Atomic_t* poolAlloc, void* element);

/*! @} */


/*! \brief Returns \p poolAlloc to the state it was after calling \p PoolAllocator_init().
 *  \note
 *      Can not be done with a \p PoolAllocator_Atomic_t, as other threads may
 *      hold pointers to elements wich are still needed.
 */
CORAL_API void PoolAllocator_clear(PoolAllocator_t* poolAlloc);

/*! @} */



//- DYNAMIC POOL ALLOCATOR

/*! \defgroup CORAL_MEMORY_DYNAMIC_POOL_ALLOCATOR DynamicPoolAllocator_t
 *  \copybrief DynamicPoolAllocator_t
 *  \addtogroup CORAL_MEMORY_DYNAMIC_POOL_ALLOCATOR
 *  @{
 */

/*! \brief Decribes a single pool in a \p DynamicPoolAllocator_t. */
typedef struct DynamicPoolAllocator_Pool DynamicPoolAllocator_Pool_ot;


/*! \brief
 *      Like a \p PoolAllocator_t, objects of the same size can be allocated and then deallocated
 *      out of order; there is, however, no limit to the amount of objects that can be allocated.
 *
 *  By allocating blocks of memory that are able to hold multiple objects of the same size each and
 *  than allocating objects from a block using a \p PoolAllocator_t, the amount of objects a
 *  \p DynamicPoolAllocator_t can supply is able to grow or shrink by allocating or deallocating
 *  blocks respectively.
 */
typedef struct {
    LinkedList_t poolList;
    uint32_t elementCount;              /*!< \brief Count of elements per pool. */
    uint32_t elementSize;               /*!< \copydoc PoolAllocator_t::elementSize */
} DynamicPoolAllocator_t;


/*! \brief Initializes \p dynamicPoolAlloc. */
CORAL_API void DynamicPoolAllocator_init(DynamicPoolAllocator_t* dynamicPoolAlloc, uint32_t elementCount, uint32_t elementSize);


/*! \brief Destructs \p dynamicPoolAlloc.
 *  \note Any objects that have been allocated but not freed by \p dynamicPoolAlloc will not be usable after this function returns.
 */
CORAL_API void DynamicPoolAllocator_destr(DynamicPoolAllocator_t* dynamicPoolAlloc);


/*! \returns The new element; does not return \c NULL. */
CORAL_API void* DynamicPoolAllocator_alloc(DynamicPoolAllocator_t* dynamicPoolAlloc);


/*! \brief Marks \p element as available in one of the pools of \p dynamicPoolAlloc. */
CORAL_API void DynamicPoolAllocator_free(DynamicPoolAllocator_t* dynamicPoolAlloc, void* element);


/*! \brief
 *      Returns each \p DynamicPoolAllocator_Pool_ot of \p dynamicPoolAlloc to the state
 *      it was immediately after being allocated.
 */
CORAL_API void DynamicPoolAllocator_clear(DynamicPoolAllocator_t* dynamicPoolAlloc);

/*! @} */

/*! @} */

#endif
