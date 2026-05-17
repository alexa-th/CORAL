#include <stdint.h>
#include <tgmath.h>
#include <string.h>
#include "common/utils.h"
#include "math/definitions.h"
#include "math/vec.h"
#include "math/mat.h"
#include "graphics/math.h"



// MATH

//- FUNCTIONS

void Graphics_get3DTransformationMatrix(Mat4_t* restrict transformMatrix,
                                        const Mat4_t* restrict baseMatrix,
                                        const Vec3_t* translation,
                                        const Vec3_t* rotation,
                                        const Vec3_t* scale,
                                        uint32_t rotationOrder
) {
    Mat4_t localTransformMatrix;

    Float_t alpha = (rotation->pitch * CORAL_MATH_PI) / 180.0;
    Float_t beta = (rotation->yaw * CORAL_MATH_PI) / 180.0;
    Float_t gamma = (rotation->roll * CORAL_MATH_PI) / 180.0;

    Float_t sin_alpha = sin(alpha);
    Float_t cos_alpha = cos(alpha);
    Float_t sin_beta = sin(beta);
    Float_t cos_beta = cos(beta);
    Float_t sin_gamma = sin(gamma);
    Float_t cos_gamma = cos(gamma);

    switch (rotationOrder) {
        case CORAL_GRAPHICS_ROTATION_ORDER_EULER_XYZ:
            localTransformMatrix = ((Mat4_t){.raw = {cos_beta * cos_gamma,
                                                     cos_beta * sin_gamma,
                                                     -sin_beta,
                                                     0.0,
                                                     sin_alpha * sin_beta * cos_gamma - cos_alpha * sin_gamma,
                                                     sin_alpha * sin_beta * sin_gamma + cos_alpha * cos_gamma,
                                                     sin_alpha * cos_beta,
                                                     0.0,
                                                     cos_alpha * sin_beta * cos_gamma + sin_alpha * sin_gamma,
                                                     cos_alpha * sin_beta * sin_gamma - sin_alpha * cos_gamma,
                                                     cos_alpha * cos_beta,
                                                     0.0
            }});
            break;

        case CORAL_GRAPHICS_ROTATION_ORDER_EULER_YXZ:
            localTransformMatrix = ((Mat4_t){.raw = {cos_beta * cos_gamma - sin_alpha * sin_beta * sin_gamma,
                                                     cos_beta * sin_gamma + sin_alpha * sin_beta * cos_gamma,
                                                     -cos_alpha * sin_beta,
                                                     0.0,
                                                     -cos_alpha * sin_gamma,
                                                     cos_alpha * cos_gamma,
                                                     sin_alpha,
                                                     0.0,
                                                     sin_beta * cos_gamma + sin_alpha * cos_beta * sin_gamma,
                                                     sin_beta * sin_gamma - sin_alpha * cos_beta * cos_gamma,
                                                     cos_alpha * cos_beta,
                                                     0.0
            }});
            break;

        case CORAL_GRAPHICS_ROTATION_ORDER_EULER_ZXY:
            localTransformMatrix = ((Mat4_t){.raw = {cos_beta * cos_gamma + sin_alpha * sin_beta * sin_gamma,
                                                     cos_alpha * sin_gamma,
                                                     sin_alpha * cos_beta * sin_gamma - sin_beta * cos_gamma,
                                                     0.0,
                                                     sin_alpha * sin_beta * cos_gamma - cos_beta * sin_gamma,
                                                     cos_alpha * cos_gamma,
                                                     sin_alpha * cos_beta * cos_gamma + sin_beta * sin_gamma,
                                                     0.0,
                                                     cos_alpha * sin_beta,
                                                     -sin_alpha,
                                                     cos_alpha * cos_beta,
                                                     0.0
            }});
            break;

        default:
            coral_assert_base("Unrecognized rotation order.", __FILE__, __LINE__);
    }

    localTransformMatrix.columnVecs[3U] = ((Vec4_t){translation->x,
                                                    translation->y,
                                                    translation->z,
                                                    1.0
    });

    for (size_t i = 0U; i < 3U; i++) {
        CORAL_MATH_VEC_GENERIC(4,
                               mul_scalar,
                               &localTransformMatrix.columnVecs[i],
                               scale->raw[i],
                               &localTransformMatrix.columnVecs[i]
        );
    }

    if (baseMatrix) { CORAL_MATH_MAT_GENERIC(4, 4, mul, &localTransformMatrix, baseMatrix, transformMatrix); }
    else { memcpy(transformMatrix, &localTransformMatrix, sizeof(Mat4_t)); }
}
