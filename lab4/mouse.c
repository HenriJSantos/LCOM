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

struct mouse_ev detect_mouse_evt(struct packet *pp, uint8_t *event)
{
  //Previous packet received for comparing with current packet
  static struct packet prevPacket;
  struct mouse_ev mouse_event;
  enum mouse_ev_t eventType;

  //Press LB
  if (prevPacket.lb == 0 && pp->lb == 1 && pp->rb == 0 && pp->mb == 0)
  {
    eventType = LB_PRESSED;
    mouse_event.type = eventType;
    mouse_event.delta_x = 0;
    mouse_event.delta_y = 0;
    *event |= MOUSE_EVT;
  }
  //Release LB
  else if (prevPacket.lb == 1 && pp->lb == 0 && pp->rb == 0 && pp->mb == 0)
  {
    eventType = LB_RELEASED;
    mouse_event.type = eventType;
    mouse_event.delta_x = 0;
    mouse_event.delta_y = 0;
    *event |= MOUSE_EVT;
  }
  //Press RB
  else if (prevPacket.rb == 0 && pp->rb == 1 && pp->lb == 0 && pp->mb == 0)
  {
    eventType = RB_PRESSED;
    mouse_event.type = eventType;
    mouse_event.delta_x = 0;
    mouse_event.delta_y = 0;
    *event |= MOUSE_EVT;
  }
  //Release RB
  else if (prevPacket.rb == 1 && pp->rb == 0 && pp->lb == 0 && pp->mb == 0)
  {
    eventType = RB_RELEASED;
    mouse_event.type = eventType;
    mouse_event.delta_x = 0;
    mouse_event.delta_y = 0;
    *event |= MOUSE_EVT;
  }
  //Move mouse
  else if (pp->delta_x != 0 || pp->delta_y != 0)
  {
    eventType = MOUSE_MOV;
    mouse_event.type = eventType;
    mouse_event.delta_x = pp->delta_x;
    mouse_event.delta_y = pp->delta_y;
    *event |= MOUSE_EVT;
  }
  //Unused mouse events
  else
  {
    eventType = BUTTON_EV;
    mouse_event.type = eventType;
    mouse_event.delta_x = 0;
    mouse_event.delta_y = 0;
  }

  //Set previous packet as current packet for next iteration
  prevPacket = *pp;
  return mouse_event;
}

void handle_mouse_evt(struct mouse_ev * mouse_event, state_t * gesture_state, uint8_t x_len, uint8_t tolerance)
{
  static int16_t dx = 0, dy = 0;
  switch (*gesture_state)
  {
  //If at the beginning, there's only one option to change state: press LB
  case INIT:
    if (mouse_event->type == LB_PRESSED)
    {
      *gesture_state = UP_RIGHT;
    }
    break;
  // While drawing up-right portion, add up all x and y movements and once LB is released check for dy>dx
  case UP_RIGHT:
    if (mouse_event->type == MOUSE_MOV)
    {
      dx += mouse_event->delta_x;
      dy += mouse_event->delta_y;
    }
    else if (mouse_event->type == LB_RELEASED)
    {
      if (dy > dx && dx >= x_len)
      {
        dx = 0;
        dy = 0;
        *gesture_state = VERTICE;
      }
      else
      {
        dx = 0;
        dy = 0;
        *gesture_state = INIT;
      }
    }
    else
    {
      dx = 0;
      dy = 0;
      *gesture_state = INIT;
    }
    break;
  // While at the vertice, ensure user doesn't move mouse beyond tolerance and move to next state if RB is pressed
  case VERTICE:
    if (mouse_event->type == RB_PRESSED)
    {
      dx = 0;
      dy = 0;
      *gesture_state = DOWN_RIGHT;
    }
    else if (mouse_event->type == LB_PRESSED)
    {
      dx = 0;
      dy = 0;
      *gesture_state = UP_RIGHT;
    }
    else if (mouse_event->type == MOUSE_MOV)
    {
      dx += mouse_event->delta_x;
      dy += mouse_event->delta_y;
      if (abs(dx) > tolerance || abs(dy) > tolerance)
      {
        dx = 0;
        dy = 0;
        *gesture_state = INIT;
      }
    }
    else
    {
      dx = 0;
      dy = 0;
      *gesture_state = INIT;
    }
    break;
  // While drawing down-right portion, add up all x and y movements and once LB is released check for -dy>dx
  case DOWN_RIGHT:
    if (mouse_event->type == MOUSE_MOV)
    {
      dx += mouse_event->delta_x;
      dy += mouse_event->delta_y;
    }
    else if (mouse_event->type == RB_RELEASED)
    {
      if (-dy > dx && dx >= x_len)
      {
        dx = 0;
        dy = 0;
        *gesture_state = COMP;
      }
      else
      {
        dx = 0;
        dy = 0;
        *gesture_state = INIT;
      }
    }
    else
    {
      dx = 0;
      dy = 0;
      *gesture_state = INIT;
    }
    break;
  default:
    *gesture_state = INIT;
  }
}
