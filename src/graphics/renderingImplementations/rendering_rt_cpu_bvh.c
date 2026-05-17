#ifdef CORAL_GRAPHICS_RENDERING_IMPL_RT_CPU

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <tgmath.h>
#include "common/utils.h"
#include "memory/allocators.h"
#include "math/vec.h"
#include "graphics/math.h"
#include "graphics/mesh.h"
#include "graphics/object.h"
#include "graphics/scene.h"
#include "graphics/renderingImplementations/rendering_rt_cpu_internal_bvh.h"



// BOUNDING VOLUME HIERARCHY

//- FUNCTIONS

//- - AXIS-ALIGNED BOUNDING BOXES

static inline void BoundingBox_init_null(BoundingBox_t* boundingBox) {
    boundingBox->minCoordinates = ((Vec3_t){{CORAL_MATH_GET_TYPE_MAX(CORAL_GRAPHICS_FLOAT_TYPE),
                                             CORAL_MATH_GET_TYPE_MAX(CORAL_GRAPHICS_FLOAT_TYPE),
                                             CORAL_MATH_GET_TYPE_MAX(CORAL_GRAPHICS_FLOAT_TYPE)
    }});

    boundingBox->maxCoordinates = ((Vec3_t){{CORAL_MATH_GET_TYPE_MIN(CORAL_GRAPHICS_FLOAT_TYPE),
                                             CORAL_MATH_GET_TYPE_MIN(CORAL_GRAPHICS_FLOAT_TYPE),
                                             CORAL_MATH_GET_TYPE_MIN(CORAL_GRAPHICS_FLOAT_TYPE)
    }});
}


void BoundingBox_init_merge(BoundingBox_t* destBoundingBox,
                            const BoundingBox_t* sourceBoundingBoxA,
                            const BoundingBox_t* sourceBoundingBoxB
) {
    for (size_t i = 0U; i < 3U; i++) {
        destBoundingBox->minCoordinates.raw[i] = CORAL_MIN(sourceBoundingBoxA->minCoordinates.raw[i],
                                                           sourceBoundingBoxB->minCoordinates.raw[i]
        );

        destBoundingBox->maxCoordinates.raw[i] = CORAL_MAX(sourceBoundingBoxA->maxCoordinates.raw[i],
                                                           sourceBoundingBoxB->maxCoordinates.raw[i]
        );
    }
}


bool BoundingBox_init_sceneObject(BoundingBox_t* boundingBox, const Base3DObject_t* object) {
    BoundingBox_t localBoundingBox;

    switch (object->base.type) {
        case CORAL_GRAPHICS_OBJECT_TYPE_SPHERE:
            BoundingBox_init_sphere(&localBoundingBox,
                                    CORAL_TO_PARENT_PTR(const SphereObject_t, base3D, object)->radius
            );
            break;

        case CORAL_GRAPHICS_OBJECT_TYPE_MESH:
            BoundingBox_init_mesh(&localBoundingBox,
                                  CORAL_TO_PARENT_PTR(const MeshObject_t, base3D, object)->mesh
            );
            break;

        default:
            return false;
    }

    BoundingBox_init_null(boundingBox);

    Mat4_t objectTransformMatrix;
    Scene_getObjectTransformationMatrix(object, &objectTransformMatrix);

    // Iterates over all 8 vertices of the local bounding box and transforms them to
    // their world coordinate counterpart to get the world bounding box
    for (size_t i = 0U; i < 8U; i++) {
        Vec4_t worldVertex;
        Vec4_t localVertex = {{(i & 1U) ? (localBoundingBox.maxCoordinates.x) : (localBoundingBox.minCoordinates.x),
                               (i & 2U) ? (localBoundingBox.maxCoordinates.y) : (localBoundingBox.minCoordinates.y),
                               (i & 4U) ? (localBoundingBox.maxCoordinates.z) : (localBoundingBox.minCoordinates.z),
                               1.0
        }};

        CORAL_MATH_VEC_MUL_MAT_GENERIC(4, 4, &objectTransformMatrix, &localVertex, &worldVertex);

        for (size_t j = 0U; j < 3U; j++) {
            boundingBox->minCoordinates.raw[j] = CORAL_MIN(boundingBox->minCoordinates.raw[j],
                                                           worldVertex.raw[j]
            );

            boundingBox->maxCoordinates.raw[j] = CORAL_MAX(boundingBox->maxCoordinates.raw[j],
                                                           worldVertex.raw[j]
            );
        }
    }

    for (size_t i = 0U; i < 3U; i++) {
        /*
        if (boundingBox->maxCoordinates.raw[i] - boundingBox->minCoordinates.raw[i] < 0.001) {
            boundingBox->minCoordinates.raw[i] = nexttoward(boundingBox->minCoordinates.raw[i],
                                                            CORAL_MATH_GET_TYPE_MIN(CORAL_GRAPHICS_FLOAT_TYPE)
            );

            boundingBox->maxCoordinates.raw[i] = nexttoward(boundingBox->maxCoordinates.raw[i],
                                                            CORAL_MATH_GET_TYPE_MAX(CORAL_GRAPHICS_FLOAT_TYPE)
            );
        }
        */

        boundingBox->maxCoordinates.raw[i] += 0.001;
        boundingBox->minCoordinates.raw[i] -= 0.001;
    }

    return true;
}


