#ifndef CORAL_MATH_VEC
#define CORAL_MATH_VEC

/*! \file */

#include <stddef.h>
#include "../common/api.h"
#include "./vecTypes.h"
#include "./matTypes.h"

#ifdef __cplusplus
#   error "This header file is not usable in C++."
#endif



// VECTORS

/*! \defgroup CORAL_MATH_VEC Vectors
 *  \ingroup CORAL_MATH
 *  \brief Types representing vectors and functions operating on vectors.
 */

/*! \defgroup CORAL_MATH_VEC_OPERATIONS Vector operations
 *  \ingroup CORAL_MATH_VEC
 *  \brief Functions operating on vectors.
 *
 *  Vector function names follow the specific pattern \p VecNT_\<OPERATION\>, where \p N is the dimensionality of the
 *  vector or vectors, \p T is the abbreviation of the base type of the vector and \p \<OPERATION\> is the name of the
 *  operation.
 *
 *  Operations can be devided into specific groups:
 *      - <tt>Vec + Vec -> Vec</tt>: These functions first take in the two input vectors to operate on and then the
 *          result vector. The function signatures follow the pattern
 *          <tt>void VecNT_\<OPERATION\>(const VecNT_u* a, const VecNT_u* b, VecNT_u* result)</tt>, where \p N and \p T
 *          are equivalent to the definitions above and \p \<OPERATION\> is one of \p add, \p sub, \p mul and \p div.
 *          Additionally, if \p N is \c 3, \p \<OPERATION\> may also be \p cross, which will compute the cross-product
 *          of the vectors \p a and \p b. All other operations described by \p \<OPERATION\> will be performed on the
 *          corresponding elements of \p a and \p b and stored in the corresponding element of \p result; the elements
 *          of \p a are to be understood as the first operands and the elements of \p b as the second operands to the
 *          operation described by \p \<OPERATION\>, which is only relevant if the operation is not commutative. Note
 *          that \p result, \p a and \p b may all point to the exact same vector.
 *
 *      - <tt>Vec + Scalar -> Vec</tt>: These functions first take in the input vector, then the scalar value and
 *          finally the result vector. The function signatures follow the pattern
 *          <tt>void VecNT_\<OPERATION\>_scalar(const VecNT_u* vec, \<TYPE\> scalar, VecNT_u* result)</tt>, where \p N
 *          and \p T are equivalent to the definitions above, \p \<TYPE\> is the base type of the vectors and
 *          \p \<OPERATION\> is one of \p add, \p sub, \p mul and \p div. The operation described by \p \<OPERATION\>
 *          will be performed on each element of \p vec, always using the same \p scalar, and stored in the
 *          corresponding element of \p result. Here, the elemens of \p vec are to be understood as the first operands
 *          and \p scalar as the second operand to the operation described by \p \<OPERATION\>, which is only relevant
 *          if the operation is not commutative. Note that \p result and \p vec may point to the exact same vector.
 *
 *      - <tt>Vec + Vec -> Scalar</tt>: These functions take in the two input vectors and return the resulting scalar
 *          value. The function signatures follow the pattern
 *          <tt>\<TYPE\> VecNT_\<OPERATION\>(const VecNT_u* a, const VecNT_u* b)</tt>, where \p N and \p T are
 *          equivalent to the definitions above, \p \<TYPE\> is the base type of the vectors and \p \<OPERATION\> is one
 *          of \p dot and \p rawDot. Here, \p dot will compute the cosine similarity of \p a and \p b while \p rawDot
 *          will compute the actual dot product of \p a and \p b. Note that \p a and \p b may point to the exact same
 *          vector.
 *
 *      - <tt>Vec -> Vec</tt>: These functions first take in the input vector and then the result vector. The function
 *          signatures follow the pattern <tt>void VecNT_normalize(const VecNT_u* vec, VecNT_u* result)</tt>, where \p N
 *          and \p T are equivalent to the definitions above. These functions will normalize \p vec such that its
 *          magnitude is \c 1 and store the normalized vector in \p result. Note that \p result and \p vec may point to
 *          the exact same vector.
 *
 *      - <tt>Vec -> Scalar</tt>: These functions take in the input vector and return the resulting scalar value. The
 *          function signatures follow the pattern <tt>\<TYPE\> VecNT_\<OPERATION\>(const VecNT_u* vec)</tt>, where \p N
 *          and \p T are equivalent to the definitions above and \p \<OPERATION\> is one of \p magnitude and
 *          \p squaredMagnitude. Here, \p magnitude will calculate the magnitude of \p vec while \p squaredMagnitude
 *          will calculate the magnitude of \p vec raised to the power of \c 2.
 *
 *      - <tt>Mat + Vec -> Vec</tt>: These functions first take in the matrix, then the input vector and finally the
 *          result vector. The function signatures follow the pattern
 *          <tt>void VecCT_mul_MatCxR(const MatCxRT_u* mat, const VecCT_u* vec, VecRT_u* restrict result)</tt>, where
 *          \p T is equivalent to the definitions above and \p C and \p R are the dimensionalities of the columns and
 *          rows of the matrix respectively. These functions will perform a matrix-vector multiplication with the given
 *          arguments \p mat and \p vec and store the resulting vector in \p result. Note that the dimensionality of the
 *          input vector must match the dimensionality of the columns of the matrix and that the dimensionality of the
 *          result vector must match the dimensionality of the rows of the matrix. Also note that, if \p C and \p R are
 *          equal, \p result and \p vec may \e not point to the exact same vector.
 */



