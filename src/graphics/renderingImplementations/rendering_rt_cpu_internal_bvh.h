#ifndef CORAL_GRAPHICS_RENDERING_RT_CPU_BOUNDING_VOLUME_HIERARCHY
#define CORAL_GRAPHICS_RENDERING_RT_CPU_BOUNDING_VOLUME_HIERARCHY

/*! \cond INTERNAL
 *  \file
 */

#include <stdint.h>
#include <stdbool.h>
#include "memory/allocators.h"
#include "graphics/math.h"
#include "graphics/mesh.h"
#include "graphics/object.h"



// BOUNDING VOLUME HIERARCHY

//- DEFINITIONS

//- - AXIS-ALIGNED BOUNDING BOXES

typedef struct {
    Vec3_t minCoordinates;
    Vec3_t maxCoordinates;
} BoundingBox_t;



//- <

typedef struct BVHNode {
    BoundingBox_t boundingBox;
    struct BVHNode* parentNode;
    struct BVHNode* leftChildNode;  /*!< \brief If \c NULL, the node is a leaf. */

    union {
        struct BVHNode* rightChildNode;

        /*! \brief May be used to store additional data if the node is a leaf. */
        uintptr_t additionalLeafNodeData;
    };
} BVHNode_t;


typedef struct {
    BVHNode_t* rootNode;
    DynamicPoolAllocator_t nodeAllocator;   /*!< \brief Used for internal nodes. */
} BVH_t;



//- FUNCTIONS

//- - AXIS-ALIGNED BOUNDING BOXES

/*! \brief
 *      Initializes \p destBoundingBox to the world-coordinate bounds of \p sourceBoundingBoxA
 *      and \p sourceBoundingBoxB, effectively merging the two bounds.
 */
void BoundingBox_init_merge(BoundingBox_t* destBoundingBox,
                            const BoundingBox_t* sourceBoundingBoxA,
                            const BoundingBox_t* sourceBoundingBoxB
);

/*! \brief Initializes \p boundingBox to the world-coordinate bounds of \p object.
 *  \returns \c true if the bounds of \p object could be determined, \c false if \p object does not have any bounds.
 */
bool BoundingBox_init_sceneObject(BoundingBox_t* boundingBox, const Base3DObject_t* object);


/*! \brief Initializes \p boundingBox to the bounds of a sphere centered at the origin with a radius of \p sphereRadius. */
void BoundingBox_init_sphere(BoundingBox_t* boundingBox, Float_t sphereRadius);


/*! \brief Initializes \p boundingBox to the bounds of \p mesh. */
void BoundingBox_init_mesh(BoundingBox_t* boundingBox, const Mesh_t* mesh);



//- <

/*! \brief Initializes \p bvh.
 *  \param[in]  nodeCount   The amount of nodes per pool of the \mlink{BVH_t,nodeAllocator}.
 */
void BVH_init(BVH_t* bvh, uint32_t nodeCount);


/*! \brief Destructs \p bvh. */
void BVH_destr(BVH_t* bvh);


/*! \brief Inserts \p bvhNode into \p bvh.
 *  \param[in]  bvhNode     Must be initialized.
 */
void BVH_insert(BVH_t* bvh, BVHNode_t* bvhNode);


/*! \brief Removes \p bvhNode from \p bvh.
 *  \param[in] bvhNodeb     Must be a leaf node.
 */
void BVH_remove(BVH_t* bvh, BVHNode_t* bvhNode);

/*! \endcond INTERNAL */

#endif