void BoundingBox_init_sphere(BoundingBox_t* boundingBox, Float_t sphereRadius) {
    boundingBox->minCoordinates = ((Vec3_t){-sphereRadius, -sphereRadius, -sphereRadius});
    boundingBox->maxCoordinates = ((Vec3_t){sphereRadius, sphereRadius, sphereRadius});
}


void BoundingBox_init_mesh(BoundingBox_t* boundingBox, const Mesh_t* mesh) {
    BoundingBox_init_null(boundingBox);

    for (size_t i = 0U; i < mesh->vertexCount; i++) {
        for (size_t j = 0U; j < 3U; j++) {
            if (mesh->vertices[i].position.raw[j] > boundingBox->maxCoordinates.raw[j]) {
                boundingBox->maxCoordinates.raw[j] = mesh->vertices[i].position.raw[j];
            }

            if (mesh->vertices[i].position.raw[j] < boundingBox->minCoordinates.raw[j]) {
                boundingBox->minCoordinates.raw[j] = mesh->vertices[i].position.raw[j];
            }
        }
    }

    for (size_t i = 0U; i < 3U; i++) {
        if (boundingBox->maxCoordinates.raw[i] - boundingBox->minCoordinates.raw[i] < 0.001) {
            boundingBox->minCoordinates.raw[i] = nexttoward(boundingBox->minCoordinates.raw[i],
                                                            CORAL_MATH_GET_TYPE_MIN(CORAL_GRAPHICS_FLOAT_TYPE)
            );

            boundingBox->maxCoordinates.raw[i] = nexttoward(boundingBox->maxCoordinates.raw[i],
                                                            CORAL_MATH_GET_TYPE_MAX(CORAL_GRAPHICS_FLOAT_TYPE)
            );
        }
    }
}


static Float_t BoundingBox_getSurfaceArea(const BoundingBox_t* boundingBox) {
    Float_t coordinateDifferences[3U];

    for (size_t i = 0U; i < 3U; i++) {
        coordinateDifferences[i] = boundingBox->maxCoordinates.raw[i] - boundingBox->minCoordinates.raw[i];
    }

    Float_t halfSurfaceArea = coordinateDifferences[0U] * coordinateDifferences[1U] +
                              coordinateDifferences[1U] * coordinateDifferences[2U] +
                              coordinateDifferences[2U] * coordinateDifferences[0U]
    ;

    return 2.0 * halfSurfaceArea;
}



//- <

static void BVH_getOptimalSiblingInSubtree(const BVHNode_t* subtreeRootNode,
                                           const BVHNode_t* bvhNode,
                                           const BVHNode_t** currentOptimalSibling,
                                           Float_t* currentOptimalAreaDifference,
                                           Float_t inheritedAreaDifference,
                                           const Float_t bvhNodeArea
) {
    BoundingBox_t mergedBoundingBox;
    BoundingBox_init_merge(&mergedBoundingBox, &subtreeRootNode->boundingBox, &bvhNode->boundingBox);

    Float_t subtreeRootArea = BoundingBox_getSurfaceArea(&subtreeRootNode->boundingBox);
    Float_t mergedArea = BoundingBox_getSurfaceArea(&mergedBoundingBox);
    Float_t totalSubtreeRootAreaDifference = mergedArea + inheritedAreaDifference;

    if (totalSubtreeRootAreaDifference < *currentOptimalAreaDifference) {
        *currentOptimalSibling = subtreeRootNode;
        *currentOptimalAreaDifference = totalSubtreeRootAreaDifference;
    }

    if (!subtreeRootNode->leftChildNode) { return; }

    inheritedAreaDifference = totalSubtreeRootAreaDifference - subtreeRootArea;
    Float_t childNodeAreaDifferenceLowerBound = inheritedAreaDifference + bvhNodeArea;

    const BVHNode_t* childNodes[] = {subtreeRootNode->leftChildNode, subtreeRootNode->rightChildNode};

    for (size_t i = 0U; i < 2U; i++) {
        if (childNodeAreaDifferenceLowerBound < *currentOptimalAreaDifference) {
            BVH_getOptimalSiblingInSubtree(childNodes[i],
                                           bvhNode,
                                           currentOptimalSibling,
                                           currentOptimalAreaDifference,
                                           inheritedAreaDifference,
                                           bvhNodeArea
            );
        }
    }
}


