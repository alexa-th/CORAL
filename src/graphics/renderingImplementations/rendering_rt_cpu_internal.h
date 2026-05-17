#ifndef CORAL_GRAPHICS_RENDERING_RT_CPU
#define CORAL_GRAPHICS_RENDERING_RT_CPU

/*! \cond INTERNAL
 *  \file
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "graphics/math.h"
#include "graphics/object.h"
#include "graphics/scene.h"
#include "graphics/windowing.h"
#include "./rendering_rt_cpu_internal_bvh.h"



// RENDERING

//- DEFINITIONS

//- - SETTINGS

#define CORAL_GRAPHICS_RENDERING_THREAD_COUNT               (16U)
#define CORAL_GRAPHICS_RENDERING_BVH_NODE_PER_POOL_COUNT    (32U)

#define CORAL_GRAPHICS_RENDERING_SAMPLE_COUNT_HORIZONTAL    (16U)
#define CORAL_GRAPHICS_RENDERING_SAMPLE_COUNT_VERTICAL      (16U)
#define CORAL_GRAPHICS_RENDERING_SAMPLE_SKIP_COUNT          (7U)
#define CORAL_GRAPHICS_RENDERING_MAX_CHILD_RAY_COUNT        (8U)

#define CORAL_GRAPHICS_RENDERING_GAMMA_EXPONENT             (2.2)



//- - OBJECT FLAGS

#define CORAL_GRAPHICS_OBJECT_RENDERING_FLAG_IS_RENDERABLE      (0x10000000U)
#define CORAL_GRAPHICS_OBJECT_RENDERING_FLAG_IS_DRAWABLE        (0x20000000U)
#define CORAL_GRAPHICS_OBJECT_RENDERING_FLAG_IS_IN_BVH          (0x40000000U)
#define CORAL_GRAPHICS_OBJECT_RENDERING_FLAG_CAMERA_HAS_CHANGED (0x80000000U)



//- - DRAWING INFO

typedef struct {
    Vec3_t* highPrecisionPixelBuffer;
    uint32_t accumulatedSamples;
    uint32_t skippedPixels;
    bool cameraIsPerspective;

    union {
        struct {
            PerspectiveCameraObject_t* cameraObject;
            Vec4_t worldPosition;
            Vec4_t basePixelWorldDirection;
            Vec4_t pixelWorldDirectionDeltaX;   /*!< \note 'X' refers to the screen space X coordinates. */
            Vec4_t pixelWorldDirectionDeltaY;   /*!< \note 'Y' refers to the screen space Y coordinates. */
            Float_t sensorWidth;
            Float_t sensorHeight;
        } perspectiveCamera;

        struct {
            OrthographicCameraObject_t* cameraObject;
            Vec4_t worldDirection;
            Vec4_t basePixelWorldPosition;
            Vec4_t pixelWorldPositionDeltaX;    /*!< \note See pixelWorldDirectionDeltaX. */
            Vec4_t pixelWorldPositionDeltaY;    /*!< \note See pixelWorldDirectionDeltaY. */
        } orthographicCamera;
    };
} DrawingInfo_t;



//- - WINDOW DRAWING RESOURCE

typedef struct {
    Framebuffer_t framebuffer;
    DrawingInfo_t drawingInfo;
    const Scene_t* lastTargetedScene;
} WindowDrawingResource_t;



//- - RENDERING COMPONENT

struct RenderingComponent {
    BVHNode_t bvhNode;
    const MaterialComponent_t* material;
};



//- - SCENE DRAWING RESOURCE

typedef struct {
    BVH_t bvh;
    bool sceneHasChanged;
} SceneDrawingResource_t;



//- FUNCTIONS

//- - RENDERING COMPONENT

RenderingComponent_ot* Rendering_getRenderingComponentFromObject(const Base3DObject_t* object);

/*! \endcond INTERNAL */

#endif
