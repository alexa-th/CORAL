#ifdef CORAL_GRAPHICS_RENDERING_IMPL_RT_CPU

#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <tgmath.h>
#include <assert.h>
#include "common/random.h"
#include "common/utils.h"
#include "memory/dynalloc.h"
#include "concurrency/threadDefinition.h"
#include "concurrency/threadPool.h"
#include "math/definitions.h"
#include "math/vec.h"
#include "graphics/math.h"
#include "graphics/mesh.h"
#include "graphics/object.h"
#include "graphics/scene.h"
#include "graphics/windowing.h"
#include "graphics/rendering.h"
#include "graphics/rendering_internal.h"
#include "graphics/renderingImplementations/rendering_rt_cpu_internal_bvh.h"
#include "graphics/renderingImplementations/rendering_rt_cpu_internal_ray.h"
#include "graphics/renderingImplementations/rendering_rt_cpu_internal.h"



// RENDERING

//- GLOBALS

static DrawingInfo_t* g_drawingInfo;
static Framebuffer_t* g_targetFramebuffer;
static const Scene_t* g_targetScene;
static bool g_drawingToWindowFramebuffer;

static ThreadPool_t g_renderingThreadPool;
static Thread_ot g_renderingThreads[CORAL_GRAPHICS_RENDERING_THREAD_COUNT];



//- STARTUP & SHUTDOWN

void Rendering_startup(void) {
    ThreadPool_init(&g_renderingThreadPool, g_renderingThreads, CORAL_GRAPHICS_RENDERING_THREAD_COUNT);
}


void Rendering_shutdown(void) {
    ThreadPool_destr(&g_renderingThreadPool);
}



//- CALLBACKS

//- - WINDOWING

void Rendering_onWindowInit(Window_t* window) {
    WindowDrawingResource_t* drawingResource = CORAL_malloc(sizeof(WindowDrawingResource_t));
    window->drawingResource = (uintptr_t)drawingResource;
    memset(drawingResource, 0, sizeof(WindowDrawingResource_t));

    Framebuffer_init(&drawingResource->framebuffer, window);
    size_t pixelBufferSize = drawingResource->framebuffer.width * drawingResource->framebuffer.height;
    memset(drawingResource->framebuffer.pixelBuffer, 0U, pixelBufferSize * sizeof(Vec4b_u));

    drawingResource->drawingInfo.highPrecisionPixelBuffer = CORAL_malloc(pixelBufferSize * sizeof(Vec3_t));
    CORAL_ASSERT(drawingResource->drawingInfo.highPrecisionPixelBuffer,
                 "Failed to allocate a window's highPrecisionPixelBuffer."
    );
}


void Rendering_onWindowDestr(Window_t* window) {
    WindowDrawingResource_t* drawingResource = (WindowDrawingResource_t*)window->drawingResource;
    window->drawingResource = 0U;

    if (drawingResource) {
        Framebuffer_destr(&drawingResource->framebuffer);
        CORAL_free(drawingResource->drawingInfo.highPrecisionPixelBuffer);
        CORAL_free(drawingResource);
    }
}


void Rendering_onWindowResize(Window_t* window, size_t newWidth, size_t newHeight) {
    WindowDrawingResource_t* drawingResource = (WindowDrawingResource_t*)window->drawingResource;

    if (drawingResource) {
        Framebuffer__resize(&drawingResource->framebuffer, newWidth, newHeight);
        size_t pixelBufferSize = drawingResource->framebuffer.width * drawingResource->framebuffer.height;

        if (pixelBufferSize) {
            memset(drawingResource->framebuffer.pixelBuffer, 0U, pixelBufferSize * sizeof(Vec4b_u));

            Vec3_t* newHighPrecisionPixelBuffer = CORAL_realloc(drawingResource->drawingInfo.highPrecisionPixelBuffer,
                                                                pixelBufferSize * sizeof(Vec3_t)
            );

            CORAL_ASSERT(newHighPrecisionPixelBuffer, "Failed to resize a window's highPrecisionPixelBuffer.");

            drawingResource->drawingInfo.highPrecisionPixelBuffer = newHighPrecisionPixelBuffer;
        }
        else if (drawingResource->drawingInfo.highPrecisionPixelBuffer) {
            CORAL_free(drawingResource->drawingInfo.highPrecisionPixelBuffer);
            drawingResource->drawingInfo.highPrecisionPixelBuffer = NULL;
        }

        drawingResource->drawingInfo.accumulatedSamples = 0U;
        drawingResource->drawingInfo.skippedPixels = 0U;
    }
}



