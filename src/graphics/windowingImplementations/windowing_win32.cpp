#include "graphics/windowing_implementation.h"
#ifdef CORAL_GRAPHICS_WINDOWING_IMPL_WIN32

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>
#include <windows.h>
#include <windowsx.h>
#include <objbase.h>
#include <dwmapi.h>
#include <d2d1.h>
#include <d2d1helper.h>

extern "C" {
#include "common/utils.h"
#include "memory/dynalloc.h"
#include "graphics/input.h"
#include "graphics/rendering_internal.h"
#include "graphics/windowing.h"
#include "graphics/windowing_internal.h"
}



// WINDOWING

//- DEFINITIONS

struct WindowData {
    alignas(LONG_PTR) bool hasMouseRestriction;
};



//- GLOBALS

static ID2D1Factory* g_d2dFactory;



//- WINDOW

//- - GLOBALS

static HINSTANCE g_processInstance;
static ATOM g_mainWindowClass;
static bool g_allShouldClose;



//- - FUNCTIONS

//- - - WINDOW-SPECIFIC

static std::pair<MouseInput_e, bool> Window_getMouseInputAndKeyPressedFromMouseEvent(UINT message, WPARAM wParam) {
    switch (message) {
        case WM_LBUTTONDOWN:    return {CORAL_GRAPHICS_INPUT_MOUSE_PRIMARY, true};
        case WM_LBUTTONUP:      return {CORAL_GRAPHICS_INPUT_MOUSE_PRIMARY, false};
        case WM_RBUTTONDOWN:    return {CORAL_GRAPHICS_INPUT_MOUSE_SECONDARY, true};
        case WM_RBUTTONUP:      return {CORAL_GRAPHICS_INPUT_MOUSE_SECONDARY, false};
        case WM_MBUTTONDOWN:    return {CORAL_GRAPHICS_INPUT_MOUSE_MIDDLE, true};
        case WM_MBUTTONUP:      return {CORAL_GRAPHICS_INPUT_MOUSE_MIDDLE, false};

        case WM_XBUTTONDOWN:    return {(HIWORD(wParam) == XBUTTON1) ?
                                        (CORAL_GRAPHICS_INPUT_MOUSE_EXTRA1) :
                                        (CORAL_GRAPHICS_INPUT_MOUSE_EXTRA2),
                                        true
                                };

        case WM_XBUTTONUP:      return {(HIWORD(wParam) == XBUTTON1) ?
                                        (CORAL_GRAPHICS_INPUT_MOUSE_EXTRA1) :
                                        (CORAL_GRAPHICS_INPUT_MOUSE_EXTRA2),
                                        false
                                };

        default:                coral_assert_base("Unrecognized message.", __FILE__, __LINE__);
    }
}


static KeyboardInput_e Window_getKeyboardInputFromKeyboardEvent(WPARAM wParam) {
    const WPARAM& virtualKeyCode = wParam;

    if (virtualKeyCode >= '0' && virtualKeyCode <= '9') {
        return (KeyboardInput_e)(CORAL_GRAPHICS_INPUT_KEYBOARD_0 + (virtualKeyCode - '0'));
    }

    if (virtualKeyCode >= 'A' && virtualKeyCode <= 'Z') {
        return (KeyboardInput_e)(CORAL_GRAPHICS_INPUT_KEYBOARD_A + (virtualKeyCode - 'A'));
    }

    switch (virtualKeyCode) {
        case VK_ESCAPE:     return CORAL_GRAPHICS_INPUT_KEYBOARD_ESCAPE;
        case VK_RETURN:     return CORAL_GRAPHICS_INPUT_KEYBOARD_ENTER;
        case VK_TAB:        return CORAL_GRAPHICS_INPUT_KEYBOARD_TAB;
        case VK_SPACE:      return CORAL_GRAPHICS_INPUT_KEYBOARD_SPACE;
        case VK_BACK:       return CORAL_GRAPHICS_INPUT_KEYBOARD_BACKSPACE;
        default:            return (KeyboardInput_e)0;
    }
}


