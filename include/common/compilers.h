#ifndef CORAL_COMMON_COMPILERS
#define CORAL_COMMON_COMPILERS

/*! \file */



// COMPILERS

/*! \defgroup CORAL_COMMON_COMPILERS Compilers
 *  \ingroup CORAL_COMMON
 *  \addtogroup CORAL_COMMON_COMPILERS
 *  @{
 */

#define CORAL_COMPILER_NONE    (0)
#define CORAL_COMPILER_OTHER   (1)
#define CORAL_COMPILER_GCC     (2)
#define CORAL_COMPILER_CLANG   (4)
#define CORAL_COMPILER_MSVC    (8)

#define CORAL_IS_COMPILER(COMPILER) (CORAL_COMPILER & COMPILER)


/*! \def CORAL_COMPILER
 *  \brief The compiler currently in use.
 *  \see \ref CORAL_COMMON_COMPILERS "Compilers" for all possible values.
 */

#if defined(DOXYGEN)
#   define CORAL_COMPILER CORAL_COMPILER_NONE
#elif defined(_MSC_VER)
#   define CORAL_COMPILER CORAL_COMPILER_MSVC
#elif defined(__clang__)
#   define CORAL_COMPILER CORAL_COMPILER_CLANG
#elif defined(__GNUC__)
#   define CORAL_COMPILER CORAL_COMPILER_GCC
#else
#   define CORAL_COMPILER CORAL_COMPILER_OTHER
#endif

/*! @} */

#endif