//- - MESHES

void Rendering_onMeshInit(Mesh_t* mesh) { }
void Rendering_onMeshDestr(Mesh_t* mesh) { }



//- - OBJECTS

void Rendering_onObjectInit(BaseObject_t* object, RenderingComponent_ot* renderingComponent) {
    if (!renderingComponent) { return; }

    switch (object->type) {
        case CORAL_GRAPHICS_OBJECT_TYPE_SPHERE:
        case CORAL_GRAPHICS_OBJECT_TYPE_MESH:
            {
                object->flags |= CORAL_GRAPHICS_OBJECT_RENDERING_FLAG_IS_RENDERABLE;

                Base3DObject_t* object3DBase = CORAL_TO_PARENT_PTR(Base3DObject_t, base, object);
                renderingComponent->bvhNode.additionalLeafNodeData = (uintptr_t)object3DBase;
                return;
            }

        default:
            return;
    }
}


void Rendering_onObjectDestr(BaseObject_t* object, RenderingComponent_ot* renderingComponent) { }


bool Rendering_onObjectSetActiveMaterial(BaseObject_t* object,
                                         RenderingComponent_ot* renderingComponent,
                                         const MaterialComponent_t* materialComponent
) {
    if (!(object->flags & CORAL_GRAPHICS_OBJECT_RENDERING_FLAG_IS_RENDERABLE)) { return false; }

    renderingComponent->material = materialComponent;

    if (materialComponent) {
        CORAL_ASSERT(materialComponent->diffuseColour.a == 255U || materialComponent->refractiveIndex > 0.0,
                     "Negative or zero refractiveIndex on transparent material."
        );

        object->flags |= CORAL_GRAPHICS_OBJECT_RENDERING_FLAG_IS_DRAWABLE;
    }
    else {
        object->flags &= ~CORAL_GRAPHICS_OBJECT_RENDERING_FLAG_IS_DRAWABLE;
    }

    return true;
}


SizeAndAlignment_t Rendering_getRenderingComponentSizeAndAlignment(void) {
    return ((SizeAndAlignment_t){sizeof(RenderingComponent_ot), alignof(RenderingComponent_ot)});
}



//- - SCENE

void Rendering_onSceneInit(Scene_t* scene) {
    SceneDrawingResource_t* sceneDrawingResource = CORAL_malloc(sizeof(SceneDrawingResource_t));
    sceneDrawingResource->sceneHasChanged = false;
    BVH_init(&sceneDrawingResource->bvh, CORAL_GRAPHICS_RENDERING_BVH_NODE_PER_POOL_COUNT);

    scene->drawingResource = (uintptr_t)sceneDrawingResource;
}


void Rendering_onSceneDestr(Scene_t* scene) {
    SceneDrawingResource_t* sceneDrawingResource = (SceneDrawingResource_t*)scene->drawingResource;

    if (sceneDrawingResource) {
        BVH_destr(&sceneDrawingResource->bvh);
        CORAL_free(sceneDrawingResource);
        scene->drawingResource = 0U;
    }
}


void Rendering_onObjectAdditionToScene(Scene_t* scene, Base3DObject_t* object, SceneComponent_t* sceneComponent) {
    const uint32_t requiredFlags = CORAL_GRAPHICS_OBJECT_RENDERING_FLAG_IS_RENDERABLE |
                                   CORAL_GRAPHICS_OBJECT_RENDERING_FLAG_IS_DRAWABLE
    ;

    if (!CORAL_GRAPHICS_OBJECT_HAS_FLAGS(object->base.flags, requiredFlags)) { return; }

    SceneDrawingResource_t* sceneDrawingResource = (void*)scene->drawingResource;
    RenderingComponent_ot* renderingComponent = Rendering_getRenderingComponentFromObject(object);

    BoundingBox_init_sceneObject(&renderingComponent->bvhNode.boundingBox, object);

    if (sceneDrawingResource) {
        BVH_insert(&sceneDrawingResource->bvh, &renderingComponent->bvhNode);
        object->base.flags |= CORAL_GRAPHICS_OBJECT_RENDERING_FLAG_IS_IN_BVH;
    }
}