static LRESULT CALLBACK Window_windowProcedure(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) {
    Window_t* window;

    if (message == WM_CREATE) {
        window = (Window_t*)((CREATESTRUCTA*)lParam)->lpCreateParams;
        SetWindowLongPtrA(windowHandle, GWLP_USERDATA, (LONG_PTR)window);
        return 0;
    }

    window = (Window_t*)GetWindowLongPtrA(windowHandle, GWLP_USERDATA);

    switch (message) {
        case WM_CLOSE:
            {
                window->shouldClose = true;
                return 0;
            }

        case WM_DESTROY:
            {
                Rendering_onWindowDestr(window);
                window->windowResource = (std::uintptr_t)nullptr;
                window->shouldClose = true;
                return 0;
            }

        case WM_SIZE:
            {
                if (wParam == SIZE_MINIMIZED) { return 0; }

                std::size_t newWidth = LOWORD(lParam);
                std::size_t newHeight = HIWORD(lParam);

                Rendering_onWindowResize(window, newWidth, newHeight);
                if (window->callbacks.resize) { window->callbacks.resize(window, newWidth, newHeight); }

                return 0;
            }

        case WM_SETFOCUS:
            {
                if (window->callbacks.focusChange) { window->callbacks.focusChange(window, true); }
                return 0;
            }

        case WM_KILLFOCUS:
            {
                if (window->callbacks.focusChange) { window->callbacks.focusChange(window, false); }
                return 0;
            }

        case WM_CAPTURECHANGED:
            {
                if (GetWindowLongPtrA(windowHandle, offsetof(WindowData, hasMouseRestriction))) {
                    SetWindowLongPtrA(windowHandle, offsetof(WindowData, hasMouseRestriction), (LONG_PTR)false);
                    ClipCursor(nullptr);
                    ShowCursor(TRUE);

                    if (window->callbacks.mouseRestrictionChange) {
                        window->callbacks.mouseRestrictionChange(window, false);
                    }
                }

                return 0;
            }

        case WM_MOUSEMOVE:
            {
                if (!window->callbacks.mouseMove) { return 0; }

                int mouseXPosition = GET_X_LPARAM(lParam);
                int mouseYPosition = GET_Y_LPARAM(lParam);
                std::uint16_t modifierFlags = Window_getModifierFlags();

                if (Window_getMouseRestriction(window)) {
                    std::size_t width;
                    std::size_t height;
                    Window_getDrawableDimensions(window, &width, &height);

                    mouseXPosition -= width / 2U;
                    mouseYPosition -= height / 2U;
                    if (!mouseXPosition && !mouseYPosition) { return 0; }

                    window->callbacks.mouseMove(window, mouseXPosition, mouseYPosition, modifierFlags);

                    POINT windowCenter = {.x = (LONG)width / 2, .y = (LONG)height / 2};
                    ClientToScreen(windowHandle, &windowCenter);
                    CORAL_ASSERT(SetCursorPos(windowCenter.x, windowCenter.y), "Failed to set cursor postion.");
                }
                else {
                    window->callbacks.mouseMove(window, mouseXPosition, mouseYPosition, modifierFlags);
                }

                return 0;
            }

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
            {
                if (window->callbacks.mouseInput) {
                    auto mouseInputAndKeyPressed = Window_getMouseInputAndKeyPressedFromMouseEvent(message, wParam);
                    window->callbacks.mouseInput(window,
                                                 mouseInputAndKeyPressed.first,
                                                 mouseInputAndKeyPressed.second,
                                                 Window_getModifierFlags()
                    );
                }

                return (GET_XBUTTON_WPARAM(wParam)) ? (TRUE) : (0);
            }

        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
            {
                if (window->callbacks.keyboardInput) {
                    KeyboardInput_e key = Window_getKeyboardInputFromKeyboardEvent(wParam);
                    bool keyPressed = !(HIWORD(lParam) & KF_UP);

                    if (key) {
                        window->callbacks.keyboardInput(window, key, keyPressed, Window_getModifierFlags());
                    }
                }

                return 0;
            }
    }

    return DefWindowProcA(windowHandle, message, wParam, lParam);
}


