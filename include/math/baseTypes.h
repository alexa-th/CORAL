#ifndef CORAL_MATH_TYPES
#define CORAL_MATH_TYPES

/*! \file */

#include <stdint.h>
#include <limits.h>
#include <math.h>
#include "../common/utils.h"



// TYPES

/*! \defgroup CORAL_MATH_TYPES Base types
 *  \ingroup CORAL_MATH
 *  \brief Fundamental types used by mathematical objects.
 *  \addtogroup CORAL_MATH_TYPES
 *  @{
 */

/*! \name CORAL_MATH_TYPE_
 *  @{
 */

/*! \brief When passed to most math macro functions, represents its corresponding type. */

#define CORAL_MATH_TYPE_BYTE        BYTE
#define CORAL_MATH_TYPE_INT         INT
#define CORAL_MATH_TYPE_UNSIGNED    UNSIGNED
#define CORAL_MATH_TYPE_FLOAT       FLOAT
#define CORAL_MATH_TYPE_DOUBLE      DOUBLE

/*! @} */


/*! \name CORAL_MATH_TYPE_NAME_
 *  @{
 */

/*! \brief Expands to its corresponding underlying type. */

#define CORAL_MATH_TYPE_NAME_BYTE       uint8_t
#define CORAL_MATH_TYPE_NAME_INT        int
#define CORAL_MATH_TYPE_NAME_UNSIGNED   unsigned
#define CORAL_MATH_TYPE_NAME_FLOAT      float
#define CORAL_MATH_TYPE_NAME_DOUBLE     double

/*! @} */


/*! \name CORAL_MATH_TYPE_ABBREVIATION_
 *  @{
 */

/*! \brief Expands to the abbreviation used for its corresponding type. */

#define CORAL_MATH_TYPE_ABBREVIATION_BYTE       b
#define CORAL_MATH_TYPE_ABBREVIATION_INT        i
#define CORAL_MATH_TYPE_ABBREVIATION_UNSIGNED   u
#define CORAL_MATH_TYPE_ABBREVIATION_FLOAT      f
#define CORAL_MATH_TYPE_ABBREVIATION_DOUBLE     d

/*! @} */


/*! \name CORAL_MATH_TYPE_MIN_
 *  @{
 */

/*! \brief Expands to the minimum value its corresponding type can represent. */

#define CORAL_MATH_TYPE_MIN_BYTE        (0U)
#define CORAL_MATH_TYPE_MIN_INT         (INT_MIN)
#define CORAL_MATH_TYPE_MIN_UNSIGNED    (0U)
#define CORAL_MATH_TYPE_MIN_FLOAT       (-HUGE_VALF)
#define CORAL_MATH_TYPE_MIN_DOUBLE      (-HUGE_VAL)

/*! @} */


/*! \name CORAL_MATH_TYPE_MAX_
 *  @{
 */

/*! \brief Expands to the maximum value its corresponding type can represent. */

#define CORAL_MATH_TYPE_MAX_BYTE        (UINT8_MAX)
#define CORAL_MATH_TYPE_MAX_INT         (INT_MAX)
#define CORAL_MATH_TYPE_MAX_UNSIGNED    (UINT_MAX)
#define CORAL_MATH_TYPE_MAX_FLOAT       (HUGE_VALF)
#define CORAL_MATH_TYPE_MAX_DOUBLE      (HUGE_VAL)

/*! @} */



//- CONDITIONALS

/*! \name CORAL_MATH_MAYBE_EXPAND_
 *  @{
 */

/*! \brief Expands to the given arguments if its corresponding type has not been disabled, otherwise expands to nothing. */

#ifdef CORAL_MATH_EXCLUDE_BYTE
#   define CORAL_MATH_MAYBE_EXPAND_BYTE(...)
#else
#   define CORAL_MATH_MAYBE_EXPAND_BYTE(...) __VA_ARGS__
#endif


#ifdef CORAL_MATH_EXCLUDE_INT
#   define CORAL_MATH_MAYBE_EXPAND_INT(...)
#else
#   define CORAL_MATH_MAYBE_EXPAND_INT(...) __VA_ARGS__
#endif


#ifdef CORAL_MATH_EXCLUDE_UNSIGNED
#   define CORAL_MATH_MAYBE_EXPAND_UNSIGNED(...)
#else
#   define CORAL_MATH_MAYBE_EXPAND_UNSIGNED(...) __VA_ARGS__
#endif


