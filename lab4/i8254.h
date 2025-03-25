#ifndef _LCOM_I8254_H_
#define _LCOM_I8254_H_

#include <lcom/lcf.h>

/** @defgroup i8254 i8254
 * @{
 *
 * Constants for programming the i8254 Timer. Needs to be completed.
 */

#define TIMER_FREQ 1193182 /**< @brief clock frequency for timer in PC and AT */
#define TIMER0_IRQ 0 /**< @brief Timer 0 IRQ line */

/* I/O port addresses */

#define TIMER_0    0x40 /**< @brief Timer 0 count register */
#define TIMER_1    0x41 /**< @brief Timer 1 count register */
#define TIMER_2    0x42 /**< @brief Timer 2 count register */
#define TIMER_CTRL 0x43 /**< @brief Control register */

#define SPEAKER_CTRL 0x61 /**< @brief Register for speaker control  */

/* Timer control */

/* Timer selection: bits 7 and 6 */

#define TIMER_SEL0   0x00              /**< @brief Control Word for Timer 0 */
#define TIMER_SEL1   BIT(6)            /**< @brief Control Word for Timer 1 */
#define TIMER_SEL2   BIT(7)            /**< @brief Control Word for Timer 2 */
#define TIMER_RB_CMD (BIT(7) | BIT(6)) /**< @brief Read Back Command */

/* Register selection: bits 5 and 4 */

#define TIMER_LSB     BIT(4)                  /**< @brief Initialize Counter LSB only */
#define TIMER_MSB     BIT(5)                  /**< @brief Initialize Counter MSB only */
#define TIMER_LSB_MSB (TIMER_LSB | TIMER_MSB) /**< @brief Initialize LSB first and MSB afterwards */

/* Operating mode: bits 3, 2 and 1 */

#define TIMER_SQR_WAVE (BIT(2) | BIT(1)) /**< @brief Mode 3: square wave generator */
#define TIMER_RATE_GEN BIT(2)            /**< @brief Mode 2: rate generator */

/* Counting mode: bit 0 */

#define TIMER_BCD 0x01 /**< @brief Count in BCD */
#define TIMER_BIN 0x00 /**< @brief Count in binary */

/* READ-BACK COMMAND FORMAT */

#define TIMER_RB_COUNT_  BIT(5)
#define TIMER_RB_STATUS_ BIT(4)
#define TIMER_RB_SEL(n)  BIT((n) + 1)

/**@}*/


/** @defgroup i8254 i8254
 * @{
 *
 * Constants for programming the i8254 Keyboard.
 */

#define KBC_IRQ 1 /**< @brief Keyboard IRQ line */

#define KBC_OUT_BUF 0x60 /**< @brief Output buffer */
#define KBC_IN_BUF 0x60 /**< @brief Input buffer */
#define KBC_CMD_REG 0x64 /**< @brief Command register */
#define KBC_ST_REG 0x64 /**< @brief Status register */

#define KBC_ST_IBF BIT(1) /**< @brief Input buffer full */
#define KBC_ST_OBF BIT(0) /**< @brief Output buffer full */
#define KBC_ST_AUX BIT(5) /**< @brief Mouse data */

#define KBC_READ_CMD 0x20 /**< @brief Read command byte */
#define KBC_WRITE_CMD 0x60 /**< @brief Write command byte */

#define KBC_PAR_ERR BIT(7) /**< @brief Parity error */
#define KBC_TO_ERR BIT(6) /**< @brief Timeout error */

#define BREAK_BIT BIT(7) /**< @brief Make bit */

#define KBC_ST_CBYTE BIT(0) /**< @brief Command byte */

#define ESC_BREAKCODE 0x81 /**< @brief Breakcode for ESC key */

#define DELAY_US 20000 /** @brief Delay for input/output buffer read */

/**@}*/

/** @defgroup i8254 i8254
 * @{
 *
 * Constants for programming the i8254 Mouse.
 */

#define MOUSE_IRQ 12 /**< @brief Mouse IRQ line */

#define MOUSE_WRITE_CMD 0xD4 /**< @brief Mouse command */

#define MOUSE_ENABLE_CMD 0xF4 /**< @brief Enable data reporting */
#define MOUSE_DISABLE_CMD 0xF5 /**< @brief Disable data reporting */

#define ACK 0xFA /**< @brief Acknowledgement byte */
#define NACK  0xFE /**< @brief Resend byte */

/**@}*/

#endif /* _LCOM_I8254_H */
