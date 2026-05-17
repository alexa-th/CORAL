#ifdef CORAL_GRAPHICS_RENDERING_IMPL_RT_CPU

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <tgmath.h>
#include "common/error.h"
#include "common/random.h"
#include "common/utils.h"
#include "math/vec.h"
#include "math/mat.h"
#include "graphics/math.h"
#include "graphics/mesh.h"
#include "graphics/scene.h"
#include "graphics/renderingImplementations/rendering_rt_cpu_internal_bvh.h"
#include "graphics/renderingImplementations/rendering_rt_cpu_internal_ray.h"



// RAYS

//- FUNCTIONS

void Ray_getPositionFromDirectionFactor(const Ray_t* ray, Float_t directionFactor, Vec4_t* position) {
    CORAL_MATH_VEC_GENERIC(4,
                           mul_scalar,
                           &ray->direction,
                           directionFactor,
                           position
    );

    CORAL_MATH_VEC_GENERIC(4,
                           add,
                           position,
                           &ray->origin,
                           position
    );
}


void Ray_reflect(const Ray_t* restrict ray,
                 Float_t directionFactor,
                 const Vec4_t* reflectionNormal,
                 Ray_t* restrict reflectedRay
) {
    Vec4_t normalizedRayDirection;
    CORAL_MATH_VEC_GENERIC(4, normalize, &ray->direction, &normalizedRayDirection);

    Float_t normalFactor = -2.0 * CORAL_MATH_VEC_GENERIC(4,
                                                         rawDot,
                                                         reflectionNormal,
                                                         &normalizedRayDirection
    );

    CORAL_MATH_VEC_GENERIC(4,
                           mul_scalar,
                           reflectionNormal,
                           normalFactor,
                           &reflectedRay->direction
    );

    CORAL_MATH_VEC_GENERIC(4,
                           add,
                           &reflectedRay->direction,
                           &normalizedRayDirection,
                           &reflectedRay->direction
    );

    Ray_getPositionFromDirectionFactor(ray, directionFactor, &reflectedRay->origin);
}


bool Ray_refract(const Ray_t* restrict ray,
                 Float_t directionFactor,
                 const Vec4_t* refractionNormal,
                 Float_t refractiveIndexRatio,
                 Ray_t* restrict refractedRay
) {
    Vec4_t normalizedRayDirection;
    CORAL_MATH_VEC_GENERIC(4, normalize, &ray->direction, &normalizedRayDirection);

    Float_t cos_theta = -CORAL_MATH_VEC_GENERIC(4, rawDot, &normalizedRayDirection, refractionNormal);
    cos_theta = CORAL_MIN(CORAL_MAX(cos_theta, -1.0), 1.0);

    Float_t sin_theta = sqrt(1.0 - cos_theta * cos_theta);
    if (sin_theta * refractiveIndexRatio > 1.0) { return false; }

    Float_t randomFloat = _Generic(((Float_t){0.0}), float: Random__float, double: Random__double)(0.0, 1.0);
    if (randomFloat == 0.0) { return false; }

    Float_t reflectance = (refractiveIndexRatio - 1.0) / (refractiveIndexRatio + 1.0);
    reflectance *= reflectance;
    reflectance += (1.0 - reflectance) * pow(1.0 - cos_theta, 5.0);
    if (reflectance > randomFloat) { return false; }

    Vec4_t perpendicularDirection = {.raw = {[3U] = 0.0}};
    Vec4_t parallelDirection = {.raw = {[3U] = 0.0}};

    for (size_t i = 0U; i < 3U; i++) {
        perpendicularDirection.raw[i] = (normalizedRayDirection.raw[i] + cos_theta * refractionNormal->raw[i]) *
                                        refractiveIndexRatio
        ;
    }

    Float_t perpendicularDirectionSquaredMagnitude = CORAL_MATH_VEC_GENERIC(4, squaredMagnitude, &perpendicularDirection);

    CORAL_MATH_VEC_GENERIC(4,
                           mul_scalar,
                           refractionNormal,
                           -sqrt(fabs(1.0 - perpendicularDirectionSquaredMagnitude)),
                           &parallelDirection
    );

    CORAL_MATH_VEC_GENERIC(4, add, &perpendicularDirection, &parallelDirection, &refractedRay->direction);

    Ray_getPositionFromDirectionFactor(ray, directionFactor, &refractedRay->origin);

    return true;
}


