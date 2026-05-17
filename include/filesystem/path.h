#ifndef CORAL_FILESYSTEM_PATH
#define CORAL_FILESYSTEM_PATH

/*! \file */

#include <stddef.h>
#include <stdbool.h>
#include "../common/api.h"
#include "../common/error.h"
#include "../containers/string.h"



// PATH

/*! \defgroup CORAL_FILESYSTEM_PATH Path_t
 *  \ingroup CORAL_FILESYSTEM
 *  \copybrief Path_t
 *  \addtogroup CORAL_FILESYSTEM_PATH
 *  @{
 */



//- DEFINITIONS

/*! \anchor CORAL_FILESYSTEM_PATH_TYPE_
 *  \name Path_t types
 *  @{
 */

/*! \brief The type of a \p Path_t.
 *  \note The \p PathType_e of a \p Path_t is stored in \mlink{String_t,pathStr.,additionalData} at index \c 0U.
 */
typedef enum {
    CORAL_FILESYSTEM_PATH_TYPE_WINDOWS = 1, /*!< \brief Windows style path strings; using \c '\\' as the directory seperator. */
    CORAL_FILESYSTEM_PATH_TYPE_UNIX         /*!< \brief UNIX style path strings; using \c '/' as the directory seperator. */
} PathType_e;

/*! @} */


/*! \def CORAL_FILESYSTEM_PATH_MAX_LENGTH
 *  \brief Maximum length of a path for the current target.
 */

#if CORAL_IS_TARGET(CORAL_TARGET_WIN64)
#   include <stdio.h>
#   define CORAL_FILESYSTEM_PATH_MAX_LENGTH     FILENAME_MAX
#elif CORAL_IS_TARGET(CORAL_TARGET_LINUX)
#   include <limits.h>
#   define CORAL_FILESYSTEM_PATH_MAX_LENGTH     _POSIX_PATH_MAX
#elif CORAL_IS_TARGET_STRICT(CORAL_TARGET_NONE)
#   define CORAL_FILESYSTEM_PATH_MAX_LENGTH     (0)
#endif


/*! \brief Describes a path in the filesystem.
 *
 *  A \p Path_t is divided into three segments: the directory, the file name, and the file extension.
 *  A \p Path_t is considered to be a directory if \mlink{Path_t,directoryLength} is equal to <tt>pathStr.used</tt>.
 */
typedef struct {
    String_t pathStr;       /*!< \brief The underlying path. */
    size_t directoryLength; /*!< \brief The length of the directory, including the last directory seperator (e.g \c '/' on UNIX). */
    size_t extensionLength; /*!< \brief The length of the file extension, including the period preceeding the file extension. */
} Path_t;



//- FUNCTIONS

/*! \brief Initializes \p path from \p rawPathStr.
 *  \param[in]  rawPathStrUsed      If \c 0U, the size of \p rawPathStr will be determined.
 *  \param[in]  rawPathStrCapacity  If \c 0U, the provided or determined \p rawPathStrUsed will be used to calculate the capacity.
 *  \param[in]  makeDynamic         If \c true, will dynamically allocate a string with a capacity of at least \p rawPathStrCapacity
 *                                  and copy \p rawPathStr into that dynamic string; otherwise, will use \p rawPathStr directly.
 *  \param[in]  isConst             Whether \p rawPathStr is constant; if \p makeDynamic is \c true, will have no effect on the allocated string.
 *  \returns
 *      \p CORAL_ERROR_NONE on success, \p CORAL_ERROR_INVALID_ARGS if a non-zero \p rawPathStrCapacity was provided
 *      and was less than or equal to the provided or determined \p rawPathStrUsed or if \p rawPathStr was larger than
 *      \p CORAL_FILESYSTEM_PATH_MAX_LENGTH, \p CORAL_ERROR_OOM if \p makeDynamic was \c true and an allocation error occurred.
 */
CORAL_API Error_t Path_init(Path_t* path, const char* rawPathStr, size_t rawPathStrUsed, size_t rawPathStrCapacity, bool makeDynamic, bool isConst);


/*! \brief Initializes \p path as relative to \p baseDirectory.
 *  \param[in]  baseDirectory       Must be a directory.
 *  \param[in]  rawPathStr          Even though it is of the type <tt>const char*</tt>, the string must be mutable if \p makeDynamic is \c false.
 *  \param[in]  rawPathStrUsed      If \c 0U, the size of \p rawPathStr will be determined.
 *  \param[in]  rawPathStrCapacity  Must be at least <tt>basePath->pathStr.used + rawPathStrUsed + 1U</tt>.
 *  \returns
 *      \p CORAL_ERROR_NONE on success, \p CORAL_ERROR_INVALID_ARGS if \p rawPathStr was larger than \p CORAL_FILESYSTEM_PATH_MAX_LENGTH after prepending \p baseDirectory->pathStr,
 *      \p CORAL_ERROR_OOM if \p makeDynamic was \c true and an allocation error occurred or if \p makeDynamic was \c false and \p rawPathStrCapacity was too small.
 */
CORAL_API Error_t Path_init_relative(Path_t* path, Path_t* baseDirectory, const char* rawPathStr, size_t rawPathStrUsed, size_t rawPathStrCapacity, bool makeDynamic);


/*! \brief Destructs \p path. */
CORAL_API void Path_destr(Path_t* path);

/*! @} */

#endif