void Rendering_onObjectRemovalFromScene(Scene_t* scene, Base3DObject_t* object, SceneComponent_t* sceneComponent) {
    if (!(object->base.flags & CORAL_GRAPHICS_OBJECT_RENDERING_FLAG_IS_IN_BVH)) { return; }

    SceneDrawingResource_t* sceneDrawingResource = (void*)scene->drawingResource;

    if (sceneDrawingResource) {
        RenderingComponent_ot* renderingComponent = Rendering_getRenderingComponentFromObject(object);
        BVH_remove(&sceneDrawingResource->bvh, &renderingComponent->bvhNode);
        object->base.flags &= ~CORAL_GRAPHICS_OBJECT_RENDERING_FLAG_IS_IN_BVH;
    }
}


void Rendering_onObjectAttributesChanged(Scene_t* scene, Base3DObject_t* object, SceneComponent_t* sceneComponent) {
    bool isCamera = object->base.type == CORAL_GRAPHICS_OBJECT_TYPE_PERSPECTIVE_CAMERA ||
                    object->base.type == CORAL_GRAPHICS_OBJECT_TYPE_ORTHOGRAPHIC_CAMERA
    ;

    if (isCamera) {
        object->base.flags |= CORAL_GRAPHICS_OBJECT_RENDERING_FLAG_CAMERA_HAS_CHANGED;
    }
    else if (object->base.flags & CORAL_GRAPHICS_OBJECT_RENDERING_FLAG_IS_RENDERABLE) {
        Rendering_onObjectRemovalFromScene(scene, object, sceneComponent);
        Rendering_onObjectAdditionToScene(scene, object, sceneComponent);
    }
    else { return; }

    ((SceneDrawingResource_t*)scene->drawingResource)->sceneHasChanged = true;
}



//- FUNCTIONS

//- - RENDERING COMPONENT

RenderingComponent_ot* Rendering_getRenderingComponentFromObject(const Base3DObject_t* object) {
    ComponentClassElement_t* renderingComponentElement;
    renderingComponentElement = ComponentClass_getComponent(object->base.componentClass,
                                                            CORAL_GRAPHICS_COMPONENT_TYPE_RENDERING,
                                                            NULL
    );

    RenderingComponent_ot* renderingComponent = (void*)((uintptr_t)object +
                                                        object->base.offsetToComponents +
                                                        renderingComponentElement->offsetFromComponents
    );

    return renderingComponent;
}



//- - CAMERA HANDLING

