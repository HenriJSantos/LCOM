// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>
// Any header files below this line should have been created by you
#include "keyboard.h"
#include "i8254.h"
#include "i8042.h"

extern uint32_t sys_inb_count;
extern uint32_t scancode;

int main(int argc, char *argv[])
{
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out ifOUT_BUF you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(kbd_test_scan)(bool assembly)
{
  int ipc_status, r;
  uint8_t irq_set;
  message msg;
  uint8_t size = 1;
  uint8_t scancodeArray[2];

  //Subscribing to keyboard
  if (keyboard_subscribe_int(&irq_set))
  {
    printf("Keyboard subscription failed.\n");
    return 1;
  }

  //Interrupt check cycle
  while ((uint8_t) scancode != ESC_BREAKCODE)
  {
    //Get a request message
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0)
    {
      printf("driver_receive failed with: %d\n", r);
      continue;
    }
    if (is_ipc_notify(ipc_status))
    { //Received notification
      switch (_ENDPOINT_P(msg.m_source))
      {
      case HARDWARE: //Hardware interrupt notification
        if (msg.m_notify.interrupts & BIT(irq_set))
        {
          // Uses respective interrupt handler according to assembly bool
          if (!assembly)
            kbc_ih();
          else
            kbc_asm_ih();

          // Checks if we've got full scancode and, if so, print it
          if (gotFullScancode(scancodeArray, &size))
          {
            kbd_print_scancode(!(scancode & BIT(7)), size, scancodeArray);
            size = 1;
          }
        }
        break;
      default:
        break;
      }
    }
  }

  // Unsubscribe to keyboard interrupts
  if (keyboard_unsubscribe_int())
  {
    printf("Keyboard unsubscription failed.\n");
    return 1;
  }

  // If using C interrupt handler, display how many sys_inb calls were made
  if (!assembly)
    kbd_print_no_sysinb(sys_inb_count);

  return 0;
}

int(kbd_test_poll)()
{

  uint8_t size = 1;
  uint8_t scancodeArray[2];

  while (scancode != ESC_BREAKCODE)
  {
    // readScancode checks OBF bit in status and if it is set, calls handler
    readScancode();
    // Checks if we've got full scancode and, if so, print it
    if (gotFullScancode(scancodeArray, &size))
    {
      kbd_print_scancode(!(scancode & BIT(7)), size, scancodeArray);
      size = 1;
    }
  }

  // Prints ammount of sys_inb calls made
  kbd_print_no_sysinb(sys_inb_count);

  // Reenable default keyboard interrupts once we're done
  if(enableKeyboardInterrupts())
  {
    printf("Keyboard interrupt reenabling failed.\n");
    return 1;
  }

  return 0;
}

int(kbd_test_timed_scan)(uint8_t n)
{
  if (n<=0)
    return 1;

  int ipc_status, r;
  uint8_t irq_set_keyboard, irq_set_timer;
  message msg;
  uint8_t size = 1;
  uint8_t scancodeArray[2];

  //Subscribing to timer 0
  if (timer_subscribe_int(&irq_set_timer))
  {
    printf("Timer subscription failed.\n");
    return 2;
  }

  //Subscribing to keyboard
  if (keyboard_subscribe_int(&irq_set_keyboard))
  {
    printf("Keyboard subscription failed.\n");
    return 3;
  }

  //Interrupt check cycle
  while (scancode != ESC_BREAKCODE && timer_counter < n * (uint8_t) sys_hz())
  {
    //Get a request message
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0)
    {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status))
    { //Received notification
      switch (_ENDPOINT_P(msg.m_source))
      {
      case HARDWARE: //Hardware interrupt notification
        if (msg.m_notify.interrupts & BIT(irq_set_timer))
        {
          // Increment timer, and if max idle ammount is reached print a timeout message
          timer_int_handler();
          if (timer_counter == n * (uint8_t) sys_hz())
            printf("Timed out.\n");
        }
        if (msg.m_notify.interrupts & BIT(irq_set_keyboard))
        {
          // If an interrupt by the keyboard was detected, reset idle counter
          timer_counter = 0;
          kbc_ih();

          // Checks if we've got full scancode and, if so, print it
          if (gotFullScancode(scancodeArray, &size))
          {
            kbd_print_scancode(!(scancode & BIT(7)), size, scancodeArray);
            size = 1;
          }
        }
        break;
      default:
        break;
      }
    }
  }

  // Unsubscribe to timer interrupts
  if (timer_unsubscribe_int())
  {
    printf("Timer unsubscription failed.\n");
    return 4;
  }

  // Unsubscribe to keyboard interrupts
  if (keyboard_unsubscribe_int())
  {
    printf("Keyboard unsubscription failed.\n");
    return 5;
  }

  return 0;
}