bool Ray_intersect_sceneObject(const Ray_t* ray, RayIntersectInfo_t* intersectInfo, const Base3DObject_t* object) {
    Mat4_t objectTransformMatrix;
    Scene_getObjectTransformationMatrix(object, &objectTransformMatrix);

    Mat4_t inverseObjectTransformMatrix;
    Error_t error = CORAL_MATH_MAT_GENERIC(4, 4, invert, &objectTransformMatrix, &inverseObjectTransformMatrix);
    if (error == CORAL_ERROR_INVALID_ARGS) { return false; }

    Ray_t transformedRay;
    CORAL_MATH_VEC_MUL_MAT_GENERIC(4, 4, &inverseObjectTransformMatrix, &ray->origin, &transformedRay.origin);
    CORAL_MATH_VEC_MUL_MAT_GENERIC(4, 4, &inverseObjectTransformMatrix, &ray->direction, &transformedRay.direction);

    RayIntersectInfo_t localIntersectInfo;
    bool hasHit = false;

    switch (object->base.type) {
        case CORAL_GRAPHICS_OBJECT_TYPE_SPHERE:
            hasHit = Ray_intersect_sphere(&transformedRay,
                                          &localIntersectInfo,
                                          CORAL_TO_PARENT_PTR(const SphereObject_t, base3D, object)->radius
            );
            break;

        case CORAL_GRAPHICS_OBJECT_TYPE_MESH:
            hasHit = Ray_intersect_mesh(&transformedRay,
                                        &localIntersectInfo,
                                        CORAL_TO_PARENT_PTR(const MeshObject_t, base3D, object)->mesh
            );
            break;

        default:
            CORAL_ASSERT(0, "Object is not renderable.");
    }

    if (!hasHit) { return false; }

    intersectInfo->entryDirectionFactor = localIntersectInfo.entryDirectionFactor;
    intersectInfo->isInternalIntersect = localIntersectInfo.isInternalIntersect;
    intersectInfo->hasExitIntersect = localIntersectInfo.hasExitIntersect;
    CORAL_ASSERT(!intersectInfo->hasExitIntersect, "Did have exit intersection.");

    if ((intersectInfo->hasNormals = localIntersectInfo.hasNormals)) {
        // To correctly transform normals
        CORAL_MATH_MAT_GENERIC(4, 4, transpose, &inverseObjectTransformMatrix, &objectTransformMatrix);
        CORAL_MATH_VEC_MUL_MAT_GENERIC(4, 4, &objectTransformMatrix, &localIntersectInfo.entryNormalVec, &intersectInfo->entryNormalVec);

        intersectInfo->entryNormalVec.raw[3U] = 0.0;
        CORAL_MATH_VEC_GENERIC(4, normalize, &intersectInfo->entryNormalVec, &intersectInfo->entryNormalVec);

    }

    return true;
}


