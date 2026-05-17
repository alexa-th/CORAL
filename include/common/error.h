#ifndef CORAL_COMMON_ERROR
#define CORAL_COMMON_ERROR

/*! \file */



// ERRORS

/*! \defgroup CORAL_COMMON_ERROR Errors
 *  \brief Error type definition and error codes returned by functions that can fail at runtime.
 *  \addtogroup CORAL_COMMON_ERROR
 *  @{
 */



//- ERROR TYPE

/*! \brief An error code definition that follows the C17 standard definition. */
typedef int Error_t;



//- COMMON ERROR CODES

/*! \defgroup CORAL_ERROR_ Common error codes
 *  \addtogroup CORAL_ERROR_
 *  @{
 */

#define CORAL_ERROR_NONE            ((Error_t)0)
#define CORAL_ERROR_UNKOWN          ((Error_t)-1)
#define CORAL_ERROR_INVALID_ARGS    ((Error_t)0x1)
#define CORAL_ERROR_OOM             ((Error_t)0xE)
#define CORAL_ERROR_OTHER           ((Error_t)0xF)

/*! @} */

/*! @} */

#endif
