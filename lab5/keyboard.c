#include <lcom/lcf.h>
#include "i8254.h"
#include "i8042.h"

uint32_t sys_inb_count = 0; // Counts ammount of sys_inb calls used for interrupts/polling
uint32_t scancode; // Stores scancode read in last iteration

#ifdef LAB3
int sys_inb_cnt(port_t port, uint32_t *byte) // Wrapper function of sys_inb to count sys_inb calls
{
  sys_inb_count++;
  if (sys_inb(port, byte))
    return 1;
  else
    return 0;
}
#else
#define sys_inb_cnt(p, q) sys_inb(p, q)
#endif

int kb_hook_id = 1; // hook_id used by the keyboard (timer uses 0)

int (keyboard_subscribe_int)(uint8_t *bit_no)
{
  *bit_no = kb_hook_id;

  if (sys_irqsetpolicy(KEYBOARD_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &kb_hook_id) == OK)
    return 0;
  else
    return 1;
}

int(keyboard_unsubscribe_int)()
{
  if (sys_irqrmpolicy(&kb_hook_id) == OK)
    return 0;
  else
    return 1;
}

bool(gotFullScancode)(uint8_t scancodeArray[], uint8_t *size)
{
  // Scancode "0" is used as an error indicator, meaning either nothing was read
  // in the last iteration or what was read was invalid
  if (scancode == 0)
  {
    return false;
  }
  // If scancode "0xE0" is read, we don't have the full scancode yet so wait for
  // the next iteration
  if (scancode == TWO_BYTE)
  {
    scancodeArray[0] = TWO_BYTE;
    *size = 2;
    return false;
  }
  // If scancode is neither 0x00 nor 0xE0, its a valid scancode and it's ready to
  // be printed, with its position depending on current scancode size (1 or 2)
  else
  {
    if (*size == 2)
      scancodeArray[1] = (uint8_t)scancode;
    else
      scancodeArray[0] = (uint8_t)scancode;
  }
  return true;
}

int(readScancode)()
{
  // Polling tecnique; try to read contents of output buffer 5 times
  for (uint8_t attempts = 5; attempts > 0; attempts--)
  {
    // Read status
    uint32_t status;
    sys_inb_cnt(STATUS_REG, &status);
    if ((status & OBF) && !(status & AUX))
    {
      // If there's something in the buffer and no errors, call IH
      kbc_ih();
      return 0;
    }
    // If OBF flag isn't set, wait a bit and try again
    tickdelay(micros_to_ticks(DELAY_US));
  }
  // Once the 5 tries are done, set scancode to invalid and move on
  scancode = INVAL_SCANCODE;
  return 1;
}

void(kbc_ih)()
{
  // Read status
  uint32_t status;
  sys_inb_cnt(STATUS_REG, &status);
  // Move contents of buffer to scancode
  sys_inb_cnt(OUTPUT_BUFFER, &scancode);
  // If an error was detected, discard what was put into scancode
  if (status & (PARITY_ERR | TIMEOUT_ERR))
  {
    scancode = INVAL_SCANCODE;
  }
}

int (enableKeyboardInterrupts)()
{
  uint32_t KBC_CMD;
  uint32_t status;

  // Send read command to KBC and read current command byte
  sys_outb(KBC_CMD_REG, KBC_READ_CMD);
  sys_inb(OUTPUT_BUFFER, &KBC_CMD);

  // Set "enable keyboard interrupts" bit in KBC command byte
  KBC_CMD |= KBC_EN_KBD;

  // Read status
  sys_inb(STATUS_REG, &status);

  // If input buffer  is full, return an error
  if (status & IBF)
    return 1;

  // Send write command to KBC and new command byte with enabled interrupts
  sys_outb(KBC_CMD_REG, KBC_WRITE_CMD);
  sys_outb(INPUT_BUFFER, KBC_CMD);

  return 0;
}
