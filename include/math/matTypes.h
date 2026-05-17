#ifndef CORAL_MATH_MAT_TYPES
#define CORAL_MATH_MAT_TYPES

/*! \file */

#include "../common/utils.h"
#include "./baseTypes.h"
#include "./vecTypes.h"



// MATRIX TYPES

/*! \defgroup CORAL_MATH_MAT_TYPES Matrix types
 *  \ingroup CORAL_MATH_MAT
 *  \brief Types representing matrices of different dimensionalities and of different base types.
 *
 *  Matrices are represented using the \p MatCxRT_u unions, where \p C and \p R are the dimensionalities of the columns
 *  and the rows of the matrix respectively and \p T is abbreviation of the base type of the matrix.
 *
 *  All matrices have a \p columnVecs member, an array of vectors whose dimensionalities are equal to the matrix' row
 *  dimensionality with a length equal to the matrix' column dimensionality.
 *
 *  Elements of matrices may also be accessed using the \p raw member, an array aliasing the \p columnVecs array. Note
 *  that the elements are stored in column-major order.
 */



//- ITERATION HELPERS

#ifdef __cplusplus

#   define CORAL_MATH_MAT_FOR_EACH_DIMENSIONALITY_COLUMN(FUNCTION, ROW_DIMENSIONALITY, ...)     \
        FUNCTION(2, ROW_DIMENSIONALITY __VA_OPT__(,) __VA_ARGS__)                               \
        FUNCTION(3, ROW_DIMENSIONALITY __VA_OPT__(,) __VA_ARGS__)                               \
        FUNCTION(4, ROW_DIMENSIONALITY __VA_OPT__(,) __VA_ARGS__)


#   define CORAL_MATH_MAT_FOR_EACH_DIMENSIONALITY_ROW(FUNCTION, COLUMN_DIMENSIONALITY, ...)     \
        FUNCTION(COLUMN_DIMENSIONALITY, 2 __VA_OPT__(,) __VA_ARGS__)                            \
        FUNCTION(COLUMN_DIMENSIONALITY, 3 __VA_OPT__(,) __VA_ARGS__)                            \
        FUNCTION(COLUMN_DIMENSIONALITY, 4 __VA_OPT__(,) __VA_ARGS__)


#   define CORAL_MATH_MAT_FOR_EACH_DIMENSIONALITY_FULL(FUNCTION, ...)                           \
        CORAL_MATH_MAT_FOR_EACH_DIMENSIONALITY_COLUMN(FUNCTION, 2 __VA_OPT__(,) __VA_ARGS__)    \
        CORAL_MATH_MAT_FOR_EACH_DIMENSIONALITY_COLUMN(FUNCTION, 3 __VA_OPT__(,) __VA_ARGS__)    \
        CORAL_MATH_MAT_FOR_EACH_DIMENSIONALITY_COLUMN(FUNCTION, 4 __VA_OPT__(,) __VA_ARGS__)

#else