//- TEMPLATES

//- - NAME HELPERS

/*! \addtogroup CORAL_MATH_VEC_OPERATIONS
 *  @{
 */

/*! \name Name helpers
 *  @{
 */

#define CORAL_MATH_VEC_GET_FUNC_NAME_BASE(VEC_NAME, OPERATION_NAME) CORAL_MATH_VEC__GET_FUNC_NAME(VEC_NAME, OPERATION_NAME)


#define CORAL_MATH_VEC_GET_FUNC_NAME(DIMENSIONALITY, CORAL_MATH_TYPE_, OPERATION_NAME)  \
    CORAL_MATH_VEC_GET_FUNC_NAME_BASE(CORAL_MATH_VEC_GET_NAME(DIMENSIONALITY,           \
                                                              CORAL_MATH_TYPE_          \
                                      ),                                                \
                                      OPERATION_NAME                                    \
    )


#define CORAL_MATH_VEC__GET_FUNC_NAME(VEC_NAME, OPERATION_NAME) VEC_NAME ## _ ## OPERATION_NAME

/*! @} */

/*! @} */



//- - OPERATIONS

//- - - BASIC OPERATIONS

#define CORAL_MATH_VEC_OPERATION_XXX(VEC_TYPE, VEC_NAME, OPERATION_NAME)                                    \
    CORAL_API void CORAL_MATH_VEC__GET_FUNC_NAME(VEC_NAME, OPERATION_NAME)(const VEC_TYPE* a,               \
                                                                           const VEC_TYPE* b,               \
                                                                           VEC_TYPE* result                 \
    )


#define CORAL_MATH_VEC_OPERATION_XXX_SCALAR(VEC_TYPE, VEC_NAME, ELEMENT_TYPE, OPERATION_NAME)               \
    CORAL_API void CORAL_MATH_VEC__GET_FUNC_NAME(VEC_NAME, OPERATION_NAME ## _scalar)(const VEC_TYPE* vec,  \
                                                                                      ELEMENT_TYPE scalar,  \
                                                                                      VEC_TYPE* result      \
    )


#define CORAL_MATH_VEC_OPERATION_DOT(VEC_TYPE, VEC_NAME, ELEMENT_TYPE)                                      \
    CORAL_API ELEMENT_TYPE CORAL_MATH_VEC__GET_FUNC_NAME(VEC_NAME, dot)(const VEC_TYPE* a,                  \
                                                                        const VEC_TYPE* b                   \
    )


#define CORAL_MATH_VEC_OPERATION_RAW_DOT(VEC_TYPE, VEC_NAME, ELEMENT_TYPE)                                  \
    CORAL_API ELEMENT_TYPE CORAL_MATH_VEC__GET_FUNC_NAME(VEC_NAME, rawDot)(const VEC_TYPE* a,               \
                                                                           const VEC_TYPE* b                \
    )


