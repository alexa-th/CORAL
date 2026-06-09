#ifndef CORAL_GRAPHICS_WINDOWING
#define CORAL_GRAPHICS_WINDOWING

/*! \file */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "../common/api.h"
#include "../math/vecTypes.h"
#include "./input.h"



// WINDOWING

/*! \defgroup CORAL_GRAPHICS_WINDOWING Windowing
 *  \ingroup CORAL_GRAPHICS
 *  \brief
 *      Types and functions related to creating and displaying windows and
 *      representing drawable content in framebuffers.
 *
 *  \addtogroup CORAL_GRAPHICS_WINDOWING
 *  @{
 */



//- WINDOW

/*! \defgroup CORAL_GRAPHICS_WINDOW Window_t
 *  \brief Represents a window displayed to the user.
 *  \addtogroup CORAL_GRAPHICS_WINDOW
 *  @{
 */



//- - DEFINITIONS

/*! \copybrief CORAL_GRAPHICS_WINDOW */
typedef struct Window Window_t;



//- - - CALLBACKS

/*! \brief Called when the user resizes a window. */
typedef void (Window_ResizeCallbackFunc_t)(Window_t* window, size_t newWidth, size_t newHeight);


/*! \brief Called when the focus of \p window changes; whether focus was gained or lost is indicated by \p hasFocus. */
typedef void (Window_FocusChangeCallbackFunc_t)(Window_t* window, bool hasFocus);


/*! \brief
 *      Called when the mouse restriction of \p window changes; whether the mouse is restricted
 *      to \p window or not is indicated by \p hasMouseRestriction.
 *
 *  \note This callback is also called when calling \p Window_setMouseRestriction.
 */
typedef void (Window_MouseRestrictionChangeCallbackFunc_t)(Window_t* window, bool hasMouseRestriction);


/*! \brief Called when the user moves the mouse in \p window.
 *  \note
 *      If the mouse was restricted to \p window, \p mouseXPosition and \p mouseYPosition will be relative to the last recorded
 *      mouse position. Otherwise, \p mouseYPosition and \p mouseYPosition will be relative to the upper-left corner of \p window.
 */
typedef void (Window_MouseMoveCallbackFunc_t)(Window_t* window, int mouseXPosition, int mouseYPosition, uint16_t modifierFlags);


/*! \brief Called when the user provides mouse input when focussed on \p window.
 *  \param[in]  keyPressed  \c true if \p key was pressed, \c false if \p key was released.
 */
typedef void (Window_MouseInputCallbackFunc_t)(Window_t* window, MouseInput_e key, bool keyPressed, uint16_t modifierFlags);


/*! \brief Called when the user provides keyboard input when focussed on \p window.
 *  \param[in]  keyPressed  \c true if \p key was pressed, \c false if \p key was released.
 */
typedef void (Window_KeyboardInputCallbackFunc_t)(Window_t* window, KeyboardInput_e key, bool keyPressed, uint16_t modifierFlags);



//- <

/*! \brief Contains all callbacks that may be executed during the lifetime of a \p Window_t. */
typedef struct {
    Window_ResizeCallbackFunc_t* resize;
    Window_FocusChangeCallbackFunc_t* focusChange;
    Window_MouseRestrictionChangeCallbackFunc_t* mouseRestrictionChange;
    Window_MouseMoveCallbackFunc_t* mouseMove;
    Window_MouseInputCallbackFunc_t* mouseInput;
    Window_KeyboardInputCallbackFunc_t* keyboardInput;
} Window_CallbackFuncs_t;


/*! \brief A window displayed to the user.
 *  \note
 *      All \p Window_ functions must be called with the same \p window as was
 *      passed to \p Window_init() until \p Window_destr() is called.
 */
struct Window {
    uintptr_t windowResource;
    uintptr_t drawingResource;
    Window_CallbackFuncs_t callbacks;
    bool shouldClose;
};



//- - FUNCTIONS

//- - - WINDOW-SPECIFIC

/*! \name Window-specific
 *  @{
 */

/*! \brief Initializes \p window with the given dimensions.
 *  \param[in]  window          Must remain in scope until \p Window_destr() is called.
 *  \param[in]  parentWindow    (Opt.) If provided, \p window will be a child of \p parentWindow.
 */
CORAL_API void Window_init(Window_t* window, size_t width, size_t height, Window_t* parentWindow);


/*! \brief Closes and destructs \p window. */
CORAL_API void Window_destr(Window_t* window);


/*! \brief Retrieves the dimensions of the drawable area of \p window.
 *  \param[out] width   The width of the drawable area.
 *  \param[out] height  The height of the drawable area.
 */
CORAL_API void Window_getDrawableDimensions(Window_t* window, size_t* width, size_t* height);


/*! \returns \c true if the mouse is restricted to \p window, otherwise \c false. */
CORAL_API bool Window_getMouseRestriction(Window_t* window);


