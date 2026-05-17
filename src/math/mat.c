#include <stddef.h>
#include "common/error.h"
#include "common/utils.h"
#include "math/mat.h"
#include "math/matTypes.h"
#include "math/vec.h"
#include "math/vecTypes.h"



// MATRICES

//- DEFINITIONS

/*! \brief Computes the determinant of a three dimensional matrix.
 *
 *  The matrix is represented as follows:
 *      | A B |
 *      | C D |
 */
#define CORAL_MATH_MAT_DETERMINANT_MAT2(A, B, C, D) ((A) * (D) - (B) * (C))


/*! \brief Computes the determinant of a three dimensional matrix.
 *
 *  The matrix is represented as follows:
 *      | A B C |
 *      | D E F |
 *      | G H I |
 */
#define CORAL_MATH_MAT_DETERMINANT_MAT3(A, B, C, D, E, F, G, H, I)                      \
    ((A) * CORAL_MATH_MAT_DETERMINANT_MAT2(E, F, H, I) -                                \
     (B) * CORAL_MATH_MAT_DETERMINANT_MAT2(D, F, G, I) +                                \
     (C) * CORAL_MATH_MAT_DETERMINANT_MAT2(D, E, G, H)                                  \
    )


/*! \brief Computes the determinant of a four dimensional matrix.
 *
 *  The matrix is represented as follows:
 *      | A B C D |
 *      | E F G H |
 *      | I J K L |
 *      | M N O P |
 */
#define CORAL_MATH_MAT_DETERMINANT_MAT4(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P) \
    ((A) * CORAL_MATH_MAT_DETERMINANT_MAT3(F, G, H, J, K, L, N, O, P) -                 \
     (B) * CORAL_MATH_MAT_DETERMINANT_MAT3(E, G, H, I, K, L, M, O, P) +                 \
     (C) * CORAL_MATH_MAT_DETERMINANT_MAT3(E, F, H, I, J, L, M, N, P) -                 \
     (D) * CORAL_MATH_MAT_DETERMINANT_MAT3(E, F, G, I, J, K, M, N, O)                   \
    )



//- TEMPLATES

//- - OPERATIONS

//- - - ANY DIMENSIONALITY

#define CORAL_MATH_MAT_OPERATION_MUL_IMPL(MAT_TYPE, MAT_NAME, MAT_NAME_NO_ABBREV, COLUMN_VEC_NAME)                  \
    CORAL_MATH_MAT_OPERATION_MUL(MAT_TYPE, MAT_NAME) {                                                              \
        for (size_t i = 0U; i < CORAL_ARRAY_LENGTH(result->columnVecs); i++) {                                      \
            CORAL_MATH_VEC__GET_FUNC_NAME(COLUMN_VEC_NAME, mul_ ## MAT_NAME_NO_ABBREV)(lhs,                         \
                                                                                       &rhs->columnVecs[i],         \
                                                                                       &result->columnVecs[i]       \
            );                                                                                                      \
        }                                                                                                           \
    }


#define CORAL_MATH_MAT_OPERATION_TRANSPOSE_IMPL(MAT_TYPE, MAT_NAME, TRANSPOSED_MAT_TYPE)                            \
    CORAL_MATH_MAT_OPERATION_TRANSPOSE(MAT_TYPE, MAT_NAME, TRANSPOSED_MAT_TYPE) {                                   \
        for (size_t resultColumn = 0U; resultColumn < CORAL_ARRAY_LENGTH(result->columnVecs); resultColumn++) {     \
            for (size_t resultRow = 0U; resultRow < CORAL_ARRAY_LENGTH(result->columnVecs[0U].raw); resultRow++) {  \
                result->columnVecs[resultColumn].raw[resultRow] = mat->columnVecs[resultRow].raw[resultColumn];     \
            }                                                                                                       \
        }                                                                                                           \
    }


