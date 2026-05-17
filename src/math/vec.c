#include <stddef.h>
#include <tgmath.h>
#include "math/vec.h"
#include "math/vecTypes.h"
#include "math/matTypes.h"



// VECTORS

//- TEMPLATES

//- - OPERATIONS

//- - - BASIC OPERATIONS

#define CORAL_MATH_VEC_OPERATION_XXX_IMPL(VEC_TYPE, VEC_NAME, OPERATION_NAME, OPERATOR)     \
    CORAL_MATH_VEC_OPERATION_XXX(VEC_TYPE, VEC_NAME, OPERATION_NAME) {                      \
        for (size_t i = 0U; i < CORAL_ARRAY_LENGTH(result->raw); i++) {                     \
            result->raw[i] = a->raw[i] OPERATOR b->raw[i];                                  \
        }                                                                                   \
    }


#define CORAL_MATH_VEC_OPERATION_XXX_SCALAR_IMPL(VEC_TYPE, VEC_NAME, ELEMENT_TYPE, OPERATION_NAME, OPERATOR)    \
    CORAL_MATH_VEC_OPERATION_XXX_SCALAR(VEC_TYPE, VEC_NAME, ELEMENT_TYPE, OPERATION_NAME) {                     \
        for (size_t i = 0U; i < CORAL_ARRAY_LENGTH(result->raw); i++) {                                         \
            result->raw[i] = vec->raw[i] OPERATOR scalar;                                                       \
        }                                                                                                       \
    }


#define CORAL_MATH_VEC_OPERATION_DOT_IMPL(VEC_TYPE, VEC_NAME, ELEMENT_TYPE)                 \
    CORAL_MATH_VEC_OPERATION_DOT(VEC_TYPE, VEC_NAME, ELEMENT_TYPE) {                        \
        ELEMENT_TYPE sum = (ELEMENT_TYPE)0;                                                 \
        ELEMENT_TYPE squareSumA = (ELEMENT_TYPE)0;                                          \
        ELEMENT_TYPE squareSumB = (ELEMENT_TYPE)0;                                          \
        for (size_t i = 0U; i < CORAL_ARRAY_LENGTH(a->raw); i++) {                          \
            squareSumA += a->raw[i] * a->raw[i];                                            \
            squareSumB += b->raw[i] * b->raw[i];                                            \
            sum += a->raw[i] * b->raw[i];                                                   \
        }                                                                                   \
        return sum / (ELEMENT_TYPE)sqrt(squareSumA * squareSumB);                           \
    }


#define CORAL_MATH_VEC_OPERATION_RAW_DOT_IMPL(VEC_TYPE, VEC_NAME, ELEMENT_TYPE)             \
    CORAL_MATH_VEC_OPERATION_RAW_DOT(VEC_TYPE, VEC_NAME, ELEMENT_TYPE) {                    \
        ELEMENT_TYPE sum = (ELEMENT_TYPE)0;                                                 \
        for (size_t i = 0U; i < CORAL_ARRAY_LENGTH(a->raw); i++) {                          \
            sum += a->raw[i] * b->raw[i];                                                   \
        }                                                                                   \
        return sum;                                                                         \
    }


#define CORAL_MATH_VEC_OPERATION_NORMALIZE_IMPL(VEC_TYPE, VEC_NAME, ELEMENT_TYPE)           \
    CORAL_MATH_VEC_OPERATION_NORMALIZE(VEC_TYPE, VEC_NAME) {                                \
        ELEMENT_TYPE magnitude = VEC_NAME ## _magnitude(vec);                               \
        for (size_t i = 0U; i < CORAL_ARRAY_LENGTH(vec->raw); i++) {                        \
            result->raw[i] = vec->raw[i] / magnitude;                                       \
        }                                                                                   \
    }


#define CORAL_MATH_VEC_OPERATION_MAGNITUDE_IMPL(VEC_TYPE, VEC_NAME, ELEMENT_TYPE)           \
    CORAL_MATH_VEC_OPERATION_MAGNITUDE(VEC_TYPE, VEC_NAME, ELEMENT_TYPE) {                  \
        ELEMENT_TYPE squareSum = (ELEMENT_TYPE)0;                                           \
        for (size_t i = 0U; i < CORAL_ARRAY_LENGTH(vec->raw); i++) {                        \
            squareSum += vec->raw[i] * vec->raw[i];                                         \
        }                                                                                   \
        return (ELEMENT_TYPE)sqrt(squareSum);                                               \
    }


