#ifndef CORAL_MATH_VEC_TYPES
#define CORAL_MATH_VEC_TYPES

/*! \file */

#include "../common/utils.h"
#include "./baseTypes.h"



// VECTOR TYPES

/*! \defgroup CORAL_MATH_VEC_TYPES Vector types
 *  \ingroup CORAL_MATH_VEC
 *  \brief Types representing vectors of different dimensionalities and of different base types.
 *
 *  Vectors are represented using the \p VecNT_u unions, where \p N is the dimensionality and \p T is the abbreviation
 *  of the base type of the vector.
 *
 *  A vector of a certain dimensionality has an amount of elements equal to the dimensionality and the same amount of
 *  the <tt>x, y, z, w</tt> members, i.e., \p Vec4T_u has the <tt>x, y, z, w</tt> members while \p Vec2T_u only has the
 *  <tt>x, y</tt> members. Additionally, each dimensionality has its own specific member pairs, for example, \p Vec2T_u
 *  has the <tt>u, v</tt> members to represent texture coordinates. These additional member pairs line up with and
 *  therefore only alias the <tt>x, y, z, w</tt> members.
 *
 *  Elements of vectors may also be accessed using the \p raw member, an array aliasing the <tt>x, y, z, w</tt> members.
 */



//- ITERATION HELPERS

#ifdef __cplusplus

#   define CORAL_MATH_VEC_FOR_EACH_DIMENSIONALITY(FUNCTION, ...)    \
        FUNCTION(2 __VA_OPT__(,) __VA_ARGS__)                       \
        FUNCTION(3 __VA_OPT__(,) __VA_ARGS__)                       \
        FUNCTION(4 __VA_OPT__(,) __VA_ARGS__)

#else

#   define CORAL_MATH_VEC_FOR_EACH_DIMENSIONALITY(FUNCTION, ...)    \
        FUNCTION(2, ## __VA_ARGS__)                                 \
        FUNCTION(3, ## __VA_ARGS__)                                 \
        FUNCTION(4, ## __VA_ARGS__)

#endif



//- TEMPLATES

#define CORAL_MATH_VEC2_TYPEDEF(TYPE, ABBREVIATION)     \
    typedef union {                                     \
        TYPE raw[2U];                                   \
                                                        \
        struct {                                        \
            TYPE x;                                     \
            TYPE y;                                     \
        };                                              \
                                                        \
        struct {                                        \
            TYPE u;                                     \
            TYPE v;                                     \
        };                                              \
    } Vec2 ## ABBREVIATION ## _u;


#define CORAL_MATH_VEC3_TYPEDEF(TYPE, ABBREVIATION)     \
    typedef union {                                     \
        TYPE raw[3U];                                   \
                                                        \
        struct {                                        \
            TYPE x;                                     \
            TYPE y;                                     \
            TYPE z;                                     \
        };                                              \
                                                        \
        struct {                                        \
            TYPE b;                                     \
            TYPE g;                                     \
            TYPE r;                                     \
        };                                              \
                                                        \
        struct {                                        \
            TYPE pitch;                                 \
            TYPE yaw;                                   \
            TYPE roll;                                  \
        };                                              \
    } Vec3 ## ABBREVIATION ## _u;


#define CORAL_MATH_VEC4_TYPEDEF(TYPE, ABBREVIATION)     \
    typedef union {                                     \
        TYPE raw[4U];                                   \
                                                        \
        struct {                                        \
            TYPE x;                                     \
            TYPE y;                                     \
            TYPE z;                                     \
            TYPE w;                                     \
        };                                              \
                                                        \
        struct {                                        \
            TYPE b;                                     \
            TYPE g;                                     \
            TYPE r;                                     \
            TYPE a;                                     \
        };                                              \
                                                        \
        struct {                                        \
            TYPE real;                                  \
            TYPE i;                                     \
            TYPE j;                                     \
            TYPE k;                                     \
        };                                              \
    } Vec4 ## ABBREVIATION ## _u;



//- <

#define CORAL_MATH_VEC_TYPEDEFS_BASE(TYPE, ABBREVIATION)    \
    CORAL_MATH_VEC2_TYPEDEF(TYPE, ABBREVIATION)             \
    CORAL_MATH_VEC3_TYPEDEF(TYPE, ABBREVIATION)             \
    CORAL_MATH_VEC4_TYPEDEF(TYPE, ABBREVIATION)


#define CORAL_MATH_VEC_TYPEDEFS(CORAL_MATH_TYPE_)                                   \
    CORAL_MATH_VEC_TYPEDEFS_BASE(CORAL_MATH_GET_TYPE_NAME(CORAL_MATH_TYPE_),        \
                                 CORAL_MATH_GET_TYPE_ABBREVIATION(CORAL_MATH_TYPE_) \
    )



//- DEFINITIONS

/*! \cond EXCLUDE */

CORAL_MATH_FOR_EACH_TYPE(CORAL_MATH_VEC_TYPEDEFS)

/*! \endcond EXCLUDE */



//- - CONVINIENCE DEFINITIONS

/*! \addtogroup CORAL_MATH_VEC_TYPES
 *  @{
 */

/*! \name Convinience definitions
 *  @{
 */

#define CORAL_MATH_VEC_GET_NAME_FROM_ABBREV(DIMENSIONALITY, ABBREVIATION)   \
    Vec ## DIMENSIONALITY ## ABBREVIATION


#define CORAL_MATH_VEC_GET_DEF_FROM_ABBREV(DIMENSIONALITY, ABBREVIATION)    \
    Vec ## DIMENSIONALITY ## ABBREVIATION ## _u


#define CORAL_MATH_VEC_GET_NAME(DIMENSIONALITY, CORAL_MATH_TYPE_)           \
    CORAL__EXPAND(CORAL_MATH_VEC_GET_NAME_FROM_ABBREV,                      \
                  DIMENSIONALITY,                                           \
                  CORAL_MATH_GET_TYPE_ABBREVIATION(CORAL_MATH_TYPE_)        \
    )


#define CORAL_MATH_VEC_GET_DEF(DIMENSIONALITY, CORAL_MATH_TYPE_)            \
    CORAL__EXPAND(CORAL_MATH_VEC_GET_DEF_FROM_ABBREV,                       \
                  DIMENSIONALITY,                                           \
                  CORAL_MATH_GET_TYPE_ABBREVIATION(CORAL_MATH_TYPE_)        \
    )


#define CORAL_MATH_VEC_GET_DIMENSIONALITY_FROM_DEF(VEC_TYPE) (sizeof(VEC_TYPE) / sizeof(((VEC_TYPE*)0)->x))

/*! @} */

/*! @} */

#endif
