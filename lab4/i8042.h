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

/** @defgroup i8042 i8042
 * @{
 *
 * Constants for programming the i8042 Mouse.
 */

#define MOUSE_IRQ 12 /**< @brief Mouse IRQ line */

#define MOUSE_LB BIT(0) /**< @brief Left button */
#define MOUSE_RB BIT(1) /**< @brief Right button */
#define MOUSE_MB BIT(2) /** @brief Middle button */
#define FIRST_BYTE BIT(3) /** @brief First byte of mouse packet */
#define MOUSE_X_SIGNAL BIT(4) /** @brief X-axis movement signal */
#define MOUSE_Y_SIGNAL BIT(5) /** @brief Y-axis movement signal */
#define MOUSE_X_OVERFLOW BIT(6) /** @brief X-axis overflow */
#define MOUSE_Y_OVERFLOW BIT(7) /** @brief Y-axis overflow */
#define MOUSE_WRITE_CMD 0xD4 /**< @brief Mouse command */
#define MOUSE_ENABLE_CMD 0xF4 /**< @brief Enable data reporting */
#define MOUSE_DISABLE_CMD 0xF5 /**< @brief Disable data reporting */

#define ACK 0xFA /**< @brief Acknowledgement byte */
#define NACK  0xFE /**< @brief Resend byte */

/**@}*/

#endif /* _LCOM_I8042_H_ */
