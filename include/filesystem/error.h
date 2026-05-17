#ifndef CORAL_FILESYSTEM_ERROR
#define CORAL_FILESYSTEM_ERROR

/*! \file */

#include "../common/error.h"



// FILESYSTEM ERROR CODES

/*! \defgroup CORAL_FILESYSTEM_ERROR_ Filesystem error codes
 *  \ingroup CORAL_FILESYSTEM
 *  \ingroup CORAL_COMMON_ERROR
 *  \brief Error codes returned by functions of the filesystem sub-library that can fail at runtime.
 *  \see \ref CORAL_ERROR_ "Common error codes" for more error codes.
 *  \addtogroup CORAL_FILESYSTEM_ERROR_
 *  @{
 */

/*! \brief No error occured, but the operation encountered EOF before fully finishing. */
#define CORAL_FILESYSTEM_ERROR_EOF                  ((Error_t)0x10)

/*! \brief No error occured, but the requested task was not finished; an optional \p maxIterations parameter was likely reached. */
#define CORAL_FILESYSTEM_ERROR_UNFINISHED           ((Error_t)0x11)

/*! \brief Attempted to read a file that is too large to be handled or write to a file such that it would become too large to be handled. */
#define CORAL_FILESYSTEM_ERROR_TOO_LARGE            ((Error_t)0x12)

/*! \brief The OS cannot provide more temporary files. */
#define CORAL_FILESYSTEM_ERROR_TMP_FILE_LIMIT       ((Error_t)0x13)

/*! @} */

#endif