#define CORAL_MATH_MAT_CxR_OPERATIONS_IMPL_BASE(MAT_TYPE, MAT_NAME, MAT_NAME_NO_ABBREV, TRANSPOSED_MAT_TYPE, COLUMN_VEC_NAME)   \
    CORAL_MATH_MAT_OPERATION_MUL_IMPL(MAT_TYPE, MAT_NAME, MAT_NAME_NO_ABBREV, COLUMN_VEC_NAME)                                  \
    CORAL_MATH_MAT_OPERATION_TRANSPOSE_IMPL(MAT_TYPE, MAT_NAME, TRANSPOSED_MAT_TYPE)


#define CORAL_MATH_MAT_CxR_OPERATIONS_IMPL(COLUMN_DIMENSIONALITY, ROW_DIMENSIONALITY, CORAL_MATH_TYPE_) \
    CORAL_MATH_MAT_CxR_OPERATIONS_IMPL_BASE(CORAL_MATH_MAT_GET_DEF(COLUMN_DIMENSIONALITY,               \
                                                                   ROW_DIMENSIONALITY,                  \
                                                                   CORAL_MATH_TYPE_                     \
                                            ),                                                          \
                                            CORAL_MATH_MAT_GET_NAME(COLUMN_DIMENSIONALITY,              \
                                                                    ROW_DIMENSIONALITY,                 \
                                                                    CORAL_MATH_TYPE_                    \
                                            ),                                                          \
                                            CORAL_MATH_MAT_GET_NAME_NO_ABBREV(COLUMN_DIMENSIONALITY,    \
                                                                              ROW_DIMENSIONALITY        \
                                            ),                                                          \
                                            CORAL_MATH_MAT_GET_DEF(ROW_DIMENSIONALITY,                  \
                                                                   COLUMN_DIMENSIONALITY,               \
                                                                   CORAL_MATH_TYPE_                     \
                                            ),                                                          \
                                            CORAL_MATH_VEC_GET_NAME(ROW_DIMENSIONALITY,                 \
                                                                    CORAL_MATH_TYPE_                    \
                                            )                                                           \
    )



//- - - SQUARE DIMENSIONALITY

//- - - - TWO DIMENSIONAL

#define CORAL_MATH_MAT_SQUARE_OPERATION_INVERT_MAT2_IMPL(MAT_TYPE, MAT_NAME, ELEMENT_TYPE)              \
    CORAL_MATH_MAT_SQUARE_OPERATION_INVERT(MAT_TYPE, MAT_NAME) {                                        \
        const ELEMENT_TYPE a = mat->columnVecs[0U].raw[0U];                                             \
        const ELEMENT_TYPE b = mat->columnVecs[1U].raw[0U];                                             \
        const ELEMENT_TYPE c = mat->columnVecs[0U].raw[1U];                                             \
        const ELEMENT_TYPE d = mat->columnVecs[1U].raw[1U];                                             \
        const ELEMENT_TYPE determinant = CORAL_MATH_MAT_DETERMINANT_MAT2(a, b, c, d);                   \
        if (!determinant) { return CORAL_ERROR_INVALID_ARGS; }                                          \
        *result = ((MAT_TYPE){.raw = {d / determinant,                                                  \
                                      -c / determinant,                                                 \
                                      -b / determinant,                                                 \
                                      a / determinant                                                   \
        }});                                                                                            \
        return CORAL_ERROR_NONE;                                                                        \
    }


#define CORAL_MATH_MAT_SQUARE_OPERATION_DETERMINANT_MAT2_IMPL(MAT_TYPE, MAT_NAME, ELEMENT_TYPE)         \
    CORAL_MATH_MAT_SQUARE_OPERATION_DETERMINANT(MAT_TYPE, MAT_NAME, ELEMENT_TYPE) {                     \
        return CORAL_MATH_MAT_DETERMINANT_MAT2(mat->columnVecs[0U].raw[0U],                             \
                                               mat->columnVecs[1U].raw[0U],                             \
                                               mat->columnVecs[0U].raw[1U],                             \
                                               mat->columnVecs[1U].raw[1U]                              \
        );                                                                                              \
    }



//- - - - THREE DIMENSIONAL

