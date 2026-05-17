#ifndef CORAL_API

/*! \file */

#include "./targets.h"



// CORAL API

/*! \def CORAL_API
 *  \ingroup CORAL_COMMON
 *  \brief Definition to make clear which functions and variables are part of the API.
 *  \note Only really needed on Windows.
 */


#if CORAL_IS_TARGET(CORAL_TARGET_WIN32)
#   ifdef CORAL_EXPORTS
#       define CORAL_API __declspec(dllexport)
#   else
#       define CORAL_API __declspec(dllimport)
#   endif
#else
#   define CORAL_API
#endif



// C++ COMPATIBILITY

#ifdef __cplusplus
#   define restrict
#endif

#endif
