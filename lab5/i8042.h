#ifndef _LCOM_I8042_H_
#define _LCOM_I8042_H_

/** @defgroup i8254 i8254
 * @{
 *
 * Constants for programming the i8042 KBD.
 */

// UTILITY

#define BIT(n)          (0x01<<(n))

#define SE_8_TO_16      0xFF00

// IRQ

#define KBD_IRQ         1

#define MOUSE_IRQ       12

//Defines relative to the KBC

#define STATUS_REG      0x64

#define INPUT_BUFFER    0x60

#define OUTPUT_BUFFER   0x60

#define OBF             BIT(0)

#define IBF             BIT(1)

#define AUX             BIT(5)

#define PARITY_ERR      BIT(7)

#define TIMEOUT_ERR     BIT(6)

// Defines relative to the keyboard

#define ESC_BREAKCODE   0x81

#define TWO_BYTE        0xE0

#define INVAL_SCANCODE  0x00

#define KBC_EN_KBD      BIT(0)

// Defines relative to KBC commands

#define KBC_CMD_REG     0x64

#define KBC_READ_CMD    0x20

#define KBC_WRITE_CMD   0x60

// Defines relative to mouse

#define ACK             0xFA

#define KBC_WRITE_MOUSE 0xD4

#define KBC_EN_DATA     0xF4

#define KBC_DIS_DATA    0xF5

#define KBC_SET_STREAM  0xEA

#define KBC_READ_MOUSE  0xEB

#define KBC_SET_REMOTE  0xF0

#define KBC_INT_MOUSE   BIT(1)

// Event handling

#define MOUSE_EVT       BIT(0)

#endif
