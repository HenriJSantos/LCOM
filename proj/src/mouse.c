#include <lcom/lcf.h>
#include "mouse.h"
#include "i8042.h"

uint32_t packet_byte;

int mouse_hook_id = 2;
bool mouse_ih_return;

int(mouse_subscribe_int)(uint8_t *bit_no)
{
  *bit_no = mouse_hook_id;

  if (sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &mouse_hook_id) == OK)
    return 0;
  else
    return 1;
}

int(mouse_unsubscribe_int)()
{
  if (sys_irqrmpolicy(&mouse_hook_id) == OK)
    return 0;
  else
    return 1;
}

void(mouse_ih)()
{
  // Read status
  uint32_t status;
  sys_inb(STATUS_REG, &status);
  // If there's an error or the byte didn't come from the mouse, dispose of it
  if (status & (PARITY_ERR | TIMEOUT_ERR) || !(status & AUX))
  {
    uint32_t temp;
    sys_inb(OUTPUT_BUFFER, &temp);
    mouse_ih_return = 1;
  }
  // Move contents of buffer to packet_byte
  else
  {
    sys_inb(OUTPUT_BUFFER, &packet_byte);
    mouse_ih_return = 0;
  }
}

int mouse_command(uint8_t arg)
{
  uint8_t tries = 10;
  uint32_t ack_byte, status;
  while (tries > 0)
  {
    tries--;
    sys_inb(STATUS_REG, &status);
    if (status & IBF)
      continue;
    sys_outb(KBC_CMD_REG, KBC_WRITE_MOUSE);
    sys_inb(STATUS_REG, &status);
    if (status & IBF)
      continue;
    sys_outb(INPUT_BUFFER, arg);
    sys_inb(OUTPUT_BUFFER, &ack_byte);
    if (ack_byte != ACK)
      continue;
    return 0;
  }
  return 1;
}

void disable_mouse_interrupts()
{
  sys_irqdisable(&mouse_hook_id);
}

void enable_mouse_interrupts()
{
  sys_irqenable(&mouse_hook_id);
}

int reset_kbc_cmd()
{
  uint32_t status;
  uint8_t defCmd = minix_get_dflt_kbc_cmd_byte();
  sys_inb(STATUS_REG, &status);
  if (status & IBF)
    return 1;
  sys_outb(STATUS_REG, KBC_WRITE_CMD);
  sys_inb(STATUS_REG, &status);
  if (status & IBF)
    return 1;
  sys_outb(INPUT_BUFFER, defCmd);
  return 0;
}

void parse_packet(uint8_t packet[], struct packet *pp)
{
  pp->bytes[0] = packet[0];
  pp->bytes[1] = packet[1];
  pp->bytes[2] = packet[2];
  pp->lb = (packet[0] & BIT(0));
  pp->rb = (packet[0] & BIT(1)) >> 1;
  pp->mb = (packet[0] & BIT(2)) >> 2;
  pp->x_ov = (packet[0] & BIT(6)) >> 6;
  pp->y_ov = (packet[0] & BIT(7)) >> 7;
  //Check delta x sign
  if (packet[0] & BIT(4))
    pp->delta_x = (packet[1] | SE_8_TO_16);
  else
    pp->delta_x = packet[1];
  //Check delta y sign
  if (packet[0] & BIT(5))
    pp->delta_y = (packet[2] | SE_8_TO_16);
  else
    pp->delta_y = packet[2];
}

void handle_packet_byte(uint8_t *packetCounter, uint8_t packetArray[])
{
  switch (*packetCounter)
  {
  case 0:
    if (!(packet_byte & BIT(3)))
      break;
    else
    {
      packetArray[0] = packet_byte;
      (*packetCounter)++;
    }
    break;
  case 1:
    packetArray[1] = packet_byte;
    (*packetCounter)++;
    break;
  case 2:
    packetArray[2] = packet_byte;
    (*packetCounter)++;
  }
}
