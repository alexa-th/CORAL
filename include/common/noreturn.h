#ifndef CORAL_NORETURN

/*! \file */

#include "./compilers.h"



// NORETURN

#ifdef __cplusplus
#	if CORAL_IS_COMPILER(CORAL_COMPILER_MSVC)
#		define CORAL_NORETURN __declspec(noreturn)
#	else
#		define CORAL_NORETURN [[noreturn]]
#	endif
#else
#   define CORAL_NORETURN _Noreturn
#endif

#endif