#define CORAL_MATH_VEC_OPERATION_NORMALIZE(VEC_TYPE, VEC_NAME)                                              \
    CORAL_API void CORAL_MATH_VEC__GET_FUNC_NAME(VEC_NAME, normalize)(const VEC_TYPE* vec,                  \
                                                                      VEC_TYPE* result                      \
    )


#define CORAL_MATH_VEC_OPERATION_MAGNITUDE(VEC_TYPE, VEC_NAME, ELEMENT_TYPE)                                \
    CORAL_API ELEMENT_TYPE CORAL_MATH_VEC__GET_FUNC_NAME(VEC_NAME, magnitude)(const VEC_TYPE* vec)


#define CORAL_MATH_VEC_OPERATION_SQUARED_MAGNITUDE(VEC_TYPE, VEC_NAME, ELEMENT_TYPE)                        \
    CORAL_API ELEMENT_TYPE CORAL_MATH_VEC__GET_FUNC_NAME(VEC_NAME, squaredMagnitude)(const VEC_TYPE* vec)


#define CORAL_MATH_VEC_BASIC_OPERATIONS_BASE(VEC_TYPE, VEC_NAME, ELEMENT_TYPE)      \
    CORAL_MATH_VEC_OPERATION_XXX(VEC_TYPE, VEC_NAME, add);                          \
    CORAL_MATH_VEC_OPERATION_XXX(VEC_TYPE, VEC_NAME, sub);                          \
    CORAL_MATH_VEC_OPERATION_XXX(VEC_TYPE, VEC_NAME, mul);                          \
    CORAL_MATH_VEC_OPERATION_XXX(VEC_TYPE, VEC_NAME, div);                          \
    CORAL_MATH_VEC_OPERATION_XXX_SCALAR(VEC_TYPE, VEC_NAME, ELEMENT_TYPE, add);     \
    CORAL_MATH_VEC_OPERATION_XXX_SCALAR(VEC_TYPE, VEC_NAME, ELEMENT_TYPE, sub);     \
    CORAL_MATH_VEC_OPERATION_XXX_SCALAR(VEC_TYPE, VEC_NAME, ELEMENT_TYPE, mul);     \
    CORAL_MATH_VEC_OPERATION_XXX_SCALAR(VEC_TYPE, VEC_NAME, ELEMENT_TYPE, div);     \
    CORAL_MATH_VEC_OPERATION_DOT(VEC_TYPE, VEC_NAME, ELEMENT_TYPE);                 \
    CORAL_MATH_VEC_OPERATION_RAW_DOT(VEC_TYPE, VEC_NAME, ELEMENT_TYPE);             \
    CORAL_MATH_VEC_OPERATION_NORMALIZE(VEC_TYPE, VEC_NAME);                         \
    CORAL_MATH_VEC_OPERATION_MAGNITUDE(VEC_TYPE, VEC_NAME, ELEMENT_TYPE);           \
    CORAL_MATH_VEC_OPERATION_SQUARED_MAGNITUDE(VEC_TYPE, VEC_NAME, ELEMENT_TYPE);


#define CORAL_MATH_VEC_BASIC_OPERATIONS(DIMENSIONALITY, CORAL_MATH_TYPE_)                           \
    CORAL_MATH_VEC_BASIC_OPERATIONS_BASE(CORAL_MATH_VEC_GET_DEF(DIMENSIONALITY, CORAL_MATH_TYPE_),  \
                                         CORAL_MATH_VEC_GET_NAME(DIMENSIONALITY, CORAL_MATH_TYPE_), \
                                         CORAL_MATH_GET_TYPE_NAME(CORAL_MATH_TYPE_)                 \
    )



//- - - MATRIX OPERATIONS

#define CORAL_MATH_VEC_OPERATION_MUL_MAT__BASE(VEC_TYPE, VEC_NAME, MAT_TYPE, MAT_NAME_NO_ABBREV, RESULT_VEC_TYPE)           \
    CORAL_API void CORAL_MATH_VEC__GET_FUNC_NAME(VEC_NAME, mul_ ## MAT_NAME_NO_ABBREV)(const MAT_TYPE* mat,                 \
                                                                                       const VEC_TYPE* vec,                 \
                                                                                       RESULT_VEC_TYPE* restrict result     \
    )