/*! \brief Possibly shifts \p bvhNode, if a shift would reduce the overall sourface area of the BVH.
 *  \returns A pointer to the \p BVHNode_t that is now at the place \p bvhNode was previosly, which may be \p bvhNode itself.
 */
static BVHNode_t* BVH_shiftNode(BVHNode_t* bvhNode) {
    BVHNode_t* possibleRotationPartners[3U] = {NULL, NULL, NULL};
    BVHNode_t* affectedNodes[3U] = {NULL, NULL, NULL};
    BoundingBox_t affectedNodesBoundingBoxesAferRotation[3U];
    Float_t affectedNodesPreviousAreas[3U];

    BVHNode_t* parentNode = bvhNode->parentNode;

    if (!parentNode) { return bvhNode; }

    if (parentNode->parentNode) {
        possibleRotationPartners[0U] = (parentNode->parentNode->leftChildNode == parentNode) ?
                                       (parentNode->parentNode->rightChildNode) :
                                       (parentNode->parentNode->leftChildNode)
        ;

        affectedNodes[0U] = parentNode;
        affectedNodesPreviousAreas[0U] = BoundingBox_getSurfaceArea(&parentNode->boundingBox);

        BoundingBox_init_merge(&affectedNodesBoundingBoxesAferRotation[0U],
                               &bvhNode->boundingBox,
                               &possibleRotationPartners[0U]->boundingBox
        );
    }

    BVHNode_t* siblingNode = (parentNode->leftChildNode == bvhNode) ?
                             (parentNode->rightChildNode) :
                             (parentNode->leftChildNode)
    ;

    if (siblingNode->leftChildNode) {
        possibleRotationPartners[1U] = siblingNode->leftChildNode;
        possibleRotationPartners[2U] = siblingNode->rightChildNode;
        affectedNodes[1U] = siblingNode;
        affectedNodes[2U] = siblingNode;
        affectedNodesPreviousAreas[1U] = BoundingBox_getSurfaceArea(&siblingNode->boundingBox);
        affectedNodesPreviousAreas[2U] = affectedNodesPreviousAreas[1U];

        BoundingBox_init_merge(&affectedNodesBoundingBoxesAferRotation[1U],
                               &bvhNode->boundingBox,
                               &siblingNode->rightChildNode->boundingBox
        );

        BoundingBox_init_merge(&affectedNodesBoundingBoxesAferRotation[2U],
                               &bvhNode->boundingBox,
                               &siblingNode->leftChildNode->boundingBox
        );
    }

    Float_t lowestAreaAfterRotation = CORAL_MATH_GET_TYPE_MAX(CORAL_GRAPHICS_FLOAT_TYPE);
    size_t lowestProfitableRotationPartnerIndex = 3U;

    for (size_t i = 0U; i < 3U; i++) {
        if (!possibleRotationPartners[i]) { continue; }

        Float_t areaAfterRotation = BoundingBox_getSurfaceArea(&affectedNodesBoundingBoxesAferRotation[i]);

        if (areaAfterRotation < affectedNodesPreviousAreas[i] && areaAfterRotation < lowestAreaAfterRotation) {
            lowestProfitableRotationPartnerIndex = i;
        }
    }

    if (lowestProfitableRotationPartnerIndex == 3U) { return bvhNode; }

    BVHNode_t* rotationPartner = possibleRotationPartners[lowestProfitableRotationPartnerIndex];
    BVHNode_t* rotationPartnerParentNode = rotationPartner->parentNode;

    rotationPartner->parentNode = bvhNode->parentNode;

    if (bvhNode->parentNode->leftChildNode == bvhNode) {
        bvhNode->parentNode->leftChildNode = rotationPartner;
    }
    else {
        bvhNode->parentNode->rightChildNode = rotationPartner;
    }

    bvhNode->parentNode = rotationPartnerParentNode;

    if (rotationPartnerParentNode->leftChildNode == rotationPartner) {
        rotationPartnerParentNode->leftChildNode = bvhNode;
    }
    else {
        rotationPartnerParentNode->rightChildNode = bvhNode;
    }

    affectedNodes[lowestProfitableRotationPartnerIndex]->boundingBox =
        affectedNodesBoundingBoxesAferRotation[lowestProfitableRotationPartnerIndex];

    return rotationPartner;
}