#define CORAL_MATH_VEC_OPERATION_SQUARED_MAGNITUDE_IMPL(VEC_TYPE, VEC_NAME, ELEMENT_TYPE)   \
    CORAL_MATH_VEC_OPERATION_SQUARED_MAGNITUDE(VEC_TYPE, VEC_NAME, ELEMENT_TYPE) {          \
        ELEMENT_TYPE squareSum = (ELEMENT_TYPE)0;                                           \
        for (size_t i = 0U; i < CORAL_ARRAY_LENGTH(vec->raw); i++) {                        \
            squareSum += vec->raw[i] * vec->raw[i];                                         \
        }                                                                                   \
        return squareSum;                                                                   \
    }


#define CORAL_MATH_VEC_BASIC_OPERATIONS_IMPL_BASE(VEC_TYPE, VEC_NAME, ELEMENT_TYPE)         \
    CORAL_MATH_VEC_OPERATION_XXX_IMPL(VEC_TYPE, VEC_NAME, add, +)                           \
    CORAL_MATH_VEC_OPERATION_XXX_IMPL(VEC_TYPE, VEC_NAME, sub, -)                           \
    CORAL_MATH_VEC_OPERATION_XXX_IMPL(VEC_TYPE, VEC_NAME, mul, *)                           \
    CORAL_MATH_VEC_OPERATION_XXX_IMPL(VEC_TYPE, VEC_NAME, div, /)                           \
    CORAL_MATH_VEC_OPERATION_XXX_SCALAR_IMPL(VEC_TYPE, VEC_NAME, ELEMENT_TYPE, add, +)      \
    CORAL_MATH_VEC_OPERATION_XXX_SCALAR_IMPL(VEC_TYPE, VEC_NAME, ELEMENT_TYPE, sub, -)      \
    CORAL_MATH_VEC_OPERATION_XXX_SCALAR_IMPL(VEC_TYPE, VEC_NAME, ELEMENT_TYPE, mul, *)      \
    CORAL_MATH_VEC_OPERATION_XXX_SCALAR_IMPL(VEC_TYPE, VEC_NAME, ELEMENT_TYPE, div, /)      \
    CORAL_MATH_VEC_OPERATION_DOT_IMPL(VEC_TYPE, VEC_NAME, ELEMENT_TYPE)                     \
    CORAL_MATH_VEC_OPERATION_RAW_DOT_IMPL(VEC_TYPE, VEC_NAME, ELEMENT_TYPE)                 \
    CORAL_MATH_VEC_OPERATION_MAGNITUDE_IMPL(VEC_TYPE, VEC_NAME, ELEMENT_TYPE)               \
    CORAL_MATH_VEC_OPERATION_SQUARED_MAGNITUDE_IMPL(VEC_TYPE, VEC_NAME, ELEMENT_TYPE)       \
    CORAL_MATH_VEC_OPERATION_NORMALIZE_IMPL(VEC_TYPE, VEC_NAME, ELEMENT_TYPE)


#define CORAL_MATH_VEC_BASIC_OPERATIONS_IMPL(DIMENSIONALITY, CORAL_MATH_TYPE_)                              \
    CORAL_MATH_VEC_BASIC_OPERATIONS_IMPL_BASE(CORAL_MATH_VEC_GET_DEF(DIMENSIONALITY, CORAL_MATH_TYPE_),     \
                                              CORAL_MATH_VEC_GET_NAME(DIMENSIONALITY, CORAL_MATH_TYPE_),    \
                                              CORAL_MATH_GET_TYPE_NAME(CORAL_MATH_TYPE_)                    \
    )



//- - - MATRIX OPERATIONS