static void Rendering_setCameraInfo(DrawingInfo_t* drawingInfo, const Base3DObject_t* cameraBase) {
    switch (cameraBase->base.type) {
        case CORAL_GRAPHICS_OBJECT_TYPE_PERSPECTIVE_CAMERA:
            {
                PerspectiveCameraObject_t* camera = CORAL_TO_PARENT_PTR(PerspectiveCameraObject_t, base3D, cameraBase);
                camera->base3D.base.flags &= ~CORAL_GRAPHICS_OBJECT_RENDERING_FLAG_CAMERA_HAS_CHANGED;

                drawingInfo->cameraIsPerspective = true;
                drawingInfo->perspectiveCamera.cameraObject = camera;

                Float_t sensorWidth = 2.0 * tan(camera->horizontalFOV * CORAL_MATH_PI / 360.0) * camera->distanceToNearPlane;
                Float_t sensorHeight = sensorWidth / camera->aspectRatio;

                drawingInfo->perspectiveCamera.sensorWidth = sensorWidth;
                drawingInfo->perspectiveCamera.sensorHeight = sensorHeight;

                Vec4_t basePixelDirection = {-sensorWidth / 2.0, sensorHeight / 2.0, -camera->distanceToNearPlane, 0.0};
                Vec4_t pixelDirectionDeltaX = {sensorWidth / (Float_t)g_targetFramebuffer->width, 0.0, 0.0, 0.0};
                Vec4_t pixelDirectionDeltaY = {0.0, -sensorHeight / (Float_t)g_targetFramebuffer->height, 0.0, 0.0};

                Mat4_t cameraTransformMatrix;
                Scene_getObjectTransformationMatrix(cameraBase, &cameraTransformMatrix);

                // Coordinates of the pixels are then transformed to the world coordinate system and the basePixel's
                // position is subtracted from both to get the world deltas of the pixels

                CORAL_MATH_VEC_MUL_MAT_GENERIC(4,
                                               4,
                                               &cameraTransformMatrix,
                                               &basePixelDirection,
                                               &drawingInfo->perspectiveCamera.basePixelWorldDirection
                );

                CORAL_MATH_VEC_MUL_MAT_GENERIC(4,
                                               4,
                                               &cameraTransformMatrix,
                                               &pixelDirectionDeltaX,
                                               &drawingInfo->perspectiveCamera.pixelWorldDirectionDeltaX
                );

                CORAL_MATH_VEC_MUL_MAT_GENERIC(4,
                                               4,
                                               &cameraTransformMatrix,
                                               &pixelDirectionDeltaY,
                                               &drawingInfo->perspectiveCamera.pixelWorldDirectionDeltaY
                );

                // Camera is at (0, 0, 0) in its own coordinate system, so a transformation to the world coordinate system
                // would be just the translation, which is exactly the fourth column vector of the tranformation matrix
                drawingInfo->perspectiveCamera.worldPosition = cameraTransformMatrix.columnVecs[3U];
                break;
            }

        case CORAL_GRAPHICS_OBJECT_TYPE_ORTHOGRAPHIC_CAMERA:
            {
                OrthographicCameraObject_t* camera = CORAL_TO_PARENT_PTR(OrthographicCameraObject_t, base3D, cameraBase);
                camera->base3D.base.flags &= ~CORAL_GRAPHICS_OBJECT_RENDERING_FLAG_CAMERA_HAS_CHANGED;

                drawingInfo->cameraIsPerspective = false;
                drawingInfo->orthographicCamera.cameraObject = camera;

                Mat4_t cameraTransformMatrix;
                Scene_getObjectTransformationMatrix(cameraBase, &cameraTransformMatrix);

                Vec4_t direction = {0.0, 0.0, -1.0, 0.0};
                Vec4_t basePixelPosition = {-camera->width / 2.0, camera->height / 2.0, 0.0, 1.0};
                Vec4_t pixelPositionDeltaX = {camera->width / (Float_t)g_targetFramebuffer->width, 0.0, 0.0, 0.0};
                Vec4_t pixelPositionDeltaY = {0.0, -camera->height / (Float_t)g_targetFramebuffer->height, 0.0, 0.0};

                CORAL_MATH_VEC_MUL_MAT_GENERIC(4,
                                               4,
                                               &cameraTransformMatrix,
                                               &direction,
                                               &drawingInfo->orthographicCamera.worldDirection
                );

                CORAL_MATH_VEC_MUL_MAT_GENERIC(4,
                                               4,
                                               &cameraTransformMatrix,
                                               &basePixelPosition,
                                               &drawingInfo->orthographicCamera.basePixelWorldPosition
                );

                CORAL_MATH_VEC_MUL_MAT_GENERIC(4,
                                               4,
                                               &cameraTransformMatrix,
                                               &pixelPositionDeltaX,
                                               &drawingInfo->orthographicCamera.pixelWorldPositionDeltaX
                );

                CORAL_MATH_VEC_MUL_MAT_GENERIC(4,
                                               4,
                                               &cameraTransformMatrix,
                                               &pixelPositionDeltaY,
                                               &drawingInfo->orthographicCamera.pixelWorldPositionDeltaY
                );
                break;
            }

        default:
            CORAL_ASSERT(0, "Renderer was given a camera object which is not a camera.");
    }
}


