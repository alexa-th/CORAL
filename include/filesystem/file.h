#ifndef CORAL_FILESYSTEM_FILE
#define CORAL_FILESYSTEM_FILE

/*! \file */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include "../common/api.h"
#include "../common/error.h"
#include "../containers/buffer.h"
#include "./path.h"



// FILE

/*! \defgroup CORAL_FILESYSTEM_FILE File_t
 *  \ingroup CORAL_FILESYSTEM
 *  \copybrief File_t
 *  \addtogroup CORAL_FILESYSTEM_FILE
 *  @{
 */



//- DEFINITIONS

/*! \anchor CORAL_FILESYSTEM_FILE_MODE_
 *  \name File modes
 *  \note The mode of a \p File_t will be stored in \mlink{String_t,filePath.pathStr.,additionalData} at index \c 1U.
 *  @{
 */

/*! \brief Reads from the beginning of a file. */
#define CORAL_FILESYSTEM_FILE_MODE_READ                 ((uint8_t)0x01)


/*! \note Not to be used on its own, use \verblink{CORAL_FILESYSTEM_FILE_MODE_WRITE} to write to files. */
#define CORAL_FILESYSTEM_FILE_MODE_WRITE_BASE           ((uint8_t)0x02)


/*! \brief Starts writing at the beginning of a file. */
#define CORAL_FILESYSTEM_FILE_MODE_WRITE                (CORAL_FILESYSTEM_FILE_MODE_WRITE_BASE | (uint8_t)0x04)


/*! \brief Starts writing at the end of a file. */
#define CORAL_FILESYSTEM_FILE_MODE_APPEND               (CORAL_FILESYSTEM_FILE_MODE_WRITE_BASE | (uint8_t)0x08)


/*! \brief Must be specified if a file that does not yet exist is to be opened in read-write mode. */
#define CORAL_FILESYSTEM_FILE_MODE_READ_WRITE_CREATE    (CORAL_FILESYSTEM_FILE_MODE_READ | CORAL_FILESYSTEM_FILE_MODE_WRITE | (uint8_t)0x10)


/*! \brief The opened file is treated as binary data and not text. */
#define CORAL_FILESYSTEM_FILE_MODE_BINARY               ((uint8_t)0x20)


/*! \brief If specified, any \p File_init() function will fail if the file to be opened already exists.
 *  \note Only affects file opening behaviour when specified in combination with \verblink{CORAL_FILESYSTEM_FILE_MODE_WRITE} or \verblink{CORAL_FILESYSTEM_FILE_MODE_READ_WRITE_CREATE}.
 */
#define CORAL_FILESYSTEM_FILE_MODE_NO_OVERWRITE         ((uint8_t)0x40)


/*! \brief Will cause any write operation on a \p File_t to not be performed on its \mlink{File_t,readBuffer} aswell. */
#define CORAL_FILESYSTEM_FILE_MODE_NO_REPLICATE_ON_WRITE ((uint8_t)0x80)


/*! \brief Checks whether \p FILE_MODE includes the given \p MODE. */
#define CORAL_FILESYSTEM_FILE_HAS_MODE(FILE_MODE, MODE) (((FILE_MODE) & (MODE)) == (MODE))

/*! @} */


/*! \brief Maximum size a file can occupy. */
#define CORAL_FILESYSTEM_FILE_MAX_FILE_SIZE ((size_t)LONG_MAX)


/*! \brief Amount of temporary files that can at most be created using \p File_init_tempfile(). */
#define CORAL_FILESYSTEM_FILE_MAX_TMP_FILES ((unsigned long long)TMP_MAX)


/*! \brief Represents a file in the filesystem which can be read from and written to. */
typedef struct {
    Path_t filePath;
    Buffer_t readBuffer;    /*!< \brief Will be used to perform read operations. */
    FILE* filePtr;
} File_t;



//- FUNCTIONS

/*! \brief Initializes \p file by opening the file described by \p file->filePath.
 *  \param[in]  file        \p file->filePath must already be initialized to a path that is not a directory.
 *  \param[in]  fileMode    Mode to open the file in; see \ref CORAL_FILESYSTEM_FILE_MODE_ "File modes".
 *  \returns
 *      \p CORAL_ERROR_NONE on success, \p CORAL_ERROR_INVALID_ARGS if an invalid \p fileMode was provided,
 *      \p CORAL_ERROR_UNKOWN if file opening failed.
 */
CORAL_API Error_t File_init(File_t* file, uint8_t fileMode);


/*! \brief Initializes \p file by opening a temporary file.
 *  \param[in]  file        In contrast to \p File_init, \p file->filePath should \e not already be initialized.
 *  \param[in]  fileMode    Mode to open the temporary file in; see \ref CORAL_FILESYSTEM_FILE_MODE_ "File modes".
 *  \returns
 *      \p CORAL_ERROR_NONE on success, \p CORAL_ERROR_INVALID_ARGS if an invalid \p fileMode was provided,
 *      \p CORAL_FILESYSTEM_ERROR_TMP_FILE_LIMIT if the OS can not provide a suitable filename or if
 *      \verblink{CORAL_FILESYSTEM_FILE_MAX_TMP_FILES} has been reached.
 *
 *  \note
 *      When specifying \p fileMode, keep in mind that the temporary file will not exist prior to opening it,
 *      which may lead to errors if \p fileMode requires the file to already exist.
 */
CORAL_API Error_t File_init_tempFile(File_t* file, uint8_t fileMode);


/*! \brief Destructs \p file.
 *  \param[in]  deleteFile  If \c true, deletes the underlying file from the filesystem.
 */
CORAL_API void File_destr(File_t* file, bool deleteFile);


/*! \brief Reopens \p file using \p newFileMode.
 *  \returns
 *      \p CORAL_ERROR_NONE on success, \p CORAL_ERROR_INVALID_ARGS if an invalid \p fileMode was provided,
 *      \p CORAL_ERROR_UNKOWN if file reopening failed.
 *
 *  \note If \p CORAL_ERROR_UNKOWN is returned, \p file may be invalid and and therefore unusable.
 *  \note \mlink{File_t,file->,readBuffer} and its metadata will be preserved.
 *  \see \ref CORAL_FILESYSTEM_FILE_MODE_ "File modes".
 */
CORAL_API Error_t File_changeMode(File_t* file, uint8_t newFileMode);

/*! @} */

#endif
