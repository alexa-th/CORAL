#ifndef CORAL_MATH_MAT
#define CORAL_MATH_MAT

/*! \file */

#include <stddef.h>
#include "../common/api.h"
#include "../common/error.h"
#include "./baseTypes.h"
#include "./vecTypes.h"
#include "./matTypes.h"

#ifdef __cplusplus
#   error "This header file is not usable in C++."
#endif


// MATRICES

/*! \defgroup CORAL_MATH_MAT Matrices
 *  \ingroup CORAL_MATH
 *  \brief Types representing matrices and functions operating on matrices.
 */

/*! \defgroup CORAL_MATH_MAT_OPERATIONS Matrix operations
 *  \ingroup CORAL_MATH_MAT
 *  \brief Functions operating on matrices.
 *
 *  Matrix function names follow the specific pattern \p MatCxRT_\<OPERATION\>, where \p C and \p R are the
 *  dimensionality of the columns and rows respectively, \p T is the abbreviation of the base type of the matrix and
 *  \p \<OPERATION\> is the name of the operation.
 *
 *  Only a few operations are defined for matrices:
 *      - Multiplication: These functions take two matrices to be multiplied and another to store the result in. The
 *          function signatures follow the pattern
 *          <tt>void MatCxRT_mul(const MatCxRT_u* lhs, const MatCxRT_u* rhs, MatCxRT_u* restrict result)</tt>, where
 *          \p C, \p R and \p T are equivalent to the definitions above. Although matrix multiplication is defined for
 *          non-square matrices aswell, only multiplication of square matrices has been implemented. Note that \p result
 *          may not point to the same matrix as any of \p lhs and \p rhs, but \p lhs and \p rhs may point to the same
 *          matrix.
 *
 *      - Transposition: These functions take a matrix to be transposed and another to store the result in. The function
 *          signatures follow the pattern
 *          <tt>void MatCxRT_transpose(const MatCxRT_u* mat, MatRxCT_u* restrict result)</tt>, where \p C, \p R and \p T
 *          are equivalent to the definitions above. Note that \p mat and \p result may not point to the same matrix.
 *
 *      - Inversion: These functions take a matrix to invert and another to store the result in and return an error code
 *          if the matrix cannot be inverted. The function signatures follow the pattern
 *          <tt>Error_t MatCxRT_invert(const MatCxRT_u* mat, MatCxRT_u* restrict result)</tt>, where \p C, \p R and \p T
 *          are equivalent to the definitions above. These functions return \p CORAL_ERROR_INVALID_ARGS if the matrix
 *          cannot be inverted, otherwise they return \p CORAL_ERROR_NONE. Note that, because the inverse is only
 *          defined for square matrices, \p C and \p R must be equal and that \p mat and \p result may not point to the
 *          same matrix.
 *
 *      - Determinant: These functions take a matrix and return the determinant of that matrix as a scalar value. The
 *          function signatures follow the pattern <tt>\<TYPE\> MatCxRT_determinant(const MatCxRT_u* mat)</tt>, where
 *          \p C, \p R and \p T are equivalent to the definitions above. Note that, because the determinant is only
 *          defined for square matrices, \p C and \p R must be equal.
 */


//- TEMPLATES

//- - OPERATIONS

//- - - ANY DIMENSIONALITY

#define CORAL_MATH_MAT_OPERATION_MUL(MAT_TYPE, MAT_NAME)        \
    CORAL_API void MAT_NAME ## _mul(const MAT_TYPE* lhs,        \
                                    const MAT_TYPE* rhs,        \
                                    MAT_TYPE* restrict result   \
    )


#define CORAL_MATH_MAT_OPERATION_TRANSPOSE(MAT_TYPE, MAT_NAME, TRANSPOSED_MAT_TYPE) \
    CORAL_API void MAT_NAME ## _transpose(const MAT_TYPE* mat,                      \
                                          TRANSPOSED_MAT_TYPE* restrict result      \
    )


#define CORAL_MATH_MAT_CxR_OPERATIONS_BASE(MAT_TYPE, MAT_NAME, TRANSPOSED_MAT_TYPE) \
    CORAL_MATH_MAT_OPERATION_MUL(MAT_TYPE, MAT_NAME);                               \
    CORAL_MATH_MAT_OPERATION_TRANSPOSE(MAT_TYPE, MAT_NAME, TRANSPOSED_MAT_TYPE);


#define CORAL_MATH_MAT_CxR_OPERATIONS(COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY, CORAL_MATH_TYPE_)  \
    CORAL_MATH_MAT_CxR_OPERATIONS_BASE(CORAL_MATH_MAT_GET_DEF(COLUMN_DIMENSIONALITY,                \
                                                              ROW_DIMENSIONALITY,                   \
                                                              CORAL_MATH_TYPE_                      \
                                       ),                                                           \
                                       CORAL_MATH_MAT_GET_NAME(COLUMN_DIMENSIONALITY,               \
                                                               ROW_DIMENSIONALITY,                  \
                                                               CORAL_MATH_TYPE_                     \
                                       ),                                                           \
                                       CORAL_MATH_MAT_GET_DEF(ROW_DIMENSIONALITY,                   \
                                                              COLUMN_DIMENSIONALITY,                \
                                                              CORAL_MATH_TYPE_                      \
                                       )                                                            \
    )