#define CORAL_MATH_MAT_SQUARE_OPERATION_INVERT_MAT3_IMPL(MAT_TYPE, MAT_NAME, ELEMENT_TYPE)              \
    CORAL_MATH_MAT_SQUARE_OPERATION_INVERT(MAT_TYPE, MAT_NAME) {                                        \
        const ELEMENT_TYPE a = mat->columnVecs[0U].raw[0U];                                             \
        const ELEMENT_TYPE b = mat->columnVecs[1U].raw[0U];                                             \
        const ELEMENT_TYPE c = mat->columnVecs[2U].raw[0U];                                             \
        const ELEMENT_TYPE d = mat->columnVecs[0U].raw[1U];                                             \
        const ELEMENT_TYPE e = mat->columnVecs[1U].raw[1U];                                             \
        const ELEMENT_TYPE f = mat->columnVecs[2U].raw[1U];                                             \
        const ELEMENT_TYPE g = mat->columnVecs[0U].raw[2U];                                             \
        const ELEMENT_TYPE h = mat->columnVecs[1U].raw[2U];                                             \
        const ELEMENT_TYPE i = mat->columnVecs[2U].raw[2U];                                             \
        const ELEMENT_TYPE determinant = CORAL_MATH_MAT_DETERMINANT_MAT3(a, b, c, d, e, f, g, h, i);    \
        if (!determinant) { return CORAL_ERROR_INVALID_ARGS; }                                          \
        *result = ((MAT_TYPE){.raw = {CORAL_MATH_MAT_DETERMINANT_MAT2(e, f, h, i) / determinant,        \
                                      -CORAL_MATH_MAT_DETERMINANT_MAT2(d, f, g, i) / determinant,       \
                                      CORAL_MATH_MAT_DETERMINANT_MAT2(d, e, g, h) / determinant,        \
                                      -CORAL_MATH_MAT_DETERMINANT_MAT2(b, c, h, i) / determinant,       \
                                      CORAL_MATH_MAT_DETERMINANT_MAT2(a, c, g, i) / determinant,        \
                                      -CORAL_MATH_MAT_DETERMINANT_MAT2(a, b, g, h) / determinant,       \
                                      CORAL_MATH_MAT_DETERMINANT_MAT2(b, c, e, f) / determinant,        \
                                      -CORAL_MATH_MAT_DETERMINANT_MAT2(a, c, d, f) / determinant,       \
                                      CORAL_MATH_MAT_DETERMINANT_MAT2(a, b, d, e) / determinant,        \
        }});                                                                                            \
        return CORAL_ERROR_NONE;                                                                        \
    }


#define CORAL_MATH_MAT_SQUARE_OPERATION_DETERMINANT_MAT3_IMPL(MAT_TYPE, MAT_NAME, ELEMENT_TYPE)         \
    CORAL_MATH_MAT_SQUARE_OPERATION_DETERMINANT(MAT_TYPE, MAT_NAME, ELEMENT_TYPE) {                     \
        return CORAL_MATH_MAT_DETERMINANT_MAT3(mat->columnVecs[0U].raw[0U],                             \
                                               mat->columnVecs[1U].raw[0U],                             \
                                               mat->columnVecs[2U].raw[0U],                             \
                                               mat->columnVecs[0U].raw[1U],                             \
                                               mat->columnVecs[1U].raw[1U],                             \
                                               mat->columnVecs[2U].raw[1U],                             \
                                               mat->columnVecs[0U].raw[2U],                             \
                                               mat->columnVecs[1U].raw[2U],                             \
                                               mat->columnVecs[2U].raw[2U]                              \
        );                                                                                              \
    }



//- - - - FOUR DIMENSIONAL


