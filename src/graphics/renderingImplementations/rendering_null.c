#ifdef CORAL_GRAPHICS_RENDERING_IMPL_NULL

#include <stddef.h>
#include <stdbool.h>
#include "graphics/object.h"
#include "graphics/object_internal.h"
#include "graphics/scene.h"
#include "graphics/windowing.h"
#include "graphics/rendering.h"
#include "graphics/rendering_internal.h"



// RENDERING

//- STARTUP & SHUTDOWN

void Rendering_startup(void) { }
void Rendering_shutdown(void) { }



//- CALLBACKS

//- - WINDOWING

void Rendering_onWindowInit(Window_t* window) { }
void Rendering_onWindowDestr(Window_t* window) { }
void Rendering_onWindowResize(Window_t* window, size_t newWidth, size_t newHeight) { }



//- - OBJECTS

void Rendering_onObjectInit(BaseObject_t* object, RenderingComponent_ot* renderingComponent) { }
void Rendering_onObjectDestr(BaseObject_t* object, RenderingComponent_ot* renderingComponent) { }


SizeAndAlignment_t Rendering_getRenderingComponentSizeAndAlignment(void) {
    return ((SizeAndAlignment_t){0U, 1U});
}



//- - SCENE

void Rendering_onSceneInit(Scene_t* scene) { }
void Rendering_onSceneDestr(Scene_t* scene) { }
void Rendering_onObjectAdditionToScene(Scene_t* scene, Base3DObject_t* object, SceneComponent_t* sceneComponent) { }
void Rendering_onObjectRemovalFromScene(Scene_t* scene, Base3DObject_t* object, SceneComponent_t* sceneComponent) { }



//- FUNCTIONS

void Rendering_drawSceneToWindow(Window_t* window, Scene_t* scene, Base3DObject_t* cameraBase, bool waitForVerticalSync) { }

#endif