#   define CORAL_MATH_MAT_FOR_EACH_DIMENSIONALITY_COLUMN(FUNCTION, ROW_DIMENSIONALITY, ...)     \
        FUNCTION(2, ROW_DIMENSIONALITY, ## __VA_ARGS__)                                         \
        FUNCTION(3, ROW_DIMENSIONALITY, ## __VA_ARGS__)                                         \
        FUNCTION(4, ROW_DIMENSIONALITY, ## __VA_ARGS__)


#   define CORAL_MATH_MAT_FOR_EACH_DIMENSIONALITY_ROW(FUNCTION, COLUMN_DIMENSIONALITY, ...)     \
        FUNCTION(COLUMN_DIMENSIONALITY, 2, ## __VA_ARGS__)                                      \
        FUNCTION(COLUMN_DIMENSIONALITY, 3, ## __VA_ARGS__)                                      \
        FUNCTION(COLUMN_DIMENSIONALITY, 4, ## __VA_ARGS__)


#   define CORAL_MATH_MAT_FOR_EACH_DIMENSIONALITY_FULL(FUNCTION, ...)                           \
        CORAL_MATH_MAT_FOR_EACH_DIMENSIONALITY_COLUMN(FUNCTION, 2, ## __VA_ARGS__)              \
        CORAL_MATH_MAT_FOR_EACH_DIMENSIONALITY_COLUMN(FUNCTION, 3, ## __VA_ARGS__)              \
        CORAL_MATH_MAT_FOR_EACH_DIMENSIONALITY_COLUMN(FUNCTION, 4, ## __VA_ARGS__)

#endif



//- TEPMPLATES

#define CORAL_MATH_MAT_TYPEDEF__BASE(COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY, TYPE, COLUMN_VEC_TYPE, ABBREVIATION)    \
    typedef union {                                                                                                     \
        TYPE raw[COLUMN_DIMENSIONALITY * ROW_DIMENSIONALITY];                                                           \
        COLUMN_VEC_TYPE columnVecs[COLUMN_DIMENSIONALITY];                                                              \
    } Mat ## COLUMN_DIMENSIONALITY ## x ## ROW_DIMENSIONALITY ## ABBREVIATION ## _u;


#define CORAL_MATH_MAT_TYPEDEF_BASE(COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY, CORAL_MATH_TYPE_, ABBREVIATION)  \
    CORAL_MATH_MAT_TYPEDEF__BASE(COLUMN_DIMENSIONALITY,                                                         \
                                 ROW_DIMENSIONALITY,                                                            \
                                 CORAL_MATH_GET_TYPE_NAME(CORAL_MATH_TYPE_),                                    \
                                 CORAL_MATH_VEC_GET_DEF(ROW_DIMENSIONALITY, CORAL_MATH_TYPE_),                  \
                                 ABBREVIATION                                                                   \
    )


#define CORAL_MATH_MAT_TYPEDEF(COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY, CORAL_MATH_TYPE_) \
    CORAL_MATH_MAT_TYPEDEF_BASE(COLUMN_DIMENSIONALITY,                                      \
                                ROW_DIMENSIONALITY,                                         \
                                CORAL_MATH_TYPE_,                                           \
                                CORAL_MATH_GET_TYPE_ABBREVIATION(CORAL_MATH_TYPE_)          \
    )


#define CORAL_MATH_MAT_TYPEDEFS(CORAL_MATH_TYPE_)                           \
    CORAL_MATH_MAT_FOR_EACH_DIMENSIONALITY_FULL(CORAL_MATH_MAT_TYPEDEF,     \
                                                CORAL_MATH_TYPE_            \
    )



//- DEFINITIONS

/*! \cond EXCLUDE */

CORAL_MATH_FOR_EACH_TYPE(CORAL_MATH_MAT_TYPEDEFS)

/*! \endcond EXCLUDE */



//- - CONVINIENCE DEFINITIONS

/*! \addtogroup CORAL_MATH_MAT_TYPES
 *  @{
 */

/*! \name Convenience definitions
 *  @{
 */

#define CORAL_MATH_MAT_GET_NAME_NO_ABBREV(COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY)                    \
    Mat ## COLUMN_DIMENSIONALITY ## x ## ROW_DIMENSIONALITY


#define CORAL_MATH_MAT_GET_NAME_FROM_ABBREV(COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY, ABBREVIATION)    \
    Mat ## COLUMN_DIMENSIONALITY ## x ## ROW_DIMENSIONALITY ## ABBREVIATION


#define CORAL_MATH_MAT_GET_DEF_FROM_ABBREV(COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY, ABBREVIATION)     \
    Mat ## COLUMN_DIMENSIONALITY ## x ## ROW_DIMENSIONALITY ## ABBREVIATION ## _u


#define CORAL_MATH_MAT_GET_NAME(COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY, CORAL_MATH_TYPE_)    \
    CORAL__EXPAND(CORAL_MATH_MAT_GET_NAME_FROM_ABBREV,                                          \
                  COLUMN_DIMENSIONALITY,                                                        \
                  ROW_DIMENSIONALITY,                                                           \
                  CORAL_MATH_GET_TYPE_ABBREVIATION(CORAL_MATH_TYPE_)                            \
    )


#define CORAL_MATH_MAT_GET_DEF(COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY, CORAL_MATH_TYPE_)     \
    CORAL__EXPAND(CORAL_MATH_MAT_GET_DEF_FROM_ABBREV,                                           \
                  COLUMN_DIMENSIONALITY,                                                        \
                  ROW_DIMENSIONALITY,                                                           \
                  CORAL_MATH_GET_TYPE_ABBREVIATION(CORAL_MATH_TYPE_)                            \
    )

/*! @} */

/*! @} */

#endif