bool Ray_intersect_sphere(const Ray_t* ray, RayIntersectInfo_t* intersectInfo, Float_t sphereRadius) {
    Float_t quadraticFormulaA = CORAL_MATH_VEC_GENERIC(4, squaredMagnitude, &ray->direction);
    Float_t quadraticFormulaB = 2.0 * CORAL_MATH_VEC_GENERIC(4, rawDot, &ray->direction, &ray->origin);
    Float_t quadraticFormulaC = CORAL_MATH_VEC_GENERIC(4, squaredMagnitude, &ray->origin) - sphereRadius * sphereRadius;
    Float_t quadraticFormulaDiscriminant = quadraticFormulaB * quadraticFormulaB - 4.0 * quadraticFormulaA * quadraticFormulaC;

    if (quadraticFormulaDiscriminant < 0.0) { return false; }

    Float_t quadraticFormulaRoot = sqrt(quadraticFormulaDiscriminant);
    Float_t quadraticFormulaReciprocal = 1.0 / (2.0 * quadraticFormulaA);
    intersectInfo->entryDirectionFactor = (-quadraticFormulaB - quadraticFormulaRoot) * quadraticFormulaReciprocal;
    intersectInfo->isInternalIntersect = false;

    if (intersectInfo->entryDirectionFactor <= 0.0) {
        intersectInfo->entryDirectionFactor = (-quadraticFormulaB + quadraticFormulaRoot) * quadraticFormulaReciprocal;
        intersectInfo->isInternalIntersect = true;

        if (intersectInfo->entryDirectionFactor < 0.0) { return false; }
    }

    Ray_getPositionFromDirectionFactor(ray, intersectInfo->entryDirectionFactor, &intersectInfo->entryNormalVec);
    intersectInfo->entryNormalVec.raw[3U] = 0.0;
    CORAL_MATH_VEC_GENERIC(4, normalize, &intersectInfo->entryNormalVec, &intersectInfo->entryNormalVec);

    if (intersectInfo->isInternalIntersect) {
        CORAL_MATH_VEC_GENERIC(4,
                               mul_scalar,
                               &intersectInfo->entryNormalVec,
                               -1.0,
                               &intersectInfo->entryNormalVec
        );
    }

    intersectInfo->hasExitIntersect = false;
    intersectInfo->hasNormals = true;
    return true;
}


