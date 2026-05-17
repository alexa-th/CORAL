#ifndef CORAL_CONTAINERS_BUFFER
#define CORAL_CONTAINERS_BUFFER

/*! \file */

#include <stddef.h>
#include <stdbool.h>
#include "../common/api.h"
#include "../common/error.h"



// BUFFER

/*! \defgroup CORAL_CONTAINERS_BUFFER Buffer_t
 *  \ingroup CORAL_CONTAINERS
 *  \copybrief Buffer_t
 *  \addtogroup CORAL_CONTAINERS_BUFFER
 *  @{
 */

/*! \brief \p Buffer_t::bufferPtr is usually allocated to a size of a multiple of \verblink{CORAL_CONTAINERS_BUFFER_CHUNK_SIZE}. */
#define CORAL_CONTAINERS_BUFFER_CHUNK_SIZE ((size_t)(2U * 1024U))


/*! \brief A dynamically resizeable buffer. */
typedef struct {
    void* bufferPtr;    /*!< \brief Pointer to the actual buffer. */
    size_t capacity;
    size_t used;
} Buffer_t;


/*! \returns \c 0 on success, \verblink{CORAL_ERROR_,CORAL_ERROR_OOM} on an OOM-error. */
CORAL_API Error_t Buffer_init(Buffer_t* buffer, size_t initCapacity);


/*! \brief Destructs \p buffer. */
CORAL_API void Buffer_destr(Buffer_t* buffer);


/*! @{ */

/*! \brief Expands \p buffer such that its size will be a multiple of \verblink{CORAL_CONTAINERS_BUFFER_CHUNK_SIZE}.
 *  \param[in]  freeOnOOMFailure    If an OOM-error occurs, frees \mlink{Buffer_t,buffer->,bufferPtr} and resets \p buffer.
 *  \returns \c 0 on success, \verblink{CORAL_ERROR_,CORAL_ERROR_OOM} on an OOM-error.
 */
CORAL_API Error_t Buffer_reserveNewBufferChunk(Buffer_t* buffer, bool freeOnOOMFailure);
CORAL_API Error_t Buffer__reserveNewBufferChunk(Buffer_t* buffer, size_t maxCapacity, bool freeOnOOMFailure);

/*! @} */


/*! @{ */

/*! \brief Expands \p buffer to a capacity of at least \p newCapacity.
 *  \param[in]  freeOnOOMFailure    If an OOM-error occurs, frees \mlink{Buffer_t,buffer->,bufferPtr} and resets \p buffer.
 *  \returns \c 0 on success, \verblink{CORAL_ERROR_,CORAL_ERROR_OOM} on an OOM-error.
 */
CORAL_API Error_t Buffer_reserveCapacity(Buffer_t* buffer, size_t newCapacity, bool freeOnOOMFailure);
CORAL_API Error_t Buffer__reserveCapacity(Buffer_t* buffer, size_t newCapacity, size_t maxCapacity, bool freeOnOOMFailure);


/*! \brief Expands \p buffer to a capacity of exactly \p newExactCapacity.
 *  \param[in]  freeOnOOMFailure    If an OOM-error occurs, frees \mlink{Buffer_t,buffer->,bufferPtr} and resets \p buffer.
 *  \returns \c 0 on success, \verblink{CORAL_ERROR_,CORAL_ERROR_OOM} on an OOM-error.
 */
CORAL_API Error_t Buffer_reserveExactCapacity(Buffer_t* buffer, size_t newExactCapacity, bool freeOnOOMFailure);

/*! @} */


/*! \brief Trims \p buffer to its currently used size. */
CORAL_API void Buffer_trimToUsedSize(Buffer_t* buffer);

/*! @} */

#endif
