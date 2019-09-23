#ifndef _LCOM_I8042_H_
#define _LCOM_I8042_H_

/** @defgroup i8254 i8254
 * @{
 *
 * Constants for programming the i8042 KBD.
 */

#define BIT(n)          (0x01<<(n))

#define KBD_IRQ         1

#define INPUT_BUFFER    0x60

#define OUTPUT_BUFFER   0x60

#define ESC_BREAKCODE   0x81

#define TWO_BYTE        0xE0

#define INVAL_SCANCODE  0x00

//Defines relative to the status register

#define STATUS_REG      0x64

#define OBF             BIT(0)

#define IBF             BIT(1)

#define AUX             BIT(5)

#define PARITY_ERR      BIT(7)

#define TIMEOUT_ERR     BIT(6)

//Defines relative to the KBC

#define KBC_CMD_REG     0x64

#define KBC_READ_CMD    0x20

#define KBC_WRITE_CMD   0x60

#define KBC_EN_KBD      BIT(0)  //Bit that enables keyboard interrupts in KBC command byte

#endif