#ifdef CORAL_MATH_EXCLUDE_FLOAT
#   define CORAL_MATH_MAYBE_EXPAND_FLOAT(...)
#else
#   define CORAL_MATH_MAYBE_EXPAND_FLOAT(...) __VA_ARGS__
#endif


#ifdef CORAL_MATH_EXCLUDE_DOUBLE
#   define CORAL_MATH_MAYBE_EXPAND_DOUBLE(...)
#else
#   define CORAL_MATH_MAYBE_EXPAND_DOUBLE(...) __VA_ARGS__
#endif

/*! @} */



//- HELPERS

/*! \returns The underlying type represented by \p CORAL_MATH_TYPE_. */
#define CORAL_MATH_GET_TYPE_NAME(CORAL_MATH_TYPE_)          CORAL_EXPAND(CORAL_CONCAT(CORAL_MATH_TYPE_NAME_, CORAL_MATH_TYPE_))


/*! \returns The abbreviation used for \p CORAL_MATH_TYPE_. */
#define CORAL_MATH_GET_TYPE_ABBREVIATION(CORAL_MATH_TYPE_)  CORAL_EXPAND(CORAL_CONCAT(CORAL_MATH_TYPE_ABBREVIATION_, CORAL_MATH_TYPE_))


/*! \returns The minimum value the underlying type of \p CORAL_MATH_TYPE_ can represent. */
#define CORAL_MATH_GET_TYPE_MIN(CORAL_MATH_TYPE_)           CORAL_EXPAND(CORAL_CONCAT(CORAL_MATH_TYPE_MIN_, CORAL_MATH_TYPE_))


/*! \returns The maximum value the underlying type of \p CORAL_MATH_TYPE_ can represent. */
#define CORAL_MATH_GET_TYPE_MAX(CORAL_MATH_TYPE_)           CORAL_EXPAND(CORAL_CONCAT(CORAL_MATH_TYPE_MAX_, CORAL_MATH_TYPE_))



//- - ITERATION

/*! \def CORAL_MATH_FOR_EACH_TYPE
 *  \brief Expands \p FUNCTION for each type that has not been excluded.
 *
 *  \p FUNCTION must accept a \p CORAL_MATH_TYPE_ as its fist argument.
 *  Other arguments to pass to \p FUNCTION may be provided as variadic arguments.
 */

#ifdef __cplusplus

#   define CORAL_MATH_FOR_EACH_TYPE(FUNCTION, ...)                                                      \
        CORAL_MATH_MAYBE_EXPAND_BYTE    (FUNCTION(CORAL_MATH_TYPE_BYTE     __VA_OPT__(,) __VA_ARGS__))  \
        CORAL_MATH_MAYBE_EXPAND_INT     (FUNCTION(CORAL_MATH_TYPE_INT      __VA_OPT__(,) __VA_ARGS__))  \
        CORAL_MATH_MAYBE_EXPAND_UNSIGNED(FUNCTION(CORAL_MATH_TYPE_UNSIGNED __VA_OPT__(,) __VA_ARGS__))  \
        CORAL_MATH_MAYBE_EXPAND_FLOAT   (FUNCTION(CORAL_MATH_TYPE_FLOAT    __VA_OPT__(,) __VA_ARGS__))  \
        CORAL_MATH_MAYBE_EXPAND_DOUBLE  (FUNCTION(CORAL_MATH_TYPE_DOUBLE   __VA_OPT__(,) __VA_ARGS__))

#else

#   define CORAL_MATH_FOR_EACH_TYPE(FUNCTION, ...)                                              \
        CORAL_MATH_MAYBE_EXPAND_BYTE    (FUNCTION(CORAL_MATH_TYPE_BYTE,     ## __VA_ARGS__))    \
        CORAL_MATH_MAYBE_EXPAND_INT     (FUNCTION(CORAL_MATH_TYPE_INT,      ## __VA_ARGS__))    \
        CORAL_MATH_MAYBE_EXPAND_UNSIGNED(FUNCTION(CORAL_MATH_TYPE_UNSIGNED, ## __VA_ARGS__))    \
        CORAL_MATH_MAYBE_EXPAND_FLOAT   (FUNCTION(CORAL_MATH_TYPE_FLOAT,    ## __VA_ARGS__))    \
        CORAL_MATH_MAYBE_EXPAND_DOUBLE  (FUNCTION(CORAL_MATH_TYPE_DOUBLE,   ## __VA_ARGS__))

#endif

/*! @} */

#endif
