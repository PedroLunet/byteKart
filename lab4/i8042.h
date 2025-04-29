#ifndef _LCOM_I8042_H_
#define _LCOM_I8042_H_

#include <lcom/lcf.h>

/** @defgroup i8042 i8042
 * @{
 *
 * Constants for programming the i8042 Keyboard.
 */

#define IRQ_KEYBOARD 1

#define ESC_MAKECODE 0x01
#define ESC_BREAKCODE 0x81

#define PARITY_STATUS BIT(7)
#define TIMEOUT_STATUS BIT(6)
#define AUX_MOUSE BIT(5)
#define INH BIT(4)
#define INPUT_BUFFER_FULL BIT(1)
#define OUTPUT_BUFFER BIT(0)

#define OUT_BUFFER 0x60
#define IN_BUFFER 0x64
#define READ_COMMAND_BYTE 0x20
#define WRITE_COMMAND_BYTE 0x60
#define KBC_ST_REG 0x64
#define KBC_CMD_REG 0x64

#define DELAY_US 20000


/* MOUSE DEFINITIONS */

#define IRQ_MOUSE 12

#define MOUSE_Y_OVFL BIT(7)
#define MOUSE_X_OVFL BIT(6)
#define MSB_Y_DELTA BIT(5)
#define MSB_X_DELTA BIT(4)
#define MOUSE_MIDDLE_BUTTON BIT(2)
#define MOUSE_RIGHT_BUTTON BIT(1)
#define MOUSE_LEFT_BUTTON BIT(0)

#define BUTTON_PRESSED 1

#define MOUSE_COMMAND 0xD4
#define MOUSE_ENABLE 0xF4
#define MOUSE_DISABLE 0xF5

#endif