#define CORAL_MATH_MAT_SQUARE_OPERATION_INVERT_MAT4_IMPL(MAT_TYPE, MAT_NAME, ELEMENT_TYPE)              \
    CORAL_MATH_MAT_SQUARE_OPERATION_INVERT(MAT_TYPE, MAT_NAME) {                                        \
        const ELEMENT_TYPE a = mat->columnVecs[0U].raw[0U];                                             \
        const ELEMENT_TYPE b = mat->columnVecs[1U].raw[0U];                                             \
        const ELEMENT_TYPE c = mat->columnVecs[2U].raw[0U];                                             \
        const ELEMENT_TYPE d = mat->columnVecs[3U].raw[0U];                                             \
        const ELEMENT_TYPE e = mat->columnVecs[0U].raw[1U];                                             \
        const ELEMENT_TYPE f = mat->columnVecs[1U].raw[1U];                                             \
        const ELEMENT_TYPE g = mat->columnVecs[2U].raw[1U];                                             \
        const ELEMENT_TYPE h = mat->columnVecs[3U].raw[1U];                                             \
        const ELEMENT_TYPE i = mat->columnVecs[0U].raw[2U];                                             \
        const ELEMENT_TYPE j = mat->columnVecs[1U].raw[2U];                                             \
        const ELEMENT_TYPE k = mat->columnVecs[2U].raw[2U];                                             \
        const ELEMENT_TYPE l = mat->columnVecs[3U].raw[2U];                                             \
        const ELEMENT_TYPE m = mat->columnVecs[0U].raw[3U];                                             \
        const ELEMENT_TYPE n = mat->columnVecs[1U].raw[3U];                                             \
        const ELEMENT_TYPE o = mat->columnVecs[2U].raw[3U];                                             \
        const ELEMENT_TYPE p = mat->columnVecs[3U].raw[3U];                                             \
        ELEMENT_TYPE determinant =                                                                      \
            CORAL_MATH_MAT_DETERMINANT_MAT4(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);            \
        if (!determinant) { return CORAL_ERROR_INVALID_ARGS; }                                          \
        *result = ((MAT_TYPE){.raw = {CORAL_MATH_MAT_DETERMINANT_MAT3(f, g, h, j, k, l, n, o, p) /      \
                                      determinant,                                                      \
                                      -CORAL_MATH_MAT_DETERMINANT_MAT3(e, g, h, i, k, l, m, o, p) /     \
                                      determinant,                                                      \
                                      CORAL_MATH_MAT_DETERMINANT_MAT3(e, f, h, i, j, l, m, n, p) /      \
                                      determinant,                                                      \
                                      -CORAL_MATH_MAT_DETERMINANT_MAT3(e, f, g, i, j, k, m, n, o) /     \
                                      determinant,                                                      \
                                      -CORAL_MATH_MAT_DETERMINANT_MAT3(b, c, d, j, k, l, n, o, p) /     \
                                      determinant,                                                      \
                                      CORAL_MATH_MAT_DETERMINANT_MAT3(a, c, d, i, k, l, m, o, p) /      \
                                      determinant,                                                      \
                                      -CORAL_MATH_MAT_DETERMINANT_MAT3(a, b, d, i, j, l, m, n, p) /     \
                                      determinant,                                                      \
                                      CORAL_MATH_MAT_DETERMINANT_MAT3(a, b, c, i, j, k, m, n, o) /      \
                                      determinant,                                                      \
                                      CORAL_MATH_MAT_DETERMINANT_MAT3(b, c, d, f, g, h, n, o, p) /      \
                                      determinant,                                                      \
                                      -CORAL_MATH_MAT_DETERMINANT_MAT3(a, c, d, e, g, h, m, o, p) /     \
                                      determinant,                                                      \
                                      CORAL_MATH_MAT_DETERMINANT_MAT3(a, b, d, e, f, h, m, n, p) /      \
                                      determinant,                                                      \
                                      -CORAL_MATH_MAT_DETERMINANT_MAT3(a, b, c, e, f, g, m, n, o) /     \
                                      determinant,                                                      \
                                      -CORAL_MATH_MAT_DETERMINANT_MAT3(b, c, d, f, g, h, j, k, l) /     \
                                      determinant,                                                      \
                                      CORAL_MATH_MAT_DETERMINANT_MAT3(a, c, d, e, g, h, i, k, l) /      \
                                      determinant,                                                      \
                                      -CORAL_MATH_MAT_DETERMINANT_MAT3(a, b, d, e, f, h, i, j, l) /     \
                                      determinant,                                                      \
                                      CORAL_MATH_MAT_DETERMINANT_MAT3(a, b, c, e, f, g, i, j, k) /      \
                                      determinant                                                       \
        }});                                                                                            \
        return CORAL_ERROR_NONE;                                                                        \
    }

