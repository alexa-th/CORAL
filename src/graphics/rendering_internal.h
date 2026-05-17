#ifndef CORAL_GRAPHICS_RENDERING_INTERNAL
#define CORAL_GRAPHICS_RENDERING_INTERNAL

/*! \cond INTERNAL
 *  \file
 */

#include <stddef.h>
#include "graphics/mesh.h"
#include "graphics/object.h"
#include "graphics/object_internal.h"
#include "graphics/scene.h"
#include "graphics/windowing.h"



// RENDERING

//- STARTUP & SHUTDOWN

void Rendering_startup(void);
void Rendering_shutdown(void);



//- CALLBACKS

//- - WINDOWING

/*! \note Called after \p window has been initialized. */
void Rendering_onWindowInit(Window_t* window);


/*! \note Called before \p window is destructed. */
void Rendering_onWindowDestr(Window_t* window);


/*! \note Called before a \p Window_ResizeCallbackFunc_t is called. */
void Rendering_onWindowResize(Window_t* window, size_t newWidth, size_t newHeight);



//- - MESHES

/*! \note Called after \p mesh is initialized. */
void Rendering_onMeshInit(Mesh_t* mesh);


/*! \note Called before \p mesh is destructed. */
void Rendering_onMeshDestr(Mesh_t* mesh);



//- - OBJECTS

/*! \note Called after all components of \p object besides \p renderingComponent have been initialized. */
void Rendering_onObjectInit(BaseObject_t* object, RenderingComponent_ot* renderingComponent);


/*! \note Called after all components of \p object besides \p renderingComponent have been destructed. */
void Rendering_onObjectDestr(BaseObject_t* object, RenderingComponent_ot* renderingComponent);


/*! \param[in]  materialComponent   May not be a component of \p object.
 *  \returns \c true if the operation was successful, otherwise \c false.
 *  \note Called to set the active material of \p object to \p materialComponent.
 */
bool Rendering_onObjectSetActiveMaterial(BaseObject_t* object,
                                         RenderingComponent_ot* renderingComponent,
                                         const MaterialComponent_t* materialComponent
);


/*! \returns The size and alignment of the renderer defined \p RenderingComponent_ot. */
SizeAndAlignment_t Rendering_getRenderingComponentSizeAndAlignment(void);



//- - SCENE

/*! \note Called after \p scene has been initialized. */
void Rendering_onSceneInit(Scene_t* scene);


/*! \note Called before \p scene is destructed. */
void Rendering_onSceneDestr(Scene_t* scene);


/*! \param[in]  sceneComponent  The \p SceneComponent_t of \p object.
 *  \note Called after \p object has been added to \p scene.
 */
void Rendering_onObjectAdditionToScene(Scene_t* scene, Base3DObject_t* object, SceneComponent_t* sceneComponent);


/*! \param[in]  sceneComponent  The \p SceneComponent_t of \p object.
 *  \note Called before \p object is removed from \p scene.
 */
void Rendering_onObjectRemovalFromScene(Scene_t* scene, Base3DObject_t* object, SceneComponent_t* sceneComponent);


/*! \param[in]  sceneComponent  The \p SceneComponent_t of \p object.
 *  \note Called in response to a call to \p Scene_changedObjectAttributes() or \p Scene__changedObjectAttributes().
 */
void Rendering_onObjectAttributesChanged(Scene_t* scene, Base3DObject_t* object, SceneComponent_t* sceneComponent);

/*! \endcond INTERNAL */

#endif
