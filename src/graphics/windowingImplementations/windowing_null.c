#include "graphics/windowing_implementation.h"
#ifdef CORAL_GRAPHICS_WINDOWING_IMPL_NULL

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "common/utils.h"
#include "memory/dynalloc.h"
#include "math/vecTypes.h"
#include "graphics/windowing.h"
#include "graphics/windowing_internal.h"



// WINDOWING

//- WINDOW

//- - FUNCTIONS

void Window_init(Window_t* window, size_t width, size_t height, Window_t* parentWindow) {
    memset(&window->callbacks, 0, sizeof(Window_CallbackFuncs_t));
    *window = ((Window_t){(uintptr_t)parentWindow, (uintptr_t)NULL, .shouldClose = false});
}


void Window_destr(Window_t* window) { }


void Window_getDrawableDimensions(Window_t* window, size_t* width, size_t* height) {
    *width = 1U;
    *height = 1U;
}


void Window_getMouseRestriction(Window_t* window) {
    return false;
}


void Window_setMouseRestriction(Window_t* window, bool restrictMouse) { }
void Window_processEvents(void) { }


bool Window_allShouldClose(void) {
    return true;
}



//- FRAMEBUFFER

//- - FUNCTIONS

void Framebuffer_init(Framebuffer_t* framebuffer, Window_t* window) {
    framebuffer->windowResource = (uintptr_t)window;
    framebuffer->drawingResource = (uintptr_t)NULL;
    framebuffer->width = 1U;
    framebuffer->height = 1U;

    framebuffer->pixelBuffer = CORAL_malloc(sizeof(Vec4b_u));
    CORAL_ASSERT(framebuffer->pixelBuffer, "Failed to allocate framebuffer.");

    framebuffer->pixelBuffer[0U] = ((Vec4b_u){0U, 0U, 0U, 0U});
}


void Framebuffer_destr(Framebuffer_t* framebuffer, Window_t* window) {
    CORAL_free(framebuffer->pixelBuffer);
    memset(framebuffer, 0, sizeof(Framebuffer_t));
}


void Framebuffer_resize(Framebuffer_t* framebuffer, Window_t* window) {
    Framebuffer__resize(framebuffer, 1U, 1U);
}


void Framebuffer__resize(Framebuffer_t* framebuffer, size_t newWidth, size_t newHeight) {
    if ((!newWidth || !newHeight) && framebuffer->pixelBuffer) {
        CORAL_free(framebuffer->pixelBuffer);
        framebuffer->pixelBuffer = NULL;
    }
    else {
        size_t oldPixelBufferAllocationSize = sizeof(Vec4b_u) * framebuffer->width * framebuffer->height;
        size_t newPixelBufferAllocationSize = sizeof(Vec4b_u) * newWidth * newHeight;

        if (oldPixelBufferAllocationSize >= newPixelBufferAllocationSize) {
            // TODO: Shrink when applicable
            goto end;
        }

        framebuffer->pixelBuffer = CORAL_realloc(framebuffer->pixelBuffer, newPixelBufferAllocationSize);
        CORAL_ASSERT(framebuffer->pixelBuffer, "Failed to allocate framebuffer.");
    }

end:
    framebuffer->width = newWidth;
    framebuffer->height = newHeight;
}


void Framebuffer_drawToWindow(Framebuffer_t* framebuffer, Window_t* window, bool waitForVerticalSync) { }



//- STARTUP & SHUTDOWN

void Windowing_startup(void) { }
void Windowing_shutdown(void) { }

#endif