void Window_init(Window_t* window, std::size_t width, std::size_t height, Window_t* parentWindow) {
    std::memset(&window->callbacks, 0, sizeof(Window_CallbackFuncs_t));

    window->windowResource = (std::uintptr_t)nullptr;
    window->drawingResource = (std::uintptr_t)nullptr;
    window->shouldClose = false;

    HWND windowHandle = CreateWindowExA(0, //WS_EX_COMPOSITED | WS_EX_TRANSPARENT,
                                        MAKEINTATOM(g_mainWindowClass),
                                        "CORAL Window",
                                        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                        CW_USEDEFAULT,
                                        CW_USEDEFAULT,
                                        width,
                                        height,
                                        nullptr,
                                        nullptr,
                                        g_processInstance,
                                        window
    );

    CORAL_ASSERT(windowHandle, "Failed to create window.");
    window->windowResource = (std::uintptr_t)windowHandle;

    Rendering_onWindowInit(window);
}


void Window_destr(Window_t* window) {
    if (!window->windowResource) { return; }

    CORAL_ASSERT(DestroyWindow((HWND)window->windowResource), "Failed to destroy window.");
}


void Window_getDrawableDimensions(Window_t* window, std::size_t* width, std::size_t* height) {
    RECT clientRectangle;

    CORAL_ASSERT(GetClientRect((HWND)window->windowResource, &clientRectangle), "Failed to get the drawable dimensions of a window.");

    *width = clientRectangle.right;
    *height = clientRectangle.bottom;
}


bool Window_getMouseRestriction(Window_t* window) {
    return GetCapture() == (HWND)window->windowResource;
}


// TODO: Check whether the mouse really is captured and whether window is in the foreground
void Window_setMouseRestriction(Window_t* window, bool restrictMouse) {
    if (Window_getMouseRestriction(window) == restrictMouse) { return; }

    if (restrictMouse) {
        SetCapture((HWND)window->windowResource);
        SetWindowLongPtrA((HWND)window->windowResource, offsetof(WindowData, hasMouseRestriction), (LONG_PTR)true);

        std::size_t width;
        std::size_t height;
        Window_getDrawableDimensions(window, &width, &height);

        POINT topLeftWindowCorner = {.x = 0, .y = 0};
        POINT bottomRightWindowCorner = {.x = (LONG)width, .y = (LONG)height};

        ClientToScreen((HWND)window->windowResource, &topLeftWindowCorner);
        ClientToScreen((HWND)window->windowResource, &bottomRightWindowCorner);

        RECT windowRectangle = {.left = topLeftWindowCorner.x,
                                .top = topLeftWindowCorner.y,
                                .right = bottomRightWindowCorner.x,
                                .bottom = bottomRightWindowCorner.y
        };

        ClipCursor(&windowRectangle);
        ShowCursor(FALSE);

        if (window->callbacks.mouseRestrictionChange) {
            window->callbacks.mouseRestrictionChange(window, true);
        }
    }
    else {
        CORAL_ASSERT(ReleaseCapture(), "Failed to release mouse capture.");
    }
}



//- - - GLOBAL

