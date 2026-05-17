#include "graphics/graphics.h"
#include "graphics/windowing_internal.h"
#include "graphics/rendering_internal.h"



// STARTUP & SHUTDOWN

void Graphics_startup(void) {
    Windowing_startup();
    Rendering_startup();
}


void Graphics_shutdown(void) {
    Rendering_shutdown();
    Windowing_shutdown();
}