// TODO: Implement fused multiplication and addition
#define CORAL_MATH_VEC_OPERATION_MUL_MAT_IMPL_BASE(VEC_TYPE, VEC_NAME, MAT_TYPE, MAT_NAME, RESULT_VEC_TYPE, RESULT_VEC_NAME)    \
    CORAL_MATH_VEC_OPERATION_MUL_MAT_BASE(VEC_TYPE, VEC_NAME, MAT_TYPE, MAT_NAME, RESULT_VEC_TYPE) {                            \
        /* Doing the first multiplication seperately because                    */                                              \
        /* adding this result to a zero initialized vector would be wasteful    */                                              \
        CORAL_MATH_VEC__GET_FUNC_NAME(RESULT_VEC_NAME, mul_scalar)(&mat->columnVecs[0U],                                        \
                                                                   vec->raw[0U],                                                \
                                                                   result                                                       \
        );                                                                                                                      \
                                                                                                                                \
        RESULT_VEC_TYPE tempMultiplicationResultVec;                                                                            \
        for (size_t i = 1U; i < CORAL_ARRAY_LENGTH(vec->raw); i++) {                                                            \
            CORAL_MATH_VEC__GET_FUNC_NAME(RESULT_VEC_NAME, mul_scalar)(&mat->columnVecs[i],                                     \
                                                                       vec->raw[i],                                             \
                                                                       &tempMultiplicationResultVec                             \
            );                                                                                                                  \
            CORAL_MATH_VEC__GET_FUNC_NAME(RESULT_VEC_NAME, add)(result,                                                         \
                                                                &tempMultiplicationResultVec,                                   \
                                                                result                                                          \
            );                                                                                                                  \
        }                                                                                                                       \
    }


#define CORAL_MATH_VEC_OPERATION_MUL_MAT_IMPL(COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY, CORAL_MATH_TYPE_)                          \
    CORAL_MATH_VEC_OPERATION_MUL_MAT_IMPL_BASE(CORAL_MATH_VEC_GET_DEF(COLUMN_DIMENSIONALITY, CORAL_MATH_TYPE_),                     \
                                               CORAL_MATH_VEC_GET_NAME(COLUMN_DIMENSIONALITY, CORAL_MATH_TYPE_),                    \
                                               CORAL_MATH_MAT_GET_DEF(COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY, CORAL_MATH_TYPE_), \
                                               CORAL_MATH_MAT_GET_NAME_NO_ABBREV(COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY),        \
                                               CORAL_MATH_VEC_GET_DEF(ROW_DIMENSIONALITY, CORAL_MATH_TYPE_),                        \
                                               CORAL_MATH_VEC_GET_NAME(ROW_DIMENSIONALITY, CORAL_MATH_TYPE_)                        \
    )



//- - - SPECIAL OPERATIONS

#define CORAL_MATH_VEC_SPECIAL_OPERATION_CROSS_IMPL_BASE(VEC_TYPE, VEC_NAME)                        \
    CORAL_MATH_VEC_SPECIAL_OPERATION_CROSS_BASE(VEC_TYPE, VEC_NAME) {                               \
        *result = ((VEC_TYPE){a->y * b->z - a->z * b->y,                                            \
                              a->z * b->x - a->x * b->z,                                            \
                              a->x * b->y - a->y * b->x                                             \
        });                                                                                         \
    }


#define CORAL_MATH_VEC_SPECIAL_OPERATION_CROSS_IMPL(CORAL_MATH_TYPE_)                               \
    CORAL_MATH_VEC_SPECIAL_OPERATION_CROSS_IMPL_BASE(CORAL_MATH_VEC_GET_DEF(3, CORAL_MATH_TYPE_),   \
                                                     CORAL_MATH_VEC_GET_NAME(3, CORAL_MATH_TYPE_)   \
    )


#define CORAL_MATH_VEC_SPECIAL_OPERATIONS_IMPL(CORAL_MATH_TYPE_)                                    \
    CORAL_MATH_VEC_SPECIAL_OPERATION_CROSS_IMPL(CORAL_MATH_TYPE_)



//- <

#define CORAL_MATH_VEC_ALL_OPERATIONS_IMPL(CORAL_MATH_TYPE_)                            \
    CORAL_MATH_VEC_BASIC_OPERATIONS_IMPL(2, CORAL_MATH_TYPE_)                           \
    CORAL_MATH_VEC_BASIC_OPERATIONS_IMPL(3, CORAL_MATH_TYPE_)                           \
    CORAL_MATH_VEC_BASIC_OPERATIONS_IMPL(4, CORAL_MATH_TYPE_)                           \
    CORAL_MATH_MAT_FOR_EACH_DIMENSIONALITY_FULL(CORAL_MATH_VEC_OPERATION_MUL_MAT_IMPL,  \
                                                CORAL_MATH_TYPE_                        \
    )                                                                                   \
    CORAL_MATH_VEC_SPECIAL_OPERATIONS_IMPL(CORAL_MATH_TYPE_)



//- IMPLEMENTATIONS

CORAL_MATH_FOR_EACH_TYPE(CORAL_MATH_VEC_ALL_OPERATIONS_IMPL)
