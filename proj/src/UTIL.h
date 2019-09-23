#ifndef _UTIL_H_
#define _UTIL_H_

/** @defgroup Utility Utility
 * @{
 *
 * @brief Module with some useful general macros
 */

#define BIT(n)      (0x01<<(n))

#define LSB(n)      (n & 0x00FF)

#define MSB(n)      (n>>8)

#define LSB2(n)     (n & 0x0000FFFF)

#define MSB2(n)     (n>>8)

#endif /* _UTIL_H */
