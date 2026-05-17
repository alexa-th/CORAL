#ifndef CORAL_COMMON_RANDOM
#define CORAL_COMMON_RANDOM

/*! \file */

#include "./api.h"

/*! \defgroup CORAL_COMMON_RANDOM Pseudo-random number generation
 *  \ingroup CORAL_COMMON
 *  \brief Functions to generate pseudo-random numbers of multiple types and ranges.
 *  \addtogroup CORAL_COMMON_RANDOM
 *  @{
 */



// PSEUDO-RANDOM NUMBER GENERATION

//- FUNCTIONS

/*! \returns
 *      A pseudo-random \c int in the range of \c INT_MIN to \c INT_MAX,
 *      including both \c INT_MIN and \c INT_MAX.
 */
CORAL_API int Random_int(void);


/*! \returns
 *      A pseudo-random \c int in the range of \p min to \p max,
 *      including both \p min and \p max.
 */
CORAL_API int Random__int(int min, int max);


/*! \returns
 *      A pseudo-random \c unsigned in the range of \c 0U to \c UINT_MAX,
 *      including both \c 0U and \c UINT_MAX.
 */
CORAL_API unsigned Random_unsigned(void);


/*! \returns
 *      A pseudo-random \c unsigned in the range of \p min to \p max,
 *      including both \p min and \p max.
 */
CORAL_API unsigned Random__unsigned(unsigned min, unsigned max);


/*! \returns
 *      A pseudo-random \c float in the range of \c -FLT_MAX to \c FLT_MAX,
 *      including both \c -FLT_MAX and \c FLT_MAX.
 */
CORAL_API float Random_float(void);


/*! \returns
 *      A pseudo-random \c float in the range of \p min to \p max,
 *      including both \p min and \p max.
 */
CORAL_API float Random__float(float min, float max);


/*! \returns
 *      A pseudo-random \c double in the range of \c -DBL_MAX to \c DBL_MAX,
 *      including both \c -DBL_MAX and \c DBL_MAX.
 */
CORAL_API double Random_double(void);


/*! \returns
 *      A pseudo-random \c double in the range of \p min to \p max,
 *      including both \p min and \p max.
 */
CORAL_API double Random__double(double min, double max);

/*! @} */

#endif