void Window_processEvents(void) {
    MSG message;

    while (PeekMessageA(&message, nullptr, 0U, 0U, PM_REMOVE)) {
        if (message.message == WM_QUIT) { g_allShouldClose = true; }
        else {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
    }
}


bool Window_allShouldClose(void) {
    return g_allShouldClose; 
}


bool Window_isInputCapable(void) {
    return true;
}


bool Window_getMouseKeyPressed(MouseInput_e key) {
    CORAL_ASSERT(key, "Invalid key.");

    static constexpr BYTE virtualKeyCodes[] = {VK_LBUTTON,
                                               VK_RBUTTON,
                                               VK_MBUTTON,
                                               VK_XBUTTON1,
                                               VK_XBUTTON2
    };

    return GetKeyState(virtualKeyCodes[key - 1U]) < 0;
}


bool Window_getKeyboardKeyPressed(KeyboardInput_e key) {
    CORAL_ASSERT(key, "Invalid key.");

    static constexpr BYTE virtualKeyCodes[] = {VK_ESCAPE,
                                               VK_RETURN,
                                               VK_TAB,
                                               VK_SPACE,
                                               VK_BACK
    };

    BYTE virtualKeyCode;

    if (key >= CORAL_GRAPHICS_INPUT_KEYBOARD_A && key <= CORAL_GRAPHICS_INPUT_KEYBOARD_Z) {
        virtualKeyCode = 'A' + (key - CORAL_GRAPHICS_INPUT_KEYBOARD_A);
    }
    else if (key >= CORAL_GRAPHICS_INPUT_KEYBOARD_0 && key <= CORAL_GRAPHICS_INPUT_KEYBOARD_9) {
        virtualKeyCode = '0' + (key - CORAL_GRAPHICS_INPUT_KEYBOARD_0);
    }
    else {
        virtualKeyCode = virtualKeyCodes[key - 1U];
    }

    return GetKeyState(virtualKeyCode) < 0;
}


std::uint16_t Window_getModifierFlags(void) {
    static constexpr std::pair<BYTE, std::uint16_t> modifierKeys[] = {{VK_CONTROL, CORAL_GRAPHICS_INPUT_MODIFIER_CTRL},
                                                                      {VK_MENU, CORAL_GRAPHICS_INPUT_MODIFIER_ALT},
                                                                      {VK_SHIFT, CORAL_GRAPHICS_INPUT_MODIFIER_SHIFT},
                                                                      {VK_LBUTTON, CORAL_GRAPHICS_INPUT_MODIFIER_MOUSE_PRIMARY},
                                                                      {VK_RBUTTON, CORAL_GRAPHICS_INPUT_MODIFIER_MOUSE_SECONDARY},
                                                                      {VK_MBUTTON, CORAL_GRAPHICS_INPUT_MODIFIER_MOUSE_MIDDLE},
                                                                      {VK_XBUTTON1, CORAL_GRAPHICS_INPUT_MODIFIER_MOUSE_EXTRA1},
                                                                      {VK_XBUTTON2, CORAL_GRAPHICS_INPUT_MODIFIER_MOUSE_EXTRA2}
    };

    std::uint16_t modifierFlags = 0U;

    for (size_t i = 0U; i < CORAL_ARRAY_LENGTH(modifierKeys); i++) {
        if (GetKeyState(modifierKeys[i].first) < 0) { modifierFlags |= modifierKeys[i].second; }
    }

    return modifierFlags;
}



//- FRAMEBUFFER

//- - DEFINITIONS

#define CORAL_GRAPHICS_FRAMEBUFFER_BLOCK_SIZE ((std::size_t)(2U * 1024U))



//- - GLOBALS

const D2D1_RENDER_TARGET_PROPERTIES g_generalRenderTargetProperties = {.type = D2D1_RENDER_TARGET_TYPE_SOFTWARE,
                                                                       .pixelFormat = {.format = DXGI_FORMAT_B8G8R8A8_UNORM,
                                                                                       .alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED,
                                                                       },
                                                                       .dpiX = 0.0f,
                                                                       .dpiY = 0.0f,
                                                                       .usage = D2D1_RENDER_TARGET_USAGE_NONE,
                                                                       .minLevel = D2D1_FEATURE_LEVEL_DEFAULT
};



//- - FUNCTIONS

static inline void Framebuffer_init_renderTarget(Framebuffer_t* framebuffer, Window_t* window) {
    D2D1_HWND_RENDER_TARGET_PROPERTIES windowRenderTargetProperties = {.hwnd = (HWND)window->windowResource,
                                                                       .pixelSize = {.width = (UINT32)framebuffer->width,
                                                                                     .height = (UINT32)framebuffer->height,
                                                                       },
                                                                       .presentOptions = D2D1_PRESENT_OPTIONS_NONE
    };

    ID2D1HwndRenderTarget* windowRenderTarget;

    HRESULT result = g_d2dFactory->CreateHwndRenderTarget(&g_generalRenderTargetProperties,
                                                          &windowRenderTargetProperties,
                                                          &windowRenderTarget
    );

    CORAL_ASSERT(result == S_OK, "Failed to create a Direct2D HwndRenderTarget.");

    framebuffer->windowResource = (uintptr_t)windowRenderTarget;
}


static inline void Framebuffer_init_bitmap(Framebuffer_t* framebuffer) {
    HRESULT result;

    ID2D1HwndRenderTarget* windowRenderTarget = (ID2D1HwndRenderTarget*)framebuffer->windowResource;
    ID2D1Bitmap* bitmap;

    D2D1_BITMAP_PROPERTIES bitmapProperties = {.pixelFormat = g_generalRenderTargetProperties.pixelFormat};
    windowRenderTarget->GetDpi(&bitmapProperties.dpiX, &bitmapProperties.dpiY);

    D2D1_SIZE_U bitmapPixelSize = {.width = (UINT32)framebuffer->width,
                                   .height = (UINT32)framebuffer->height
    };

    result = windowRenderTarget->CreateBitmap(bitmapPixelSize,
                                              framebuffer->pixelBuffer,
                                              sizeof(Vec4b_u) * framebuffer->width,
                                              &bitmapProperties,
                                              &bitmap
    );

    CORAL_ASSERT(result == S_OK, "Failed to create a Direct2D Bitmap.");

    framebuffer->drawingResource = (uintptr_t)bitmap;
}


void Framebuffer_init(Framebuffer_t* framebuffer, Window_t* window) {
    HRESULT result;

    if (!g_d2dFactory) {
        result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        CORAL_ASSERT(result == S_OK, "Failed to initialize the COM.");

        result = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_ID2D1Factory, (void**)&g_d2dFactory);
        CORAL_ASSERT(result == S_OK, "Failed to create the Direct2D Factory.");
    }

    std::size_t width;
    std::size_t height;
    Window_getDrawableDimensions(window, &width, &height);

    Framebuffer__init(framebuffer, width, height);
    Framebuffer_init_renderTarget(framebuffer, window);
    Framebuffer_init_bitmap(framebuffer);
}


