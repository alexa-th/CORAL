#ifndef CORAL_GRAPHICS_MATH
#define CORAL_GRAPHICS_MATH

/*! \file */

#include <stdint.h>
#include "../common/api.h"
#include "../math/baseTypes.h"
#include "../math/vecTypes.h"
#include "../math/matTypes.h"



// MATH

/*! \defgroup CORAL_GRAPHICS_MATH Math
 *  \ingroup CORAL_GRAPHICS
 *  \brief Mathematical types and definitions used by the graphics sub-module.
 *  \addtogroup CORAL_GRAPHICS_MATH
 *  @{
 */



//- DEFINITIONS

//- - DEFAULT TYPES

/*! \def CORAL_GRAPHICS_FLOAT_TYPE
 *  \brief The math sub-module definition of the standard floating-point type used by the graphics sub-module.
 */

#ifndef CORAL_GRAPHICS_FLOAT_TYPE
#   define CORAL_GRAPHICS_FLOAT_TYPE CORAL_MATH_TYPE_FLOAT
#endif


/*! \brief The standard floating-point type of the graphics sub-module. */
typedef CORAL_MATH_GET_TYPE_NAME(CORAL_GRAPHICS_FLOAT_TYPE) Float_t;


/*! @{ */

/*! \brief Standard floating-point vector type. */
typedef CORAL_MATH_VEC_GET_DEF(2, CORAL_GRAPHICS_FLOAT_TYPE) Vec2_t;
typedef CORAL_MATH_VEC_GET_DEF(3, CORAL_GRAPHICS_FLOAT_TYPE) Vec3_t;
typedef CORAL_MATH_VEC_GET_DEF(4, CORAL_GRAPHICS_FLOAT_TYPE) Vec4_t;

/*! @} */


/*! @{ */

/*! \brief Standard floating-point square matrix type. */
typedef CORAL_MATH_MAT_GET_DEF(2, 2, CORAL_GRAPHICS_FLOAT_TYPE) Mat2_t;
typedef CORAL_MATH_MAT_GET_DEF(3, 3, CORAL_GRAPHICS_FLOAT_TYPE) Mat3_t;
typedef CORAL_MATH_MAT_GET_DEF(4, 4, CORAL_GRAPHICS_FLOAT_TYPE) Mat4_t;

/*! @} */



//- - ROTATION ORDERINGS

/*! \defgroup CORAL_GRAPHICS_ROTATION_ORDER_ Rotation ordering
 *  \addtogroup CORAL_GRAPHICS_ROTATION_ORDER_
 *  @{
 */

/*! \brief The ranges in which the different rotation orderings are defined. */
#define CORAL_GRAPHICS_ROTATION_ORDER_BIT_RANGE (0x00000FC0U)

/*! \brief Rotates in <tt>X -> Y -> Z</tt> order. */
#define CORAL_GRAPHICS_ROTATION_ORDER_EULER_XYZ (0x00000040U)

/*! \brief Rotates in <tt>X -> Z -> Y</tt> order. */
#define CORAL_GRAPHICS_ROTATION_ORDER_EULER_XZY (0x00000080U)

/*! \brief Rotates in <tt>Y -> X -> Z</tt> order. */
#define CORAL_GRAPHICS_ROTATION_ORDER_EULER_YXZ (0x00000100U)

/*! \brief Rotates in <tt>Y -> Z -> X</tt> order. */
#define CORAL_GRAPHICS_ROTATION_ORDER_EULER_YZX (0x00000200U)

/*! \brief Rotates in <tt>Z -> X -> Y</tt> order. */
#define CORAL_GRAPHICS_ROTATION_ORDER_EULER_ZXY (0x00000400U)

/*! \brief Rotates in <tt>Z -> Y -> X</tt> order. */
#define CORAL_GRAPHICS_ROTATION_ORDER_EULER_ZYX (0x00000800U)

/*! @} */



//- FUNCTIONS

/*! \brief Calculates the matrix for the transformation described by \p translation, \p rotation, and \p scale.
 *  \param[out] transformMatrix     Will contain the transformation matrix on return.
 *  \param[in]  baseMatrix          (Opt.) If provided, the transformation of \p baseMatrix will first be applied to the
 *                                  transformation matrix before it is stored in \p transformMatrix.
 *  \param[in]  rotationOrder       Describes the way in which \p rotation will be interpreted and applied to the
 *                                  transformation; for all possible values see \ref CORAL_GRAPHICS_ROTATION_ORDER_ "Rotation ordering".
 */
CORAL_API void Graphics_get3DTransformationMatrix(Mat4_t* restrict transformMatrix,
                                                  const Mat4_t* restrict baseMatrix,
                                                  const Vec3_t* translation,
                                                  const Vec3_t* rotation,
                                                  const Vec3_t* scale,
                                                  uint32_t rotationOrder
);

/*! @} */

#endif