void BVH_init(BVH_t* bvh, uint32_t nodeCount) {
    DynamicPoolAllocator_init(&bvh->nodeAllocator, nodeCount, sizeof(BVHNode_t));
    bvh->rootNode = NULL;
}


void BVH_destr(BVH_t* bvh) {
    DynamicPoolAllocator_destr(&bvh->nodeAllocator);
    bvh->rootNode = NULL;
}


void BVH_insert(BVH_t* bvh, BVHNode_t* bvhNode) {
    bvhNode->leftChildNode = NULL;

    if (!bvh->rootNode) {
        bvh->rootNode = bvhNode;
        bvhNode->parentNode = NULL;
        return;
    }

    BVHNode_t* newInternalNode = DynamicPoolAllocator_alloc(&bvh->nodeAllocator);
    BVHNode_t* newSiblingNode = NULL;

    BVH_getOptimalSiblingInSubtree(bvh->rootNode,
                                   bvhNode,
                                   (const BVHNode_t**)&newSiblingNode,
                                   &((Float_t){CORAL_MATH_GET_TYPE_MAX(CORAL_GRAPHICS_FLOAT_TYPE)}),
                                   0.0,
                                   BoundingBox_getSurfaceArea(&bvhNode->boundingBox)
    );

    if (newSiblingNode->parentNode) {
        if (newSiblingNode->parentNode->leftChildNode == newSiblingNode) {
            newSiblingNode->parentNode->leftChildNode = newInternalNode;
        }
        else {
            newSiblingNode->parentNode->rightChildNode = newInternalNode;
        }
    }
    else {
        bvh->rootNode = newInternalNode;
    }

    newInternalNode->parentNode = newSiblingNode->parentNode;

    newInternalNode->leftChildNode = newSiblingNode;
    newSiblingNode->parentNode = newInternalNode;

    newInternalNode->rightChildNode = bvhNode;
    bvhNode->parentNode = newInternalNode;

    BVHNode_t* currentNode = newInternalNode;

    do {
        BoundingBox_init_merge(&currentNode->boundingBox,
                               &currentNode->leftChildNode->boundingBox,
                               &currentNode->rightChildNode->boundingBox
        );

        currentNode = BVH_shiftNode(currentNode);
    } while ((currentNode = currentNode->parentNode));
}


void BVH_remove(BVH_t* bvh, BVHNode_t* bvhNode) {
    CORAL_ASSERT(!bvhNode->leftChildNode, "Tried to remove an internal node.");

    BVHNode_t* parentNode = bvhNode->parentNode;

    if (!parentNode) {
        bvh->rootNode = NULL;
        return;
    }

    BVHNode_t* grandparentNode = parentNode->parentNode;
    BVHNode_t* siblingNode = (parentNode->leftChildNode == bvhNode) ?
                             (parentNode->rightChildNode) :
                             (parentNode->leftChildNode)
    ;

    if (!grandparentNode) {
        bvh->rootNode = siblingNode;
        siblingNode->parentNode = NULL;
        return;
    }

    if (grandparentNode->leftChildNode == parentNode) {
        grandparentNode->leftChildNode = siblingNode;
    }
    else {
        grandparentNode->rightChildNode = siblingNode;
    }

    siblingNode->parentNode = grandparentNode;
    DynamicPoolAllocator_free(&bvh->nodeAllocator, parentNode);

    BVHNode_t* currentNode = grandparentNode;

    do {
        BoundingBox_init_merge(&currentNode->boundingBox,
                               &currentNode->leftChildNode->boundingBox,
                               &currentNode->rightChildNode->boundingBox
        );

        currentNode = BVH_shiftNode(currentNode);
    } while ((currentNode = currentNode->parentNode));
}

#endif