static void Rendering_getCameraRay(Ray_t* ray,
                                   const DrawingInfo_t* drawingInfo,
                                   uint32_t pixelXPosition,
                                   uint32_t pixelYPosition,
                                   uint32_t sampleIndex
) {
    size_t sampleXPosition = sampleIndex % CORAL_GRAPHICS_RENDERING_SAMPLE_COUNT_HORIZONTAL;
    size_t sampleYPosition = sampleIndex / CORAL_GRAPHICS_RENDERING_SAMPLE_COUNT_HORIZONTAL;

    Float_t pixelFactorX = (Float_t)pixelXPosition +
                           (Float_t)(sampleXPosition + 1U) /
                           (Float_t)(CORAL_GRAPHICS_RENDERING_SAMPLE_COUNT_HORIZONTAL + 1U)
    ;

    Float_t pixelFactorY = (Float_t)pixelYPosition +
                           (Float_t)(sampleYPosition + 1U) /
                           (Float_t)(CORAL_GRAPHICS_RENDERING_SAMPLE_COUNT_VERTICAL + 1U)
    ;

    if (drawingInfo->cameraIsPerspective) {
        ray->origin = drawingInfo->perspectiveCamera.worldPosition;
        ray->direction = drawingInfo->perspectiveCamera.basePixelWorldDirection;

        Vec4_t directionDeltaX = drawingInfo->perspectiveCamera.pixelWorldDirectionDeltaX;

        CORAL_MATH_VEC_GENERIC(4,
                               mul_scalar,
                               &directionDeltaX,
                               pixelFactorX,
                               &directionDeltaX
        );

        CORAL_MATH_VEC_GENERIC(4,
                               add,
                               &ray->direction,
                               &directionDeltaX,
                               &ray->direction
        );

        Vec4_t directionDeltaY = drawingInfo->perspectiveCamera.pixelWorldDirectionDeltaY;

        CORAL_MATH_VEC_GENERIC(4,
                               mul_scalar,
                               &directionDeltaY,
                               pixelFactorY,
                               &directionDeltaY
        );

        CORAL_MATH_VEC_GENERIC(4,
                               add,
                               &ray->direction,
                               &directionDeltaY,
                               &ray->direction
        );
    }
    else {
        ray->origin = drawingInfo->orthographicCamera.basePixelWorldPosition;
        ray->direction = drawingInfo->orthographicCamera.worldDirection;

        Vec4_t positionDeltaX = drawingInfo->orthographicCamera.pixelWorldPositionDeltaX;

        CORAL_MATH_VEC_GENERIC(4,
                               mul_scalar,
                               &positionDeltaX,
                               pixelFactorX,
                               &positionDeltaX
        );

        CORAL_MATH_VEC_GENERIC(4,
                               add,
                               &ray->origin,
                               &positionDeltaX,
                               &ray->origin
        );

        Vec4_t positionDeltaY = drawingInfo->orthographicCamera.pixelWorldPositionDeltaY;

        CORAL_MATH_VEC_GENERIC(4,
                               mul_scalar,
                               &positionDeltaY,
                               pixelFactorY,
                               &positionDeltaY
        );

        CORAL_MATH_VEC_GENERIC(4,
                               add,
                               &ray->origin,
                               &positionDeltaY,
                               &ray->origin
        );
    }
}



//- - RANDOM DIRECTIONS

static Vec4_t Rendering_getRandomUnitDirectionVec(void) {
    Vec4_t randomVec = {.raw = {[3U] = 0.0}};
    Float_t randomVecSquaredMagnitude;

    do {
        for (size_t i = 0U; i < 3U; i++) {
            randomVec.raw[i] = (Float_t)Random__double(-1.0, 1.0);
        }
        randomVecSquaredMagnitude = CORAL_MATH_VEC_GENERIC(4, squaredMagnitude, &randomVec);
    } while (randomVecSquaredMagnitude > 1.0);

    CORAL_MATH_VEC_GENERIC(4, div_scalar, &randomVec, sqrt(randomVecSquaredMagnitude), &randomVec);
    return randomVec;
}



//- - BVH-NODE INTERSECTION

static bool Rendering_bvhNodeIntersectFunc(const Ray_t* ray,
                                           RayIntersectInfo_t* intersectInfo,
                                           const BVH_t* bvh,
                                           const BVHNode_t* bvhNode,
                                           uintptr_t context
) {
    const bool allowInternalIntersect = context;
    const Base3DObject_t* object = (void*)bvhNode->additionalLeafNodeData;
    RayIntersectInfo_t localIntersectInfo;

    if (Ray_intersect_sceneObject(ray, &localIntersectInfo, object) &&
        (allowInternalIntersect || !localIntersectInfo.isInternalIntersect) &&
        (localIntersectInfo.entryDirectionFactor > 0.0001)
    ) {
        *intersectInfo = localIntersectInfo;
        return true;
    }

    return false;
}



//- <