void Framebuffer__init(Framebuffer_t* framebuffer, size_t width, size_t height) {
    *framebuffer = ((Framebuffer_t){0U, 0U, 0U, 0U, nullptr});
    Framebuffer__resize(framebuffer, width, height);
}


static inline void Framebuffer_destr_renderTarget(Framebuffer_t* framebuffer) {
    if (framebuffer->windowResource) {
        ((ID2D1HwndRenderTarget*)framebuffer->windowResource)->Release();
        framebuffer->windowResource = (uintptr_t)nullptr;
    }
}


static inline void Framebuffer_destr_bitmap(Framebuffer_t* framebuffer) {
    if (framebuffer->drawingResource) {
        ((ID2D1Bitmap*)framebuffer->drawingResource)->Release();
        framebuffer->drawingResource = (uintptr_t)nullptr;
    }
}


void Framebuffer_destr(Framebuffer_t* framebuffer) {
    Framebuffer_destr_bitmap(framebuffer);
    Framebuffer_destr_renderTarget(framebuffer);

    if (framebuffer->pixelBuffer) {
        CORAL_free(framebuffer->pixelBuffer);
        framebuffer->pixelBuffer = nullptr;
    }
}


bool Framebuffer_isWindowDrawable(Framebuffer_t* framebuffer, Window_t* window) {
    if (framebuffer->windowResource &&
        ((ID2D1HwndRenderTarget*)framebuffer->windowResource)->GetHwnd() == (HWND)window->windowResource
    ) {
        CORAL_ASSERT(framebuffer->drawingResource,
                     "Framebuffer had valid windowResource but invalid drawingResource."
        );

        return true;
    }

    return false;
}


void Framebuffer_makeWindowDrawable(Framebuffer_t* framebuffer, Window_t* window) {
    if (Framebuffer_isWindowDrawable(framebuffer, window)) { return; }

    Framebuffer_destr_bitmap(framebuffer);
    Framebuffer_destr_renderTarget(framebuffer);

    Framebuffer_init(framebuffer, window);
}


void Framebuffer_resize(Framebuffer_t* framebuffer, Window_t* window) {
    std::size_t newWidth;
    std::size_t newHeight;
    Window_getDrawableDimensions(window, &newWidth, &newHeight);
    Framebuffer__resize(framebuffer, newWidth, newHeight);
}


void Framebuffer__resize(Framebuffer_t* framebuffer, std::size_t newWidth, std::size_t newHeight) {
    if ((!newWidth || !newHeight) && framebuffer->pixelBuffer) {
        CORAL_free(framebuffer->pixelBuffer);
        framebuffer->pixelBuffer = nullptr;
    }
    else {
        std::size_t oldAllocationSize = CORAL_TO_ALIGNMENT(sizeof(Vec4b_u) * framebuffer->width * framebuffer->height,
                                                           CORAL_GRAPHICS_FRAMEBUFFER_BLOCK_SIZE
        );

        std::size_t newAllocationSize = CORAL_TO_ALIGNMENT(sizeof(Vec4b_u) * newWidth * newHeight,
                                                           CORAL_GRAPHICS_FRAMEBUFFER_BLOCK_SIZE
        );

        if (oldAllocationSize != newAllocationSize) {
            framebuffer->pixelBuffer = (Vec4b_u*)CORAL_realloc(framebuffer->pixelBuffer, newAllocationSize);

            CORAL_ASSERT(framebuffer->pixelBuffer, "Failed to allocate memory for a framebuffer.");
        }
    }

    framebuffer->width = newWidth;
    framebuffer->height = newHeight;
}


