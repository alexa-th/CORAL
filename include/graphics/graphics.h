#ifndef CORAL_GRAPHICS
#define CORAL_GRAPHICS

/*! \file */

#include "../common/api.h"

/*! \defgroup CORAL_GRAPHICS Graphics
 *  \brief
 *      Types and functions related to rendering graphics, displaying them in windows and
 *      receiving input from those windows.
 *
 *  \note
 *      Unless otherwise stated, all functions of this sub-module may only be called by one thread;
 *      specifically the one thread that called \p Graphics_startup().
 */



// INCLUDES

#include "./math.h"
#include "./mesh.h"
#include "./object.h"
#include "./scene.h"
#include "./input.h"
#include "./windowing.h"
#include "./rendering.h"



// STARTUP & SHUTDOWN

/*! \addtogroup CORAL_GRAPHICS
 *  @{
 */

/*! \brief Starts the graphics sub-module.
 *
 *  Must be called before any other function of this sub-module.
 */
CORAL_API void Graphics_startup(void);


/*! \brief Shuts the graphics sub-module down.
 *
 *  Must be called after any other function of this sub-module and after all objects
 *  initialized by this sub-module have been destructed.
 */
CORAL_API void Graphics_shutdown(void);

/*! @} */

#endif