bool Ray_intersect_mesh(const Ray_t* ray, RayIntersectInfo_t* intersectInfo, const Mesh_t* mesh) {
    bool hasIntersectWithAny = false;

    for (size_t i = 0U; i < mesh->triangleCount; i++) {
        Vec3_t triangleVertexPositions[3U];
        Vec3_t triangleNormal = mesh->vertices[mesh->indices[3U * i]].normal;

        for (size_t j = 0U; j < 3U; j++) {
            const Vertex_t* currentVertex = mesh->vertices + mesh->indices[3U * i + j];
            triangleVertexPositions[j] = currentVertex->position;
        }

        Vec3_t rayOrigin = {ray->origin.x, ray->origin.y, ray->origin.z};
        Vec3_t rayDirection = {ray->direction.x, ray->direction.y, ray->direction.z};

        Vec3_t firstTriangleVertexToRayOrigin;
        CORAL_MATH_VEC_GENERIC(3,
                               sub,
                               &rayOrigin,
                               &triangleVertexPositions[0U],
                               &firstTriangleVertexToRayOrigin
        );

        Float_t denominator = CORAL_MATH_VEC_GENERIC(3, rawDot, &rayDirection, &triangleNormal);
        if (denominator == 0.0) { continue; }

        Float_t directionFactor = -CORAL_MATH_VEC_GENERIC(3, rawDot, &firstTriangleVertexToRayOrigin, &triangleNormal) /
                                  denominator
        ;

        if (directionFactor <= 0.0 || (hasIntersectWithAny && intersectInfo->entryDirectionFactor < directionFactor)) {
            continue;
        }

        Vec3_t firstTriangleVertexToIntersectPosition;

        CORAL_MATH_VEC_GENERIC(3, mul_scalar, &rayDirection, directionFactor, &firstTriangleVertexToIntersectPosition);

        CORAL_MATH_VEC_GENERIC(3,
                               add,
                               &firstTriangleVertexToIntersectPosition,
                               &firstTriangleVertexToRayOrigin,
                               &firstTriangleVertexToIntersectPosition
        );

        Vec3_t uTriangleEdge;
        Vec3_t vTriangleEdge;
        Vec3_t wTriangleCoordinateHelper;
        Vec3_t uCoordinateHelper;
        Vec3_t vCoordinateHelper;

        CORAL_MATH_VEC_GENERIC(3, sub, &triangleVertexPositions[1U], &triangleVertexPositions[0U], &uTriangleEdge);
        CORAL_MATH_VEC_GENERIC(3, sub, &triangleVertexPositions[2U], &triangleVertexPositions[0U], &vTriangleEdge);

        CORAL_MATH_VEC_GENERIC(3, cross, &uTriangleEdge, &vTriangleEdge, &wTriangleCoordinateHelper);

        Float_t coordinateHelperFactor = CORAL_MATH_VEC_GENERIC(3, squaredMagnitude, &wTriangleCoordinateHelper);
        CORAL_MATH_VEC_GENERIC(3, div_scalar, &wTriangleCoordinateHelper, coordinateHelperFactor, &wTriangleCoordinateHelper);

        CORAL_MATH_VEC_GENERIC(3,
                               cross,
                               &firstTriangleVertexToIntersectPosition,
                               &vTriangleEdge,
                               &uCoordinateHelper
        );

        CORAL_MATH_VEC_GENERIC(3,
                               cross,
                               &uTriangleEdge,
                               &firstTriangleVertexToIntersectPosition,
                               &vCoordinateHelper
        );

        Vec2_t intersectCoordinates = {.u = CORAL_MATH_VEC_GENERIC(3, rawDot, &wTriangleCoordinateHelper, &uCoordinateHelper),
                                       .v = CORAL_MATH_VEC_GENERIC(3, rawDot, &wTriangleCoordinateHelper, &vCoordinateHelper)
        };

        bool hasIntersect = intersectCoordinates.u + intersectCoordinates.v <= 1.0 &&
                            intersectCoordinates.u >= 0.0 &&
                            intersectCoordinates.u <= 1.0 &&
                            intersectCoordinates.v >= 0.0 &&
                            intersectCoordinates.v <= 1.0
        ;

        if (hasIntersect) {
            hasIntersectWithAny = true;
            intersectInfo->entryDirectionFactor = directionFactor;
            intersectInfo->entryNormalVec = ((Vec4_t){triangleNormal.x, triangleNormal.y, triangleNormal.z, 0.0});
            intersectInfo->isInternalIntersect = denominator > 0.0;
            intersectInfo->hasExitIntersect = false;
            intersectInfo->hasNormals = true;
        }
    }

    if (hasIntersectWithAny && intersectInfo->isInternalIntersect) {
        CORAL_MATH_VEC_GENERIC(4,
                               mul_scalar,
                               &intersectInfo->entryNormalVec,
                               -1.0,
                               &intersectInfo->entryNormalVec
        );
    }

    return hasIntersectWithAny;
}