static Vec3_t Rendering_sampleScene(const Ray_t* ray, size_t recursiveDepth) {
    SceneDrawingResource_t* const sceneDrawingResource = (void*)g_targetScene->drawingResource;
    Vec3_t returnColour;

    if (recursiveDepth >= CORAL_GRAPHICS_RENDERING_MAX_CHILD_RAY_COUNT) { goto failure_noHit; }

    RayIntersectInfo_t intersectInfo = {CORAL_MATH_GET_TYPE_MAX(CORAL_GRAPHICS_FLOAT_TYPE), 0.0};
    BVHNode_t* hitNode = Ray_intersect_bvh(ray,
                                           &intersectInfo,
                                           &sceneDrawingResource->bvh,
                                           &Rendering_bvhNodeIntersectFunc,
                                           recursiveDepth
    );

    if (!hitNode) { goto failure_noHit; }

    Vec4_t scaledHitNormal;
    CORAL_MATH_VEC_GENERIC(4, mul_scalar, &intersectInfo.entryNormalVec, 0.001, &scaledHitNormal);

    const MaterialComponent_t* const materialComponent =
        CORAL_TO_PARENT_PTR(RenderingComponent_ot, bvhNode, hitNode)->material;

    bool isRefracted = (materialComponent->diffuseColour.a == 0U) ||
                       (materialComponent->diffuseColour.a != 255U &&
                        Random__unsigned(1U, 255U) > materialComponent->diffuseColour.a
                       )
    ;

    if (isRefracted) { goto refract; }

    bool isReflected = (materialComponent->specularIntensity == 255U) ||
                       (materialComponent->specularIntensity &&
                        Random__unsigned(1U, 255U) <= materialComponent->specularIntensity
                       )
    ;

    if (isReflected) { goto reflect; }
    else { goto scatter; }

scatter:
    {
        Ray_t scatteredRay = {.direction = Rendering_getRandomUnitDirectionVec()};

        CORAL_MATH_VEC_GENERIC(4,
                               add,
                               &scatteredRay.direction,
                               &intersectInfo.entryNormalVec,
                               &scatteredRay.direction
        );

        if (CORAL_MATH_VEC_GENERIC(4, squaredMagnitude, &scatteredRay.direction) < 0.0001) {
            scatteredRay.direction = intersectInfo.entryNormalVec;
        }

        Ray_getPositionFromDirectionFactor(ray, intersectInfo.entryDirectionFactor, &scatteredRay.origin);
        CORAL_MATH_VEC_GENERIC(4, add, &scatteredRay.origin, &scaledHitNormal, &scatteredRay.origin);

        returnColour = Rendering_sampleScene(&scatteredRay, recursiveDepth + 1U);
        Vec3_t diffuseColour = {.r = (Float_t)materialComponent->diffuseColour.r / 255.0,
                                .g = (Float_t)materialComponent->diffuseColour.g / 255.0,
                                .b = (Float_t)materialComponent->diffuseColour.b / 255.0
        };

        CORAL_MATH_VEC_GENERIC(3, mul, &returnColour, &diffuseColour, &returnColour);
        goto success;
    }

reflect:
    {
        Ray_t reflectedRay;
        Ray_reflect(ray, intersectInfo.entryDirectionFactor, &intersectInfo.entryNormalVec, &reflectedRay);

        if (materialComponent->roughness) {
            Vec4_t fuzzVec = Rendering_getRandomUnitDirectionVec();

            if (materialComponent->roughness != 255U) {
                CORAL_MATH_VEC_GENERIC(4,
                                       mul_scalar,
                                       &fuzzVec,
                                       (Float_t)materialComponent->roughness / 255.0,
                                       &fuzzVec
                );
            }

            CORAL_MATH_VEC_GENERIC(4,
                                   add,
                                   &reflectedRay.direction,
                                   &fuzzVec,
                                   &reflectedRay.direction
            );

            Float_t fuzziedDirectionVecRawDot = CORAL_MATH_VEC_GENERIC(4,
                                                                       rawDot,
                                                                       &reflectedRay.direction,
                                                                       &intersectInfo.entryNormalVec
            );

            if (fuzziedDirectionVecRawDot < 0.0) { goto failure_absorp; }
        }

        CORAL_MATH_VEC_GENERIC(4, add, &reflectedRay.origin, &scaledHitNormal, &reflectedRay.origin);

        returnColour = Rendering_sampleScene(&reflectedRay, recursiveDepth + 1U);
        Vec3_t specularColour = {.r = (Float_t)materialComponent->specularColour.r / 255.0,
                                 .g = (Float_t)materialComponent->specularColour.g / 255.0,
                                 .b = (Float_t)materialComponent->specularColour.b / 255.0
        };

        CORAL_MATH_VEC_GENERIC(3, mul, &returnColour, &specularColour, &returnColour);
        goto success;
    }

refract:
    {
        Ray_t refractedRay;
        Float_t refractiveIndexRatio;

        if (intersectInfo.isInternalIntersect) {
            refractiveIndexRatio = materialComponent->refractiveIndex;
        }
        else {
            refractiveIndexRatio = 1.0 / materialComponent->refractiveIndex;
        }

        bool shouldReflect = !Ray_refract(ray,
                                          intersectInfo.entryDirectionFactor,
                                          &intersectInfo.entryNormalVec,
                                          refractiveIndexRatio,
                                          &refractedRay
        );

        if (shouldReflect) { goto reflect; }

        CORAL_MATH_VEC_GENERIC(4, sub, &refractedRay.origin, &scaledHitNormal, &refractedRay.origin);

        returnColour = Rendering_sampleScene(&refractedRay, recursiveDepth + 1U);
        goto success;
    }

success:
    if (intersectInfo.isInternalIntersect) {
        Vec4_t scaledRayDirection;
        Float_t scaledRayDirectionMagnitude;

        CORAL_MATH_VEC_GENERIC(4, mul_scalar, &ray->direction, intersectInfo.entryDirectionFactor, &scaledRayDirection);
        scaledRayDirection.raw[3U] = 0.0;
        scaledRayDirectionMagnitude = CORAL_MATH_VEC_GENERIC(4, magnitude, &scaledRayDirection);

        Vec3_t rayMediumColour = {.r = (Float_t)materialComponent->diffuseColour.r / 255.0,
                                  .g = (Float_t)materialComponent->diffuseColour.g / 255.0,
                                  .b = (Float_t)materialComponent->diffuseColour.b / 255.0
        };

        for (size_t i = 0U; i < 3U; i++) {
            returnColour.raw[i] *= exp((rayMediumColour.raw[i] - 1.0) * scaledRayDirectionMagnitude);
        }
    }

    if (materialComponent->emissionStrength) {
        for (size_t i = 0U; i < 3U; i++) {
            returnColour.raw[i] += ((Float_t)materialComponent->emissionColour.raw[i] / 255.0) * materialComponent->emissionStrength;
        }
    }

    return returnColour;

failure_noHit:
    for (size_t i = 0U; i < 3U; i++) {
        returnColour.raw[i] = (Float_t)g_targetScene->backgroundColour.raw[i] / 255.0;
    }

    return returnColour;

failure_absorp:
    return ((Vec3_t){0.0, 0.0, 0.0});
}