/*! \brief Will restrict the mouse to \p window or release it from restriction.
 *  \param[in]  restrictMouse   If \c true, will hide the cursor and restrict the mouse such that the cursor can not
 *                              leave \p window. If \c false, will show the cursor and release the mouse such that it
 *                              can move freely again.
 *
 *  \note
 *      This function is merely a request to restrict the mouse; to find out whether the mouse really is restricted,
 *      check the value returned by \p Window_getMouseRestriction().
 */
CORAL_API void Window_setMouseRestriction(Window_t* window, bool restrictMouse);

/*! @} */



//- - - GLOBAL

/*! \name Global
 *  @{
 */

/*! \brief Processes events directed at the calling thread and all windows. */
CORAL_API void Window_processEvents(void);


/*! \returns \c true if all windows should close and the process should terminate, otherwise \c false. */
CORAL_API bool Window_allShouldClose(void);


/*! \returns
 *      \c true if windows can receive input through the respective callback functions and input can
 *      be queried through the \p Window_getMouseKeyPressed(), \p Window_getKeyboardKeyPressed(), and
 *      \p Window_getModifierFlags() functions; otherwise, \c false.
 *
 *  \note
 *      Should be checked before assuming the user will perform some input in order to close the program.
 *      If the return value is \c false, it can be assumed that the program is running headless, which means
 *      that no user input can occur.
 */
CORAL_API bool Window_isInputCapable(void);


/*! \returns \c true if \p key is pressed, otherwise \c false.
 *  \note
 *      Does not necessarily represent the real-time state of \p key, as the returned
 *      value may be dependent on calls to \p Window_processEvents().
 */
CORAL_API bool Window_getMouseKeyPressed(MouseInput_e key);


/*! \copydoc Window_getMouseKeyPressed */
CORAL_API bool Window_getKeyboardKeyPressed(KeyboardInput_e key);


/*! \returns The flags for the currently pressed modifier keys.
 *  \note
 *      Does not necessarily represent the real-time state of the modifier keys, as the
 *      returned value may be dependent on calls to \p Window_processEvents().
 */
CORAL_API uint16_t Window_getModifierFlags(void);

/*! @} */

/*! @} */



//- FRAMEBUFFER

/*!\defgroup CORAL_GRAPHICS_FRAMEBUFFER Framebuffer_t
 * \brief Represents content that may be drawn directly into a \p Window_t or used in other ways.
 * \addtogroup CORAL_GRAPHICS_FRAMEBUFFER
 * @{
 */



//- - DEFINITIONS

/*! \copybrief CORAL_GRAPHICS_FRAMEBUFFER */
typedef struct {
    uintptr_t windowResource;
    uintptr_t drawingResource;
    size_t width;           /*!< \note May be \c 0U. */
    size_t height;          /*!< \note May be \c 0U. */
    Vec4b_u* pixelBuffer;   /*!< \note May be \c NULL if at least one of \p witdh and \p height is \c 0U. */
} Framebuffer_t;



//- - FUNCTIONS

/*! \brief Initializes \p framebuffer such that it can be used to draw to \p window.
 *  \note \p framebuffer will be initialized with the dimensions provided by \p Window_getDrawableDimensions().
 */
CORAL_API void Framebuffer_init(Framebuffer_t* framebuffer, Window_t* window);


/*! \brief Initializes \p framebuffer with the dimensions \p width and \p height.
 *  \note \p framebuffer may not be able to be used to draw to a \p Window_t.
 */
CORAL_API void Framebuffer__init(Framebuffer_t* framebuffer, size_t width, size_t height);


/*! \brief Destructs \p framebuffer. */
CORAL_API void Framebuffer_destr(Framebuffer_t* framebuffer);


/*! \returns \c true if \p framebuffer can be used to draw to \p window, otherwise \c false. */
CORAL_API bool Framebuffer_isWindowDrawable(Framebuffer_t* framebuffer, Window_t* window);


/*! \brief Will make \p framebuffer be able to be used to draw to \p window.
 *  \note \p framebuffer may be able to be used to draw to \e only \p window.
 */
CORAL_API void Framebuffer_makeWindowDrawable(Framebuffer_t* framebuffer, Window_t* window);


/*! \brief Resizes \p framebuffer to the dimensions of \p window.
 *  \param[in]  window  May be any \p Window_t, \p framebuffer must not be able to be used to draw to \p window.
 *  \note Any image data stored in \p framebuffer may need to be redrawn.
 */
CORAL_API void Framebuffer_resize(Framebuffer_t* framebuffer, Window_t* window);


/*! \brief Resizes \p framebuffer to the dimensions described by \p newWidth and \p newHeight.
 *  \note Any image data stored in \p framebuffer may need to be redrawn.
 */
CORAL_API void Framebuffer__resize(Framebuffer_t* framebuffer, size_t newWidth, size_t newHeight);


/*! \brief Draws the contents of \p framebuffer to \p window.
 *  \param[in]  waitForVerticalSync     Will wait until the screen refreshes to present the window.
 *  \returns \c true if drawing succeeded, otherwise \c false.
 */
CORAL_API bool Framebuffer_drawToWindow(Framebuffer_t* framebuffer, Window_t* window, bool waitForVerticalSync);

/*! @} */

/*! @} */

#endif