#define CORAL_MATH_VEC_OPERATION_MUL_MAT_BASE(VEC_TYPE, VEC_NAME, MAT_TYPE, MAT_NAME_NO_ABBREV, RESULT_VEC_TYPE)            \
    CORAL_MATH_VEC_OPERATION_MUL_MAT__BASE(VEC_TYPE, VEC_NAME, MAT_TYPE, MAT_NAME_NO_ABBREV, RESULT_VEC_TYPE)


#define CORAL_MATH_VEC_OPERATION_MUL_MAT(COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY, CORAL_MATH_TYPE_)                           \
    CORAL_MATH_VEC_OPERATION_MUL_MAT_BASE(CORAL_MATH_VEC_GET_DEF(COLUMN_DIMENSIONALITY, CORAL_MATH_TYPE_),                      \
                                          CORAL_MATH_VEC_GET_NAME(COLUMN_DIMENSIONALITY, CORAL_MATH_TYPE_),                     \
                                          CORAL_MATH_MAT_GET_DEF(COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY, CORAL_MATH_TYPE_),  \
                                          CORAL_MATH_MAT_GET_NAME_NO_ABBREV(COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY),         \
                                          CORAL_MATH_VEC_GET_DEF(ROW_DIMENSIONALITY, CORAL_MATH_TYPE_)                          \
    );




//- - - SPECIAL OPERATIONS

#define CORAL_MATH_VEC_SPECIAL_OPERATION_CROSS_BASE(VEC_TYPE, VEC_NAME)                         \
    CORAL_API void CORAL_MATH_VEC__GET_FUNC_NAME(VEC_NAME, cross)(const VEC_TYPE* a,            \
                                                                  const VEC_TYPE* b,            \
                                                                  VEC_TYPE* result              \
    )


#define CORAL_MATH_VEC_SPECIAL_OPERATION_CROSS(CORAL_MATH_TYPE_)                                \
    CORAL_MATH_VEC_SPECIAL_OPERATION_CROSS_BASE(CORAL_MATH_VEC_GET_DEF(3, CORAL_MATH_TYPE_),    \
                                                CORAL_MATH_VEC_GET_NAME(3, CORAL_MATH_TYPE_)    \
    )


#define CORAL_MATH_VEC_SPECIAL_OPERATIONS(CORAL_MATH_TYPE_)                                     \
    CORAL_MATH_VEC_SPECIAL_OPERATION_CROSS(CORAL_MATH_TYPE_);




//- <

#define CORAL_MATH_VEC_ALL_OPERATIONS(CORAL_MATH_TYPE_)                                         \
    CORAL_MATH_VEC_FOR_EACH_DIMENSIONALITY(CORAL_MATH_VEC_BASIC_OPERATIONS, CORAL_MATH_TYPE_)   \
    CORAL_MATH_MAT_FOR_EACH_DIMENSIONALITY_FULL(CORAL_MATH_VEC_OPERATION_MUL_MAT,               \
                                                CORAL_MATH_TYPE_                                \
    )                                                                                           \
    CORAL_MATH_VEC_SPECIAL_OPERATIONS(CORAL_MATH_TYPE_)



//- DEFINITIONS

CORAL_MATH_FOR_EACH_TYPE(CORAL_MATH_VEC_ALL_OPERATIONS)



//- GENERICS

/*! \addtogroup CORAL_MATH_VEC_OPERATIONS
 *  @{
 */

/*! \name Generics
 *  @{
 */

/*! \cond EXLCUDE */

#define CORAL_MATH_VEC_GENERIC_SINGLE_ASSOCIATION(CORAL_MATH_TYPE_, DIMENSIONALITY, OPERATION_NAME) \
    CORAL_MATH_VEC_GET_DEF(DIMENSIONALITY, CORAL_MATH_TYPE_):                                       \
        CORAL_MATH_VEC_GET_FUNC_NAME(DIMENSIONALITY, CORAL_MATH_TYPE_, OPERATION_NAME),

/*! \endcond EXLCUDE */