//- - - SQUARE DIMENSIONALITY

// Returns CORAL_ERROR_INVALID_ARGS if mat is not invertable
#define CORAL_MATH_MAT_SQUARE_OPERATION_INVERT(MAT_TYPE, MAT_NAME)      \
    CORAL_API Error_t MAT_NAME ## _invert(const MAT_TYPE* mat,          \
                                          MAT_TYPE* restrict result     \
    )


#define CORAL_MATH_MAT_SQUARE_OPERATION_DETERMINANT(MAT_TYPE, MAT_NAME, ELEMENT_TYPE)   \
    CORAL_API ELEMENT_TYPE MAT_NAME ## _determinant(const MAT_TYPE* mat)


#define CORAL_MATH_MAT_SQUARE_OPERATIONS_BASE(MAT_TYPE, MAT_NAME, ELEMENT_TYPE)         \
    CORAL_MATH_MAT_SQUARE_OPERATION_INVERT(MAT_TYPE, MAT_NAME);                         \
    CORAL_MATH_MAT_SQUARE_OPERATION_DETERMINANT(MAT_TYPE, MAT_NAME, ELEMENT_TYPE);


#define CORAL_MATH_MAT_SQUARE_OPERATIONS(DIMENSIONALITY, CORAL_MATH_TYPE_)              \
    CORAL_MATH_MAT_SQUARE_OPERATIONS_BASE(CORAL_MATH_MAT_GET_DEF(DIMENSIONALITY,        \
                                                                 DIMENSIONALITY,        \
                                                                 CORAL_MATH_TYPE_       \
                                          ),                                            \
                                          CORAL_MATH_MAT_GET_NAME(DIMENSIONALITY,       \
                                                                  DIMENSIONALITY,       \
                                                                  CORAL_MATH_TYPE_      \
                                          ),                                            \
                                          CORAL_MATH_GET_TYPE_NAME(CORAL_MATH_TYPE_)    \
    )



//- - <

#define CORAL_MATH_MAT_ALL_OPERATIONS(CORAL_MATH_TYPE_)                                 \
    CORAL_MATH_MAT_CxR_OPERATIONS(2, 2, CORAL_MATH_TYPE_)                               \
    CORAL_MATH_MAT_CxR_OPERATIONS(3, 3, CORAL_MATH_TYPE_)                               \
    CORAL_MATH_MAT_CxR_OPERATIONS(4, 4, CORAL_MATH_TYPE_)                               \
    CORAL_MATH_VEC_FOR_EACH_DIMENSIONALITY(CORAL_MATH_MAT_SQUARE_OPERATIONS,            \
                                           CORAL_MATH_TYPE_                             \
    )



//- DEFINITIONS

CORAL_MATH_FOR_EACH_TYPE(CORAL_MATH_MAT_ALL_OPERATIONS)



//- GENERICS

/*! \addtogroup CORAL_MATH_MAT_OPERATIONS
 *  @{
 */

/*! \name Generics
 *  @{
 */

/*! \cond EXCLUDE */

#define CORAL_MATH_MAT_GENERIC_SINGLE_ASSOCIATION_BASE(MAT_TYPE, MAT_NAME, OPERATION_NAME)  \
    MAT_TYPE:                                                                               \
        CORAL_CONCAT(MAT_NAME, OPERATION_NAME),


#define CORAL_MATH_MAT_GENERIC_SINGLE_ASSOCIATION(CORAL_MATH_TYPE_, COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY, OPERATION_NAME)  \
    CORAL_MATH_MAT_GENERIC_SINGLE_ASSOCIATION_BASE(CORAL_MATH_MAT_GET_DEF(COLUMN_DIMENSIONALITY,                                \
                                                                          ROW_DIMENSIONALITY,                                   \
                                                                          CORAL_MATH_TYPE_                                      \
                                                   ),                                                                           \
                                                   CORAL_MATH_MAT_GET_NAME(COLUMN_DIMENSIONALITY,                               \
                                                                           ROW_DIMENSIONALITY,                                  \
                                                                           CORAL_MATH_TYPE_                                     \
                                                   ),                                                                           \
                                                   _ ## OPERATION_NAME                                                          \
    )

/*! \endcond EXCLUDE */


/*! \brief Can be used to call a matrix operation if the dimensionalities of the matrix are not explicitly known. */
#define CORAL_MATH_MAT_GENERIC(COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY, OPERATION_NAME, MAT_ARG0, ...)    \
    _Generic(*(MAT_ARG0),                                                                                   \
             CORAL_MATH_FOR_EACH_TYPE(CORAL_MATH_MAT_GENERIC_SINGLE_ASSOCIATION,                            \
                                      COLUMN_DIMENSIONALITY,                                                \
                                      ROW_DIMENSIONALITY,                                                   \
                                      OPERATION_NAME                                                        \
             )                                                                                              \
             default: (void (*)(void*, ...))NULL                                                            \
    )(MAT_ARG0, ## __VA_ARGS__)

/*! @} */

/*! @} */

#endif
