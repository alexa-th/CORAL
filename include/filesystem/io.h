#ifndef CORAL_FILESYSTEM_IO
#define CORAL_FILESYSTEM_IO

/*! \file */

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include "../common/api.h"
#include "../common/error.h"
#include "./file.h"



// IO OPERATIONS

/*! \defgroup CORAL_FILESYSTEM_IO I/O operations
 *  \ingroup CORAL_FILESYSTEM
 *  \brief All functions that write to or read from a \p File_t.
 *  \addtogroup CORAL_FILESYSTEM_IO
 *  @{
 */



//- DEFINITIONS

/*! \brief Size to which read operations are usually aligned to. */
#define CORAL_FILESYSTEM_IO_OS_BUFFER_SIZE  ((size_t)BUFSIZ)



//- SYNCHRONOUS

/*! \defgroup CORAL_FILESYSTEM_IO_SYNC Synchronous operations
 *  \addtogroup CORAL_FILESYSTEM_IO_SYNC
 *  @{
 */

/*! \brief Reads at most \p size bytes into \mlink{File_t,file->,readBuffer}.
 *  \param[in]  size                If \c 0U, will not stop reading after a certain amount of bytes have been read.
 *  \param[out] read                (Opt.) If provided, set to the number of bytes read, which may be less than \p size
 *                                  if the return value indicates a partial read.
 *  \param[in]  stopBeforeResize    If \c true, will resize on the first iteration if needed and read until another
 *                                  resize is required or a return condition is reached.
 *
 *  \returns
 *      \c 0U on success, \verblink{CORAL_FILESYSTEM_ERROR_EOF} on a partial read caused by encountering EOF,
 *      \verblink{CORAL_FILESYSTEM_ERROR_UNFINISHED} on a partial read caused by encountering a resize with
 *      \p stopBeforeResize set to \c true, a different nonzero \ref CORAL_FILESYSTEM_ERROR_ "error code" on failure.
 */
CORAL_API Error_t IO_read(File_t* file, size_t size, size_t* read, bool stopBeforeResize);


/*! \brief Reads the entire \p file into \mlink{File_t,file->,readBuffer}.
 *  \returns \p CORAL_ERROR_NONE on success, a nonzero \ref CORAL_FILESYSTEM_ERROR_ "error code" on failure.
 */
CORAL_API Error_t IO_readComplete(File_t* file);


/*! \brief Writes \p size bytes beginning at \p source to \p file.
 *  \param[out] written     (Opt.) Will be set to the number of bytes written to the file, which may be less than
 *                          \p size if the return value indicates failure.
 *
 *  \returns \p CORAL_ERROR_NONE on success, a nonzero \ref CORAL_FILESYSTEM_ERROR_ "error code" on failure.
 *  \note
 *      If the return value indicates failure and the file mode of \p file does not include
 *      \verblink{CORAL_FILESYSTEM_FILE_MODE_NO_REPLICATE_ON_WRITE}, the same amount if bytes written to the underlying
 *      file, if any, will have been written to \mlink{File_t,file->,readBuffer} aswell.
 */
CORAL_API Error_t IO_write(File_t* file, const void* source, size_t size, size_t* written);

/*! @} */

/*! @} */

#endif
