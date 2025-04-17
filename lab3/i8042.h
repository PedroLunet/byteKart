#ifndef _LCOM_I8042_H_
#define _LCOM_I8042_H_

/** @defgroup i8042 i8042
 * @{
 *
 * Constants for programming the i8042 Keyboard.
 */

#define KEYBOARD_IRQ 1 /**< @brief Keyboard IRQ line */

#define ESC_BREAKCODE 0x81 /**< @brief Breakcode for ESC key */
#define MAKE_CODE BIT(7) /**< @brief Make code for key press */
#define TWO_BYTES 0xE0 /**< @brief Two bytes scancode */
#define ENABLE_INT BIT(0) /**< @brief Enable interrupts */
#define BREAK_BIT BIT(7) /**< @brief Break code bit */

#define KBC_TO_ERR BIT(6) /**< @brief Timeout error */
#define KBC_PAR_ERR BIT(7) /**< @brief Parity error */
#define KBC_ST_IBF BIT(1) /**< @brief Input buffer full */
#define KBC_ST_OBF BIT(0) /**< @brief Output buffer full */
#define KBC_ST_AUX BIT(5) /**< @brief Mouse data */

#define KBC_ST_REG 0x64 /**< @brief Status register */
#define KBC_OUT_CMD 0x60 /**< @brief Output buffer */
#define KBC_IN_CMD 0x64 /**< @brief Input buffer */
#define KBC_READ_CMD 0x20 /**< @brief Read command byte */
#define KBC_WRITE_CMD 0x60 /**< @brief Write command byte */

#define DELAY 20000 /** @brief Delay for input/output buffer read */
#define MAX_ATTEMPS 10 /**< @brief Max attempts to read/write from/to KBC */

/**@}*/

#endif /* _LCOM_I8042_H_ */
