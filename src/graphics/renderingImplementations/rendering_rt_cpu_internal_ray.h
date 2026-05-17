#ifndef CORAL_GRAPHICS_RENDERING_RT_CPU_RAY
#define CORAL_GRAPHICS_RENDERING_RT_CPU_RAY

/*! \cond INTERNAL
 *  \file
 */

#include <stdint.h>
#include <stdbool.h>
#include "graphics/math.h"
#include "graphics/mesh.h"
#include "graphics/object.h"
#include "./rendering_rt_cpu_internal_bvh.h"



// RAYS

//- DEFINITIONS

typedef struct {
    Vec4_t origin;
    Vec4_t direction;
} Ray_t;


typedef struct {
    /*! \brief Factor to be applied to the ray's direction in order to find the entry of the intersection. */
    Float_t entryDirectionFactor;

    /*! \brief Factor to be applied to the ray's direction in order to find the exit of the intersection.
     *  \note Will only be set if \p hasExitIntersect is \c true.
     */
    Float_t exitDirectionFactor;

    /*! \note Will only be set if \p hasNormals is \c true. */
    Vec4_t entryNormalVec;

    /*! \note Will only be set if both \p hasExitIntersect and \p hasNormals are \c true. */
    Vec4_t exitNormalVec;

    /*! \brief \c true if the intersection is within the intersected object. */
    bool isInternalIntersect;
    bool hasExitIntersect;
    bool hasNormals;
} RayIntersectInfo_t;


/*! \brief Calculates the intersection of \p ray with the object encapsulated by the leaf node \p bvhNode.
 *  \returns \c true if an intersection was found, otherwise \c false.
 */
typedef bool (RayBVHNodeIntersectFunc_t)(const Ray_t* ray,
                                         RayIntersectInfo_t* intersectInfo,
                                         const BVH_t* bvh,
                                         const BVHNode_t* bvhNode,
                                         const uintptr_t context
);



//- FUNCTIONS

/*! \brief Calculates the position of a point on \p ray.
 *  \param[out] position    Will contain the calculated position.
 */
void Ray_getPositionFromDirectionFactor(const Ray_t* ray, Float_t directionFactor, Vec4_t* position);


/*! \brief Reflects \p ray at the position of \p directionFactor along \p reflectionNormal.
 *  \param[out] reflectedRay    Will contain the reflected ray.
 */
void Ray_reflect(const Ray_t* restrict ray,
                 Float_t directionFactor,
                 const Vec4_t* reflectionNormal,
                 Ray_t* restrict reflectedRay
);


/*! \brief Refracts \p ray at the position of \p directionFactor along \p refractionNormal.
 *  \param[out] refractedRay    Will contain the refracted ray.
 *  \returns
 *      \c true if \p ray was refracted, \c false if \p ray is to be reflected, which occurs if \p ray
 *      cannot refract or if \p ray is randomly determined to reflect based on Schlick's approximiation.
 */
bool Ray_refract(const Ray_t* restrict ray,
                 Float_t directionFactor,
                 const Vec4_t* refractionNormal,
                 Float_t refractiveIndexRatio,
                 Ray_t* restrict refractedRay
);


/*! \brief Calculates the intersection between \p ray and \p object.
 *  \param[out] intersectInfo   If an intersection was found, will contain the information about it.
 *  \returns \c true if an intersection was found.
 *  \note If an intersection was found, \p intersectInfo->hasExitIntersect will always be \c false.
 */
bool Ray_intersect_sceneObject(const Ray_t* ray, RayIntersectInfo_t* intersectInfo, const Base3DObject_t* object);


/*! \brief Calculates the intersection between \p ray and a sphere centered at the origin with a radius of \p sphereRadius.
 *  \copydetails Ray_intersect_sceneObject
 */
bool Ray_intersect_sphere(const Ray_t* ray, RayIntersectInfo_t* intersectInfo, Float_t sphereRadius);


/*! \brief Calculates the intersection between \p ray and \p mesh.
 *  \copydetails Ray_intersect_sceneObject
 */
bool Ray_intersect_mesh(const Ray_t* ray, RayIntersectInfo_t* intersectInfo, const Mesh_t* mesh);


/*! \brief Calculates the intersection between \p ray and \p boundingBox.
 *  \note
 *      If an intersection was found, \p intersectInfo->hasExitIntersect will always be \c true and
 *      \p intersectInfo->hasNormals will always be \c false.
 */
bool Ray_intersect_boundingBox(const Ray_t* ray, RayIntersectInfo_t* intersectInfo, const BoundingBox_t* boundingBox);


/*! \brief Calculates the intersection between \p ray and \p bvh.
 *  \param[in,out] intersectInfo                If an intersection with a leaf node was found, will contain the information
 *                                              about it; the \p entryRayDirectionFactor member will be treated as the
 *                                              maximum accaptable value for the \p entryRayDirectionFactor of an
 *                                              intersection with the bounding box of a node and \p exitRayDirectionFactor
 *                                              will be treated as the maximum accaptable value for the
 *                                              \p exitRayDirectionFactor of an intersection with the bounding box of a node.
 *  \param[in]  leafNodeIntersectFunc           Will be used to determine the exact intersection with the objects
 *                                              encapsulated by the leaf nodes.
 *  \param[in]  leafNodeIntersectFuncContext    Will be passed to \p leafNodeIntersectFunc as the \p context argument.
 *  \returns
 *      A pointer to the leaf node whose bounding box was intersected with the lowest \p entryRayDirectionFactor,
 *      or \c NULL if no intersection with a leaf node was found within the given constraints.
 */
BVHNode_t* Ray_intersect_bvh(const Ray_t* ray,
                             RayIntersectInfo_t* intersectInfo,
                             const BVH_t* bvh,
                             RayBVHNodeIntersectFunc_t* leafNodeIntersectFunc,
                             uintptr_t leafNodeIntersectFuncContext
);

/*! \endcond INTERNAL */

#endif
