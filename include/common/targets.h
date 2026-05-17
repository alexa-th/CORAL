#ifndef CORAL_COMMON_TARGETS
#define CORAL_COMMON_TARGETS

/*! \file */



// TARGETS

/*! \defgroup CORAL_COMMON_TARGETS Targets
 *  \ingroup CORAL_COMMON
 *  \addtogroup CORAL_COMMON_TARGETS
 *  @{
 */

#define CORAL_TARGET_NONE       (0)
#define CORAL_TARGET_OTHER      (1)
#define CORAL_TARGET_LINUX      (2)
#define CORAL_TARGET_WIN_BASE   (4)
#define CORAL_TARGET_WIN32      (CORAL_TARGET_WIN_BASE | 8)
#define CORAL_TARGET_WIN64      (CORAL_TARGET_WIN_BASE | 16)

#define CORAL_IS_TARGET(TARGET)         (CORAL_TARGET & (TARGET))
#define CORAL_IS_TARGET_STRICT(TARGET)  (CORAL_TARGET == (TARGET))


/*! \def CORAL_TARGET
 *  \brief The target of the current compilation.
 *  \see \ref CORAL_COMMON_TARGETS "Targets" for all possible values.
 */

#if defined(DOXYGEN)
#   define CORAL_TARGET CORAL_TARGET_NONE
#elif defined(_WIN32)
#   if defined(_WIN64)
#       define CORAL_TARGET CORAL_TARGET_WIN64
#   else
#       define CORAL_TARGET CORAL_TARGET_WIN32
#   endif
#elif defined(__linux__)
#   define CORAL_TARGET CORAL_TARGET_LINUX
#else
#   define CORAL_TARGET CORAL_TARGET_OTHER
#endif

/*! @} */

#endif
