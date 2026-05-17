#ifndef CORAL_GRAPHICS_SCENE
#define CORAL_GRAPHICS_SCENE

/*! \file */

#include <stdint.h>
#include "../common/api.h"
#include "../containers/linkedList.h"
#include "../math/vecTypes.h"
#include "./math.h"
#include "./object.h"



// SCENE

/*! \defgroup CORAL_GRAPHICS_SCENE Scene_t
 *  \ingroup CORAL_GRAPHICS
 *  \brief Represents a collection of objects, which can also be assembled into a hierarchy.
 *  \addtogroup CORAL_GRAPHICS_SCENE
 *  @{
 */



//- DEFINITIONS

/*! \copybrief CORAL_GRAPHICS_SCENE */
typedef struct {
    /*! \brief List of all root objects in a \p Scene_t.
     *
     *  The individual links of this list are the \mlink{SceneComponent_t,link} members of
     *  the \p SceneComponent_t components of the root objects.
     */
    LinkedList_t rootObjects;
    uintptr_t drawingResource;
    size_t totalObjectCount;
    uint32_t rootObjectCount;
    Vec3b_u backgroundColour;
} Scene_t;



//- FUNCTIONS

/*! \brief Initializes \p scene. */
CORAL_API void Scene_init(Scene_t* scene);


/*! \brief Destructs \p scene.
 *  \note
 *      All objects within \p scene will be removed from \p scene by calling \p Scene_removeObject().
 *      However, these objects must still be destructed manually.
 */
CORAL_API void Scene_destr(Scene_t* scene);


/*! \brief Adds \p object to \p scene.
 *  \param[in]  object          Must have a \p SceneComponent_t component.
 *  \param[in]  parentObject    (Opt.) If provided, must already be within \p scene.
 */
CORAL_API void Scene_addObject(Scene_t* scene, Base3DObject_t* restrict object, Base3DObject_t* restrict parentObject);


/*! \copydoc Scene_addObject
 *  \param[in]  parentObjectSceneComponent  (Opt.) If \p parentObject has been provided,
 *                                          the \p SceneComponent_t of \p parentObject.
 */
CORAL_API void Scene__addObject(Scene_t* scene,
                                Base3DObject_t* restrict object,
                                Base3DObject_t* restrict parentObject,
                                SceneComponent_t* restrict objectSceneComponent,
                                SceneComponent_t* restrict parentObjectSceneComponent
);


/*! \brief Removes \p object from \p scene. */
CORAL_API void Scene_removeObject(Scene_t* scene, Base3DObject_t* object);


/*! \copybrief Scene_removeObject */
CORAL_API void Scene__removeObject(Scene_t* scene, Base3DObject_t* object, SceneComponent_t* sceneComponent);


/*! \brief Must be called if any attributes of \p object have been changed. */
CORAL_API void Scene_changedObjectAttributes(Scene_t* scene, Base3DObject_t* object);


/*! \copybrief Scene_changedObjectAttributes */
CORAL_API void Scene__changedObjectAttributes(Scene_t* scene, Base3DObject_t* object, SceneComponent_t* sceneComponent);



//- - SCENE OBJECTS

/*! \returns The pointer to the object whose \p SceneComponent_t includes \p sceneComponentLink. */
CORAL_API Base3DObject_t* Scene_getObject(const LinkedList_SingleLink_t* sceneComponentLink);


/*! \returns The pointer to the object which includes \p objectSceneComponent. */
CORAL_API Base3DObject_t* Scene__getObject(const SceneComponent_t* sceneComponent);


/*! \brief Calculates the transformation matrix of \p object.
 *  \param[out] transformMatrix     Will contain the transformation matrix.
 */
CORAL_API void Scene_getObjectTransformationMatrix(const Base3DObject_t* object, Mat4_t* transformMatrix);


/*! \copydoc Scene_getObjectTransformationMatrix */
CORAL_API void Scene__getObjectTransformationMatrix(const Base3DObject_t* object,
                                                    const SceneComponent_t* sceneComponent,
                                                    Mat4_t* transformMatrix
);

/*! @} */

#endif
