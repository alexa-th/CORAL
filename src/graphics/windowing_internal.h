#ifndef CORAL_GRAPHICS_WINDOWING_INTERNAL
#define CORAL_GRAPHICS_WINDOWING_INTERNAL

/*! \cond INTERNAL
 *  \file
 */

#include <stddef.h>
#include <stdbool.h>
#include "filesystem/path.h"
#include "graphics/windowing.h"



// WINDOWING

//- STARTUP & SHUTDOWN

void Windowing_startup(void);
void Windowing_shutdown(void);



//- FRAMEBUFFER

//- - FUNCTIONS

/*! \name Common framebuffer functions
 *  \brief
 *      These functions must not be implemented by the windowing implementation as they do not
 *      depend on the specifics of the windowing implementation.
 *
 *  @{
 */

void Framebuffer__init(Framebuffer_t* framebuffer, size_t width, size_t height);


void Framebuffer__resize(Framebuffer_t* framebuffer, size_t width, size_t height);


Error_t Framebuffer_writeToFile(const Framebuffer_t* framebuffer, const Path_t* filePath, bool allowOverwrite);

/*! @} */

/*! \endcond INTERNAL */

#endif