#define CORAL_MATH_MAT_SQUARE_OPERATION_DETERMINANT_MAT4_IMPL(MAT_TYPE, MAT_NAME, ELEMENT_TYPE)         \
    CORAL_MATH_MAT_SQUARE_OPERATION_DETERMINANT(MAT_TYPE, MAT_NAME, ELEMENT_TYPE) {                     \
        return CORAL_MATH_MAT_DETERMINANT_MAT4(mat->columnVecs[0U].raw[0U],                             \
                                               mat->columnVecs[1U].raw[0U],                             \
                                               mat->columnVecs[2U].raw[0U],                             \
                                               mat->columnVecs[3U].raw[0U],                             \
                                               mat->columnVecs[0U].raw[1U],                             \
                                               mat->columnVecs[1U].raw[1U],                             \
                                               mat->columnVecs[2U].raw[1U],                             \
                                               mat->columnVecs[3U].raw[1U],                             \
                                               mat->columnVecs[0U].raw[2U],                             \
                                               mat->columnVecs[1U].raw[2U],                             \
                                               mat->columnVecs[2U].raw[2U],                             \
                                               mat->columnVecs[3U].raw[2U],                             \
                                               mat->columnVecs[0U].raw[3U],                             \
                                               mat->columnVecs[1U].raw[3U],                             \
                                               mat->columnVecs[2U].raw[3U],                             \
                                               mat->columnVecs[3U].raw[3U]                              \
        );                                                                                              \
    }


#define CORAL_MATH_MAT_SQUARE_OPERATIONS_IMPL_BASE(DIMENSIONALITY, MAT_TYPE, MAT_NAME, ELEMENT_TYPE)    \
    CORAL_MATH_MAT_SQUARE_OPERATION_INVERT_MAT ##                                                       \
    DIMENSIONALITY ##                                                                                   \
    _IMPL(MAT_TYPE, MAT_NAME, ELEMENT_TYPE)                                                             \
    CORAL_MATH_MAT_SQUARE_OPERATION_DETERMINANT_MAT ##                                                  \
    DIMENSIONALITY ##                                                                                   \
    _IMPL(MAT_TYPE, MAT_NAME, ELEMENT_TYPE)


#define CORAL_MATH_MAT_SQUARE_OPERATIONS_IMPL(DIMENSIONALITY, CORAL_MATH_TYPE_)             \
    CORAL_MATH_MAT_SQUARE_OPERATIONS_IMPL_BASE(DIMENSIONALITY,                              \
                                               CORAL_MATH_MAT_GET_DEF(DIMENSIONALITY,       \
                                                                      DIMENSIONALITY,       \
                                                                      CORAL_MATH_TYPE_      \
                                               ),                                           \
                                               CORAL_MATH_MAT_GET_NAME(DIMENSIONALITY,      \
                                                                  DIMENSIONALITY,           \
                                                                  CORAL_MATH_TYPE_          \
                                               ),                                           \
                                               CORAL_MATH_GET_TYPE_NAME(CORAL_MATH_TYPE_)   \
    )



//- - <

#define CORAL_MATH_MAT_ALL_OPERATIONS_IMPL(CORAL_MATH_TYPE_)        \
    CORAL_MATH_MAT_CxR_OPERATIONS_IMPL(2, 2, CORAL_MATH_TYPE_)      \
    CORAL_MATH_MAT_CxR_OPERATIONS_IMPL(3, 3, CORAL_MATH_TYPE_)      \
    CORAL_MATH_MAT_CxR_OPERATIONS_IMPL(4, 4, CORAL_MATH_TYPE_)      \
    CORAL_MATH_MAT_SQUARE_OPERATIONS_IMPL(2, CORAL_MATH_TYPE_)      \
    CORAL_MATH_MAT_SQUARE_OPERATIONS_IMPL(3, CORAL_MATH_TYPE_)      \
    CORAL_MATH_MAT_SQUARE_OPERATIONS_IMPL(4, CORAL_MATH_TYPE_)



//- IMPLEMENTATIONS

CORAL_MATH_FOR_EACH_TYPE(CORAL_MATH_MAT_ALL_OPERATIONS_IMPL)
