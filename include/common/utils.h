#ifndef CORAL_COMMON_UTILS
#define CORAL_COMMON_UTILS

/*! \file */

#include <stdint.h>
#include "./api.h"
#include "./compilers.h"
#include "./noreturn.h"



// UTILS

/*! \defgroup CORAL_COMMON_UTILS Utils
 *  \ingroup CORAL_COMMON
 *  \brief Common utility macros.
 *  \addtogroup CORAL_COMMON_UTILS
 *  @{
 */



//- ASSERT

/*! \internal
 *  \brief
 *      Prints an error message and \p message to \c stderr as if an assertion had failed
 *      at \p line in \p file and then aborts the proccess.
 */
CORAL_API CORAL_NORETURN void coral_assert_base(const char* message, const char* file, unsigned line);


/*! \def CORAL_ASSERT
 *  \brief If \p CONDITION does not evaluate to \c true, the program will abort and print \p MESSAGE.
 *  \note Shamelessly copied from the mingw64 defintion of \c assert.
 */


/*! \def CORAL_HAS_ASSERT
 *  \brief
 *      \c 1 if \p CORAL_ASSERT checks the condition,
 *      \c 0 if it only evaluates the condition and ignores the outcome.
 */


#ifndef NDEBUG
#define CORAL_ASSERT(CONDITION, MESSAGE) (void)((CONDITION) || (coral_assert_base(MESSAGE, __FILE__, __LINE__), 0))
#define CORAL_HAS_ASSERT (1)
#else 
#define CORAL_ASSERT(CONDITION, MESSAGE) (void)(CONDITION)
#define CORAL_HAS_ASSERT (0)
#endif



//- MIN/MAX

/*! \brief Evaluates to the minimum value of \p A and \p B.
 *  \warning
 *      Evaluating \p A and \p B should not include side effects such as function calls,
 *      as either \p A or \p B will be evaluated twice, which could lead to unintended behaviour.
 */
#define CORAL_MIN(A, B) (((A) < (B)) ? (A) : (B))


/*! \brief Evaluates to the maximum value of \p A and \p B.
 *  \copydetails CORAL_MIN
 */
#define CORAL_MAX(A, B) (((A) > (B)) ? (A) : (B))



//- MEMBER POINTER CASTING

/*! \brief Evaluates the pointer to \p PARENT_NAME. */
#define CORAL_TO_PARENT_PTR(PARENT_NAME, MEMBER_NAME, MEMBER_PTR)              ((PARENT_NAME*)((uintptr_t)(MEMBER_PTR) - (uintptr_t)offsetof(PARENT_NAME, MEMBER_NAME)))


/*! \copybrief CORAL_TO_PARENT_PTR
 *  \param[in]  MEMBER_PTR  If \c NULL, the resulting pointer will be \c NULL aswell.
 */
#define CORAL_TO_PARENT_PTR_NULL_SAFE(PARENT_NAME, MEMBER_NAME, MEMBER_PTR)    ((MEMBER_PTR) ? (CORAL_TO_PARENT_PTR(PARENT_NAME, MEMBER_NAME, MEMBER_PTR)) : ((PARENT_NAME*)NULL))



//- ALIGNMENT

/*! \brief Rounds \p SIZE up to the next multiple of \p ALIGNMENT. */
#define CORAL_TO_ALIGNMENT(SIZE, ALIGNMENT)         ((SIZE) + (((SIZE) % (ALIGNMENT)) ? ((ALIGNMENT) - ((SIZE) % (ALIGNMENT))) : (0)))


/*! \brief Evaluates the size of \p TYPE_NAME if it were aligned to \p ALIGNMENT.  */
#define CORAL_ALIGNED_SIZEOF(TYPE_NAME, ALIGNMENT)  CORAL_TO_ALIGNMENT(sizeof(TYPE_NAME), ALIGNMENT)



//- MACRO HELPERS

#define CORAL_EXPAND(ARG)               ARG
#define CORAL__EXPAND(FUNCTION, ...)    FUNCTION(__VA_ARGS__)
#define CORAL_CONCAT(LHS, RHS)          LHS ## RHS



//- ARRAY LENGTH

#define CORAL_ARRAY_LENGTH(ARRAY) (sizeof(ARRAY) / sizeof((ARRAY)[0U]))



//- SPECIAL MSVC DEFINITION

#if CORAL_IS_COMPILER(CORAL_COMPILER_MSVC) && defined(CORAL_MSVC_MAX_ALIGN_FIX) && !defined(__cplusplus)

// May be correct, may not be correct
// MSVC should define this but still doesn't
typedef struct {
    long double reserved;
} max_align_t;

#endif

/*! @} */

#endif