bool Framebuffer_drawToWindow(Framebuffer_t* framebuffer, Window_t* window, bool waitForVerticalSync) {
    if (!Framebuffer_isWindowDrawable(framebuffer, window)) { return false; }

    HRESULT result;

    if (framebuffer->pixelBuffer) {
        ID2D1HwndRenderTarget* windowRenderTarget = (ID2D1HwndRenderTarget*)framebuffer->windowResource;
        D2D1_SIZE_U windowRenderTargetPixelSize = windowRenderTarget->GetPixelSize();

        if (windowRenderTargetPixelSize.width != framebuffer->width || windowRenderTargetPixelSize.height != framebuffer->height) {
            windowRenderTargetPixelSize.width = framebuffer->width;
            windowRenderTargetPixelSize.height = framebuffer->height;

            result = windowRenderTarget->Resize(&windowRenderTargetPixelSize);
            CORAL_ASSERT(result == S_OK, "Failed to resize a Direct2D HwndRenderTarget.");
        }

        ID2D1Bitmap* bitmap = (ID2D1Bitmap*)framebuffer->drawingResource;
        D2D1_SIZE_U bitmapPixelSize = bitmap->GetPixelSize();

        if (bitmapPixelSize.width != framebuffer->width || bitmapPixelSize.height != framebuffer->height) {
            Framebuffer_destr_bitmap(framebuffer);
            Framebuffer_init_bitmap(framebuffer);

            bitmap = (ID2D1Bitmap*)framebuffer->drawingResource;
        }

        result = bitmap->CopyFromMemory(nullptr, framebuffer->pixelBuffer, sizeof(Vec4b_u) * framebuffer->width);
        CORAL_ASSERT(result == S_OK, "Failed to copy from an in-memory pixel buffer to a Direct2D Bitmap.");

        windowRenderTarget->BeginDraw();
        windowRenderTarget->DrawBitmap(bitmap);
        result = windowRenderTarget->EndDraw();
        CORAL_ASSERT(result == S_OK, "Failed to draw a Direct2D Bitmap to a Direct2D HwndRenderTarget.");
    }

    if (waitForVerticalSync) {
        result = DwmFlush();
        CORAL_ASSERT(result == S_OK, "Failed to wait for V-Sync by using the Windows DWM.");
    }

    return true;
}



//- STARTUP & SHUTDOWN

void Windowing_startup() {
    g_processInstance = GetModuleHandleA(nullptr);

    WNDCLASSEXA mainWindowClass = {.cbSize = sizeof(WNDCLASSEXA),
                                   .style = 0U,
                                   .lpfnWndProc = &Window_windowProcedure,
                                   .cbClsExtra = 0,
                                   .cbWndExtra = sizeof(WindowData),
                                   .hInstance = g_processInstance,
                                   .hIcon = nullptr,
                                   .hCursor = (HCURSOR)LoadImageA(nullptr,
                                                                  IDC_ARROW,
                                                                  IMAGE_CURSOR,
                                                                  0,
                                                                  0,
                                                                  LR_DEFAULTSIZE | LR_SHARED
                                   ),
                                   .hbrBackground = (HBRUSH)(COLOR_BACKGROUND + 1),
                                   .lpszMenuName = nullptr,
                                   .lpszClassName = "CORAL_MAIN_WINDOW",
                                   .hIconSm = nullptr
    };

    g_mainWindowClass = RegisterClassExA(&mainWindowClass);
    CORAL_ASSERT(g_mainWindowClass, "Failed to register the main window class.");
}


void Windowing_shutdown() {
    if (g_d2dFactory) {
        g_d2dFactory->Release();
        CoUninitialize();
    }

    UnregisterClassA(MAKEINTATOM(g_mainWindowClass), g_processInstance);
}

#endif