static uintptr_t Rendering_drawScanline(uintptr_t scanline) {
    const size_t offsetToScanlineBegin = scanline * g_targetFramebuffer->width;
    const uint32_t samplesPerPixel = (!g_drawingToWindowFramebuffer) ?
                                     (CORAL_GRAPHICS_RENDERING_SAMPLE_COUNT_HORIZONTAL *
                                      CORAL_GRAPHICS_RENDERING_SAMPLE_COUNT_VERTICAL
                                     ) :
                                     (1U)
    ;

    Vec4b_u currentPixelColour = ((Vec4b_u){.r = 0U, .g = 0U, .b = 0U, .a = 255U});

    for (size_t i = 0U; i < g_targetFramebuffer->width; i++) {
        if (g_drawingToWindowFramebuffer &&
            ((i + g_drawingInfo->skippedPixels) % (CORAL_GRAPHICS_RENDERING_SAMPLE_SKIP_COUNT + 1U))
        ) {
            if (!g_drawingInfo->accumulatedSamples && !g_drawingInfo->skippedPixels) {
                g_targetFramebuffer->pixelBuffer[offsetToScanlineBegin + i] = currentPixelColour;
            }
            continue;
        }

        Vec3_t additionalHighPrecisionPixelColour = ((Vec3_t){0.0, 0.0, 0.0});

        for (size_t sampleIndex = 0U; sampleIndex < samplesPerPixel; sampleIndex++) {
            Ray_t currentRay;
            Rendering_getCameraRay(&currentRay, g_drawingInfo, i, scanline, g_drawingInfo->accumulatedSamples + sampleIndex);

            Vec3_t currentSampleColour = Rendering_sampleScene(&currentRay, 0U);
            CORAL_MATH_VEC_GENERIC(3,
                                   add,
                                   &additionalHighPrecisionPixelColour,
                                   &currentSampleColour,
                                   &additionalHighPrecisionPixelColour
            );
        }

        Vec3_t currentHighPrecisionLinearPixelColour;

        if (!g_drawingInfo->accumulatedSamples) {
            currentHighPrecisionLinearPixelColour = ((Vec3_t){0.0, 0.0, 0.0});
        }
        else if (g_drawingInfo->highPrecisionPixelBuffer) {
            currentHighPrecisionLinearPixelColour = g_drawingInfo->highPrecisionPixelBuffer[offsetToScanlineBegin + i];
        }
        else {
            currentPixelColour = g_targetFramebuffer->pixelBuffer[offsetToScanlineBegin + i];

            for (size_t j = 0U; j < 3U; j++) {
                currentHighPrecisionLinearPixelColour.raw[j] = (Float_t)currentPixelColour.raw[j] / 255.0;
            }
        }

        Vec3_t currentHighPrecisionPixelColour;

        for (size_t j = 0U; j < 3U; j++) {
            currentHighPrecisionLinearPixelColour.raw[j] += additionalHighPrecisionPixelColour.raw[j];
            currentHighPrecisionPixelColour.raw[j] = pow(currentHighPrecisionLinearPixelColour.raw[j] /
                                                         (Float_t)(g_drawingInfo->accumulatedSamples + samplesPerPixel),
                                                         1.0 / (Float_t)CORAL_GRAPHICS_RENDERING_GAMMA_EXPONENT
            );
        }

        currentPixelColour = ((Vec4b_u){.r = CORAL_MIN((unsigned)(currentHighPrecisionPixelColour.r * 255.0), 255U),
                                        .g = CORAL_MIN((unsigned)(currentHighPrecisionPixelColour.g * 255.0), 255U),
                                        .b = CORAL_MIN((unsigned)(currentHighPrecisionPixelColour.b * 255.0), 255U),
                                        .a = 255U
        });

        g_targetFramebuffer->pixelBuffer[offsetToScanlineBegin + i] = currentPixelColour;

        if (g_drawingInfo->highPrecisionPixelBuffer) {
            g_drawingInfo->highPrecisionPixelBuffer[offsetToScanlineBegin + i] = currentHighPrecisionLinearPixelColour;
        }
    }

    return 0U;
}