bool Ray_intersect_boundingBox(const Ray_t* ray, RayIntersectInfo_t* intersectInfo, const BoundingBox_t* boundingBox) {
    Float_t intersectIntervalBeginMax = CORAL_MATH_GET_TYPE_MIN(CORAL_GRAPHICS_FLOAT_TYPE);
    Float_t intersectIntervalEndMin = CORAL_MATH_GET_TYPE_MAX(CORAL_GRAPHICS_FLOAT_TYPE);

    for (size_t i = 0U; i < 3U; i++) {
        Float_t intersectIntervalBegin;
        Float_t intersectIntervalEnd;
        Float_t minCoordinateDifference = boundingBox->minCoordinates.raw[i] - ray->origin.raw[i];
        Float_t maxCoordinateDifference = boundingBox->maxCoordinates.raw[i] - ray->origin.raw[i];

        /*
        if ((fabs(minCoordinateDifference) < 0.001) || (fabs(maxCoordinateDifference) < 0.001)) {
            return false;
        }
        */

        if (ray->direction.raw[i] == 0.0) {
            intersectIntervalBegin = (boundingBox->minCoordinates.raw[i] > 0.0) ?
                                     (CORAL_MATH_GET_TYPE_MAX(CORAL_GRAPHICS_FLOAT_TYPE)) :
                                     (CORAL_MATH_GET_TYPE_MIN(CORAL_GRAPHICS_FLOAT_TYPE))
            ;

            intersectIntervalEnd = (boundingBox->maxCoordinates.raw[i] > 0.0) ?
                                   (CORAL_MATH_GET_TYPE_MAX(CORAL_GRAPHICS_FLOAT_TYPE)) :
                                   (CORAL_MATH_GET_TYPE_MIN(CORAL_GRAPHICS_FLOAT_TYPE))
            ;
        }
        else {
            intersectIntervalBegin = minCoordinateDifference / ray->direction.raw[i];
            intersectIntervalEnd = maxCoordinateDifference / ray->direction.raw[i];
        }

        if (intersectIntervalBegin > intersectIntervalEnd) {
            Float_t intersectIntervalBeginCopy = intersectIntervalBegin;
            intersectIntervalBegin = intersectIntervalEnd;
            intersectIntervalEnd = intersectIntervalBeginCopy;
        }

        if (intersectIntervalBegin > intersectIntervalBeginMax) {
            intersectIntervalBeginMax = intersectIntervalBegin;
        }

        if (intersectIntervalEnd < intersectIntervalEndMin) {
            intersectIntervalEndMin = intersectIntervalEnd;
        }
    }

    if (intersectIntervalBeginMax >= intersectIntervalEndMin) { return false; }

    intersectInfo->entryDirectionFactor = intersectIntervalBeginMax;
    intersectInfo->exitDirectionFactor = intersectIntervalEndMin;
    intersectInfo->isInternalIntersect = false;
    intersectInfo->hasExitIntersect = true;
    intersectInfo->hasNormals = false;
    return true;
}


