#ifndef CORAL_GRAPHICS_RENDERING
#define CORAL_GRAPHICS_RENDERING

/*! \file */

#include <stdbool.h>
#include "../common/api.h"
#include "./object.h"
#include "./scene.h"
#include "./windowing.h"



// RENDERING

/*! \defgroup CORAL_GRAPHICS_RENDERING Rendering
 *  \ingroup CORAL_GRAPHICS
 *  \brief Functions used in order to render scenes.
 *  \addtogroup CORAL_GRAPHICS_RENDERING
 *  @{
 */



//- FUNCTIONS

/*! \brief Draws \p scene to \p window.
 *  \param[in]  cameraBase              Must point to an object of type \p CORAL_GRAPHICS_OBJECT_TYPE_PERSPECTIVE_CAMERA
 *                                      or \p CORAL_GRAPHICS_OBJECT_TYPE_ORTHOGRAPHIC_CAMERA within \p scene.
 *  \param[in]  waitForVerticalSync     Will wait for the screen to refresh before presenting the window.
 */
CORAL_API void Rendering_drawSceneToWindow(Window_t* window,
                                           const Scene_t* scene,
                                           const Base3DObject_t* cameraBase,
                                           bool waitForVerticalSync
);


/*! \brief Draws \p scene to \p framebuffer.
 *  \param[in]  cameraBase  Must point to an object of type \p CORAL_GRAPHICS_OBJECT_TYPE_PERSPECTIVE_CAMERA or
 *                          \p CORAL_GRAPHICS_OBJECT_TYPE_ORTHOGRAPHIC_CAMERA within \p scene.
 */
CORAL_API void Rendering_drawSceneToFramebuffer(Framebuffer_t* framebuffer,
                                                const Scene_t* scene,
                                                const Base3DObject_t* cameraBase
);

/*! @} */

#endif