/*! \brief Can be used to call a vector operation if the base type is not explicitly known. */
#define CORAL_MATH_VEC_GENERIC(DIMENSIONALITY, OPERATION_NAME, VEC_ARG0, ...)                       \
    _Generic(*(VEC_ARG0),                                                                           \
             CORAL_MATH_FOR_EACH_TYPE(CORAL_MATH_VEC_GENERIC_SINGLE_ASSOCIATION,                    \
                                      DIMENSIONALITY,                                               \
                                      OPERATION_NAME                                                \
             )                                                                                      \
             default: (void(*)(void*, ...))NULL                                                     \
    )(VEC_ARG0, ## __VA_ARGS__)


/*! \brief Can be used to call a vector operation if the base type and the dimensionality are not explicitly known. */
#define CORAL_MATH_VEC_FULL_GENERIC(OPERATION_NAME, VEC_ARG0, ...)                                  \
    _Generic(((int (*)[CORAL_ARRAY_LENGTH((VEC_ARG0)->raw)]){NULL}),                                \
             int (*)[2]: CORAL_MATH_VEC_GENERIC(2, OPERATION_NAME, VEC_ARG0, ## __VA_ARGS__),       \
             int (*)[3]: CORAL_MATH_VEC_GENERIC(3, OPERATION_NAME, VEC_ARG0, ## __VA_ARGS__),       \
             int (*)[4]: CORAL_MATH_VEC_GENERIC(4, OPERATION_NAME, VEC_ARG0, ## __VA_ARGS__)        \
    )


/*! \cond EXLCUDE */

#define CORAL_MATH_VEC_MUL_MAT_GENERIC_SINGLE_ASSOCIATION(CORAL_MATH_TYPE_, COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY)  \
        CORAL_MATH_MAT_GET_DEF(COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY, CORAL_MATH_TYPE_):                            \
            CORAL_MATH_VEC_GET_FUNC_NAME(COLUMN_DIMENSIONALITY,                                                         \
                                         CORAL_MATH_TYPE_,                                                              \
                                         mul_Mat ## COLUMN_DIMENSIONALITY ## x ## ROW_DIMENSIONALITY                    \
            ),

/*! \endcond EXLCUDE */


/*! \brief Can be used to multiply a vector with a matrix if the base type is not explicitly known. */
#define CORAL_MATH_VEC_MUL_MAT_GENERIC(COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY, MAT, VEC, RESULT) \
    _Generic(*(MAT),                                                                                \
             CORAL_MATH_FOR_EACH_TYPE(CORAL_MATH_VEC_MUL_MAT_GENERIC_SINGLE_ASSOCIATION,            \
                                      COLUMN_DIMENSIONALITY,                                        \
                                      ROW_DIMENSIONALITY                                            \
             )                                                                                      \
             default: (void (*)(void*, ...))NULL                                                    \
    )(MAT, VEC, RESULT)


/*! \cond EXLCUDE */

#define CORAL_MATH_VEC_MUL_MAT_FULL_GENERIC_ASSOCIATIONS_BASE(COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY, CORAL_MATH_TYPE_)  \
    CORAL_MATH_VEC_MUL_MAT_GENERIC_SINGLE_ASSOCIATION(CORAL_MATH_TYPE_, COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY)


#define CORAL_MATH_VEC_MUL_MAT_FULL_GENERIC_ASSOCIATIONS(CORAL_MATH_TYPE_)                                  \
    CORAL_MATH_MAT_FOR_EACH_DIMENSIONALITY_FULL(CORAL_MATH_VEC_MUL_MAT_FULL_GENERIC_ASSOCIATIONS_BASE,      \
                                                CORAL_MATH_TYPE_                                            \
    )

/*! \endcond EXCLUDE */


/*! \brief
 *      Can be used to multiply a vector with a matrix if the base type and
 *      the dimensionalities of the matrix are not explicitly known.
 */
#define CORAL_MATH_VEC_MUL_MAT_FULL_GENERIC(MAT, VEC, RESULT)                           \
    _Generic(*(MAT),                                                                    \
             CORAL_MATH_FOR_EACH_TYPE(CORAL_MATH_VEC_MUL_MAT_FULL_GENERIC_ASSOCIATIONS) \
    )(MAT, VEC, RESULT)

/*! @} */

/*! @} */

#endif
