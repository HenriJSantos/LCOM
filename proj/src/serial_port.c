#include "serial_port.h"

int serial_port_hook_id = 4;

int serial_port_subscribe_int(uint8_t *bit_no)
{
  *bit_no = serial_port_hook_id;

  if (sys_irqsetpolicy(SERIAL_PORT_IRQ_1, (IRQ_REENABLE | IRQ_EXCLUSIVE), &serial_port_hook_id) == OK)
    return 0;
  else
    return 1;
}

int serial_port_unsubscribe_int()
{
  if (sys_irqrmpolicy(&serial_port_hook_id) == OK)
    return 0;
  else
    return 1;
}

void serial_port_com1_config(int port)
{
  int base_adr;
  switch(port)
  {
    case 1:
      base_adr = BASE_COM1;
      break;
    case 2:
      base_adr = BASE_COM2;
      break;
    default:
      return;
  }

  //Set Serial Port
  sys_outb(base_adr + LCR, (BITS_PER_CHAR_8 | STOP_BITS_1 | PARITY_ODD | DLAB));

  //Set bit rate at 19200
  sys_outb(base_adr + DLL, 0x06);
	sys_outb(base_adr + DLM, 0x00);

  sys_outb(base_adr + LCR, (BITS_PER_CHAR_8 | STOP_BITS_1 | PARITY_ODD));

  sys_outb(base_adr + IIR, (FIFO_INTERRUPT_TRIGGER_LEVEL_1 | CLEAR_TRANSMIT_FIFO | CLEAR_RECEIVE_FIFO | ENABLE_FIFO));

  sys_outb(base_adr + IER, (ENABLE_RECEIVER_LINE_STATUS_INTERRUPT | ENABLE_RECEIVED_DATA_INTERRUPT));
}

void serial_port_ih(char ** bytes, unsigned int * size)
{
	uint32_t int_id = 0;

  sys_inb(BASE_COM1 + IIR, &int_id);

	if(INTERRUPT_ORIGIN_RECEIVED_DATA & int_id) // receiver ready
	{
    uint32_t received;
		sys_inb(BASE_COM1, &received);
    (*size) = (*size) + 1;
		*bytes = (char *) realloc (*bytes, sizeof(char)*(*size));
    (*bytes)[(*size)-1] = received;
	} 
}

void clear_sp_buffer()
{
  uint32_t status;
  sys_inb(BASE_COM1 + LSR, &status);
  while(status & RECEIVER_READY)
  {
    uint32_t temp;
    sys_inb(BASE_COM1, &temp);
    sys_inb(BASE_COM1 + LSR, &status);
  }
}

bool got_full_message(char ** bytes, unsigned int * size)
{
  if(*size < 1)
    return false;
  if(*size > 6)
  {
    *size = 0;
    free(*bytes);
    *bytes = NULL;
    return false;
  }
  switch((*bytes)[0])
  {
  case 'T':
    if(*size == 3)
      return true;
    else
      return false;
    break;
  case 'K':
    if(*size == 2)
      return true;
    else
      return false;
    break;
  case 'M':
    if(*size == 6)
      return true;
    else
      return false;
    break;
  case 'P':
    return true;
    break;
  default:
    *size = 0;
    free(*bytes);
    *bytes = NULL;
    return false;
  }
}

void send_sp_byte(uint8_t data)
{
  uint32_t lsr;
  do
  {
    sys_inb(BASE_COM1 + LSR, &lsr);
  } while (!(lsr & TRANSMITTER_EMPTY));

	sys_outb(BASE_COM1, data);
}

void send_sp_message(char * message, uint8_t size)
{
  for (unsigned int i = 0; i < size; i++)
    send_sp_byte(message[i]);
}
