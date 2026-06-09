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

//- DEFINITIONS

typedef struct {
    size_t width;
    size_t height;
} WindowInfo_t;



//- WINDOW

//- - FUNCTIONS

//- - - WINDOW-SPECIFIC

void Window_init(Window_t* window, size_t width, size_t height, Window_t* parentWindow) {
    WindowInfo_t* windowInfo = CORAL_malloc(sizeof(WindowInfo_t));
    CORAL_ASSERT(windowInfo, "Failed to allocate memory for window information.");

    windowInfo->width = width;
    windowInfo->height = height;

    *window = ((Window_t){(uintptr_t)windowInfo, 0U, .shouldClose = false});
    memset(&window->callbacks, 0, sizeof(Window_CallbackFuncs_t));
}


void Window_destr(Window_t* window) {
    CORAL_free((void*)window->windowResource);
}


void Window_getDrawableDimensions(Window_t* window, size_t* width, size_t* height) {
    WindowInfo_t* windowInfo = (void*)window->windowResource;
    *width = windowInfo->width;
    *height = windowInfo->height;
}


bool Window_getMouseRestriction(Window_t* window) {
    return false;
}


void Window_setMouseRestriction(Window_t* window, bool restrictMouse) { }



//- - - GLOBAL

void Window_processEvents(void) { }


bool Window_allShouldClose(void) {
    return false;
}


bool Window_isInputCapable(void) {
    return false;
}


bool Window_getMouseKeyPressed(MouseInput_e key) {
    return false;
}


bool Window_getKeyboardKeyPressed(KeyboardInput_e key) {
    return false;
}


uint16_t Window_getModifierFlags(void) {
    return 0U;
}



//- FRAMEBUFFER

//- - FUNCTIONS

void Framebuffer_init(Framebuffer_t* framebuffer, Window_t* window) {
    WindowInfo_t* windowInfo = (void*)window->windowResource;
    Framebuffer__init(framebuffer, windowInfo->width, windowInfo->height);
}


void Framebuffer__init(Framebuffer_t* framebuffer, size_t width, size_t height) {
    *framebuffer = ((Framebuffer_t){0U, 0U, 0U, 0U, NULL});
    Framebuffer__resize(framebuffer, width, height);
}


void Framebuffer_destr(Framebuffer_t* framebuffer) {
    CORAL_free(framebuffer->pixelBuffer);
}


bool Framebuffer_isWindowDrawable(Framebuffer_t* framebuffer, Window_t* window) {
    return true;
}


void Framebuffer_makeWindowDrawable(Framebuffer_t* framebuffer, Window_t* window) { }


void Framebuffer_resize(Framebuffer_t* framebuffer, Window_t* window) {
    WindowInfo_t* windowInfo = (void*)window->windowResource;
    Framebuffer__resize(framebuffer, windowInfo->width, windowInfo->height);
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


bool Framebuffer_drawToWindow(Framebuffer_t* framebuffer, Window_t* window, bool waitForVerticalSync) {
    return true;
}



//- STARTUP & SHUTDOWN

void Windowing_startup(void) { }
void Windowing_shutdown(void) { }

#endif