static const BVHNode_t* Ray_intersect_bvh_base(const Ray_t* ray,
                                               RayIntersectInfo_t* intersectInfo,
                                               const BVH_t* bvh,
                                               const BVHNode_t* currentNode,
                                               RayBVHNodeIntersectFunc_t* leafNodeIntersectFunc,
                                               const uintptr_t leafNodeIntersectFuncContext
) {
    Float_t maxEntryDirectionFactor = intersectInfo->entryDirectionFactor;
    Float_t minExitDirectionFactor = intersectInfo->exitDirectionFactor;

    while (true) {
        if (!currentNode->leftChildNode) {
            RayIntersectInfo_t leafNodeIntersectInfo;

            if (leafNodeIntersectFunc(ray, &leafNodeIntersectInfo, bvh, currentNode, leafNodeIntersectFuncContext)) {
                *intersectInfo = leafNodeIntersectInfo;
                return currentNode;
            }

            return NULL;
        }

        RayIntersectInfo_t childIntersectInfos[2U];
        const BVHNode_t* childNodesWithIntersect[2U] = {currentNode->leftChildNode, currentNode->rightChildNode};

        for (size_t i = 0U; i < 2U; i++) {
            bool hasIntersect = Ray_intersect_boundingBox(ray,
                                                          &childIntersectInfos[i],
                                                          &childNodesWithIntersect[i]->boundingBox
            );

            if (!hasIntersect ||
                childIntersectInfos[i].entryDirectionFactor > maxEntryDirectionFactor ||
                childIntersectInfos[i].exitDirectionFactor < minExitDirectionFactor
            ) { childNodesWithIntersect[i] = NULL; }
        }

        if (childNodesWithIntersect[0U] && childNodesWithIntersect[1U]) {
            if (childIntersectInfos[0U].entryDirectionFactor > childIntersectInfos[1U].entryDirectionFactor) {
                RayIntersectInfo_t tempIntersectInfo = childIntersectInfos[0U];
                childIntersectInfos[0U] = childIntersectInfos[1U];
                childIntersectInfos[1U] = tempIntersectInfo;

                childNodesWithIntersect[0U] = currentNode->rightChildNode;
                childNodesWithIntersect[1U] = currentNode->leftChildNode;
            }

            RayIntersectInfo_t recursiveIntersectInfos[2U];
            const BVHNode_t* recursiveReturnValues[2U];

            recursiveIntersectInfos[0U] = ((RayIntersectInfo_t){maxEntryDirectionFactor, minExitDirectionFactor});
            recursiveReturnValues[0U] = Ray_intersect_bvh_base(ray,
                                                               &recursiveIntersectInfos[0U],
                                                               bvh,
                                                               childNodesWithIntersect[0U],
                                                               leafNodeIntersectFunc,
                                                               leafNodeIntersectFuncContext
            );

            if (recursiveReturnValues[0U]) {
                maxEntryDirectionFactor = recursiveIntersectInfos[0U].entryDirectionFactor;

                // Second recursive branch can already not be closer
                if (childIntersectInfos[1U].entryDirectionFactor > maxEntryDirectionFactor) {
                    *intersectInfo = recursiveIntersectInfos[0U];
                    return recursiveReturnValues[0U];
                }
            }

            recursiveIntersectInfos[1U] = ((RayIntersectInfo_t){maxEntryDirectionFactor, minExitDirectionFactor});
            recursiveReturnValues[1U] = Ray_intersect_bvh_base(ray,
                                                               &recursiveIntersectInfos[1U],
                                                               bvh,
                                                               childNodesWithIntersect[1U],
                                                               leafNodeIntersectFunc,
                                                               leafNodeIntersectFuncContext
            );

            if (recursiveReturnValues[1U] &&
                recursiveIntersectInfos[1U].entryDirectionFactor < recursiveIntersectInfos[0U].entryDirectionFactor
            ) {
                *intersectInfo = recursiveIntersectInfos[1U];
                return recursiveReturnValues[1U];
            }
            else if (recursiveReturnValues[0U]) {
                *intersectInfo = recursiveIntersectInfos[0U];
                return recursiveReturnValues[0U];
            }
            else { return NULL; }
        }
        else if (childNodesWithIntersect[0U]) {
            currentNode = childNodesWithIntersect[0U];
        }
        else if (childNodesWithIntersect[1U]) {
            currentNode = childNodesWithIntersect[1U];
        }
        else { return NULL; }
    }
}


BVHNode_t* Ray_intersect_bvh(const Ray_t* ray,
                             RayIntersectInfo_t* intersectInfo,
                             const BVH_t* bvh,
                             RayBVHNodeIntersectFunc_t* leafNodeIntersectFunc,
                             const uintptr_t leafNodeIntersectFuncContext
) {
    Float_t maxEntryRayDirectionFactor = intersectInfo->entryDirectionFactor;
    Float_t minExitRayDirectionFactor = intersectInfo->exitDirectionFactor;
    RayIntersectInfo_t localIntersectInfo;

    if (!bvh->rootNode ||
        !Ray_intersect_boundingBox(ray, &localIntersectInfo, &bvh->rootNode->boundingBox) ||
        localIntersectInfo.entryDirectionFactor > maxEntryRayDirectionFactor ||
        localIntersectInfo.exitDirectionFactor < minExitRayDirectionFactor
    ) { return NULL; }

    localIntersectInfo = ((RayIntersectInfo_t){maxEntryRayDirectionFactor,
                                               minExitRayDirectionFactor
    });

    const BVHNode_t* intersectedNode = Ray_intersect_bvh_base(ray,
                                                              &localIntersectInfo,
                                                              bvh,
                                                              bvh->rootNode,
                                                              leafNodeIntersectFunc,
                                                              leafNodeIntersectFuncContext
    );

    if (intersectedNode) { *intersectInfo = localIntersectInfo; }
    return (BVHNode_t*)intersectedNode;
}

#endif
