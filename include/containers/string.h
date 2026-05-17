#ifndef CORAL_CONTAINERS_STRING
#define CORAL_CONTAINERS_STRING

/*! \file */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "../common/api.h"
#include "../common/error.h"



// STRINGS

/*! \defgroup CORAL_CONTAINERS_STRING String_t
 *  \ingroup CORAL_CONTAINERS
 *  \brief Types representing strings and functions operating on strings.
 *  \addtogroup CORAL_CONTAINERS_STRING
 *  @{
 */



//- DEFINITIONS

typedef struct {
    union {
        const char* constStr;
        char* mutableStr;
    } rawStr;
    size_t used;                /*!< \brief Does not include the null-terminator of \p rawStr. */
    size_t capacity;            /*!< \brief Includes the null-terminator of \p rawStr. */
    bool isStatic;              /*!< \brief If \c true, \p rawStr can not be resized. */
    bool isConst;               /*!< \brief If \c true, the characters in \p rawStr can not be written to. */
    uint8_t additionalData[2U]; /*!< \brief May be used to store additional data instead of being wasted for padding. */
} String_t;



//- FUNCTIONS

/*! \brief Initializes \p string as not static and with a capacity of \p initCapacity.
 *  \param[in]  initCapacity    \copybrief String_t::capacity
 *  \returns \p CORAL_ERROR_NONE on success, \p CORAL_ERROR_OOM if an allocation error occured.
 *  \note If an allocation error occurs, \p string will not be vaild.
 */
CORAL_API Error_t String_init(String_t* string, size_t initCapacity);


/*! \brief Initializes \p string as static.
 *  \param[in]  used        If \c 0U, the size of \p rawStr will be determined.
 *  \param[in]  capacity    If \c 0U, the provided or determined \p used will be used to calculate the capacity.
 *  \returns
 *      \p CORAL_ERROR_NONE on success, \p CORAL_ERROR_INVALID_ARGS if a non-zero \p capacity was
 *      provided and was less than or equal to the provided or determined \p used.
 */
CORAL_API Error_t String_init_static(String_t* string, const char* rawStr, size_t used, size_t capacity, bool isConst);


/*! \brief Destructs \p string.
 *  \remarks If \p string is static, will return immediately.
 */
CORAL_API void String_destr(String_t* string);


/*! \brief Copies \p srcString to \p destString.
 *  \param[in]  destString  Must be mutable; if static, must have a capacity of at least <tt>srcString->used + 1U</tt>.
 *  \returns
 *      \p CORAL_ERROR_NONE on success, \p CORAL_ERROR_OOM if \p destString was not static and an allocation error occured or if
 *      \p destString was static and did not have sufficient capacity, \p CORAL_ERROR_INVALID_ARGS if \p destString was constant.
 *
 *  \note If an allocation error occurs, \p destString will still be valid.
 */
CORAL_API Error_t String_copy(String_t* restrict destString, const String_t* restrict srcString);


/*! \brief Copies \p count characters from \p srcString to \p destString.
 *  \param[in]  destString  Must be mutable; if static, must have a capacity of at least <tt>destStart + count + 1U</tt>.
 *  \param[in]  destStart   Will start copying into \p destString at \p destStart; must not be greater than \p destString->used.
 *  \param[in]  srcStart    Will start copying from \p srcString at \p srcStart; must not be greater than \p srcString->used.
 *  \param[in]  count       Must not be greater than <tt>srcString->used - srcStart</tt>.
 *  \returns
 *      \p CORAL_ERROR_NONE on success, \p CORAL_ERROR_OOM if \p destString was not static and an allocation error occured or if
 *      \p destString was static and did not have sufficient capacity, \p CORAL_ERROR_INVALID_ARGS if \p destString was constant,
 *      srcString was empty or if \p destStart or \p srcStart were too large.
 *
 *  \note If an allocation error occurs, \p destString will still be valid.
 */
CORAL_API Error_t String__copy(String_t* restrict destString, const String_t* restrict srcString, size_t destStart, size_t srcStart, size_t count);


/*! \brief Appends \p srcString to \p destString.
 *  \param[in]  destString  Must be mutable; if static, must have a capacity of at least <tt>destString->used + srcString->used + 1U</tt>.
 *  \returns
 *      \p CORAL_ERROR_NONE on success, \p CORAL_ERROR_OOM if \p destString was not static and an allocation error occured or if
 *      \p destString was static and did not have sufficient capacity, \p CORAL_ERROR_INVALID_ARGS if \p destString was constant.
 *
 *  \note If an allocation error occurs, \p destString will still be valid.
 */
CORAL_API Error_t String_concat(String_t* restrict destString, const String_t* restrict srcString);


/*! \brief Places \p count characters from \p srcString into \p destString.
 *  \param[in]  destString  Must be mutable; if static, must have a capacity of at least <tt>destString->used + count + 1U</tt>.
 *  \param[in]  destStart   Will start placing characters into \p destString at \p destStart; must not be greater than \p destString->used.
 *  \param[in]  srcStart    Will start copying from \p srcString at \p srcStart; must not be greater than \p srcString->used.
 *  \param[in]  count       Must not be greater than <tt>srcString->used - 1U - srcStart</tt>.
 *  \returns
 *      \p CORAL_ERROR_NONE on success, \p CORAL_ERROR_OOM if \p destString was not static and an allocation error occured or if
 *      \p destString was static and did not have sufficient capacity, \p CORAL_ERROR_INVALID_ARGS if \p destString was constant
 *      or if \p destStart or \p srcStart were too large.
 *
 *  \note If an allocation error occurs, \p destString will still be valid.
 */
CORAL_API Error_t String__concat(String_t* restrict destString, const String_t* restrict srcString, size_t destStart, size_t srcStart, size_t count);

/*! @} */

#endif