void Rendering_drawSceneToWindow(Window_t* window,
                                 const Scene_t* scene,
                                 const Base3DObject_t* cameraBase,
                                 bool waitForVerticalSync
) {
    WindowDrawingResource_t* const windowDrawingResource = (void*)window->drawingResource;
    SceneDrawingResource_t* const sceneDrawingResource = (void*)scene->drawingResource;

    if (!windowDrawingResource->framebuffer.pixelBuffer) { goto end; }

    g_drawingInfo = &windowDrawingResource->drawingInfo;
    g_targetFramebuffer = &windowDrawingResource->framebuffer;
    g_targetScene = scene;
    g_drawingToWindowFramebuffer = true;

    if ((!g_drawingInfo->accumulatedSamples && !g_drawingInfo->skippedPixels) ||
        windowDrawingResource->lastTargetedScene != scene ||
        sceneDrawingResource->sceneHasChanged
    ) {
        windowDrawingResource->lastTargetedScene = scene;
        sceneDrawingResource->sceneHasChanged = false;
        g_drawingInfo->accumulatedSamples = 0U;
        g_drawingInfo->skippedPixels = 0U;

        Rendering_setCameraInfo(g_drawingInfo, cameraBase);
    }

    if (g_drawingInfo->accumulatedSamples <
        CORAL_GRAPHICS_RENDERING_SAMPLE_COUNT_HORIZONTAL * CORAL_GRAPHICS_RENDERING_SAMPLE_COUNT_VERTICAL
    ) {
        ThreadPool_startJob(&g_renderingThreadPool,
                            &Rendering_drawScanline,
                            NULL,
                            windowDrawingResource->framebuffer.height
        );

        ThreadPool_waitOnFinish(&g_renderingThreadPool);

        g_drawingInfo->skippedPixels = (g_drawingInfo->skippedPixels + 1U) %
                                       (CORAL_GRAPHICS_RENDERING_SAMPLE_SKIP_COUNT + 1U)
        ;

        if (!g_drawingInfo->skippedPixels) {
            g_drawingInfo->accumulatedSamples++;
        }
    }

end:
    Framebuffer_drawToWindow(&windowDrawingResource->framebuffer, window, waitForVerticalSync);
}


void Rendering_drawSceneToFramebuffer(Framebuffer_t* framebuffer,
                                      const Scene_t* scene,
                                      const Base3DObject_t* cameraBase
) {
    SceneDrawingResource_t* const sceneDrawingResource = (void*)scene->drawingResource;

    if (!framebuffer->pixelBuffer) { return; }

    g_drawingInfo = &((DrawingInfo_t){NULL, 0U, 0U});
    g_targetFramebuffer = framebuffer;
    g_targetScene = scene;
    g_drawingToWindowFramebuffer = false;

    Rendering_setCameraInfo(g_drawingInfo, cameraBase);

    ThreadPool_startJob(&g_renderingThreadPool,
                        &Rendering_drawScanline,
                        NULL,
                        framebuffer->height
    );

    ThreadPool_waitOnFinish(&g_renderingThreadPool);
}

#endif
