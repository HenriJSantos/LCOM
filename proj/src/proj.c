// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>
#include "game.h"
#include "bmp.h"
#include "keyboard.h"
#include "video.h"
#include "i8042.h"
#include "mouse.h"
#include "sprites.h"
#include "RTC.h"
#include "serial_port.h"
#include "text.h"

/** @defgroup Proj Proj
 * @{
 *
 * @brief File containing main function
 */

uint8_t irq_set_kb, irq_set_timer, irq_set_mouse, irq_set_com1;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  // lcf_trace_calls("/home/lcom/labs/proj/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/proj/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return 1]
  lcf_cleanup();

  return 0;
}

int (proj_main_loop)(int argc, char * argv[]) {
  (void)argc;
  (void)argv;
  serial_port_com1_config(1);

  if(serial_port_subscribe_int(&irq_set_com1))
  {
    printf("Serial Port subscription failed.\n");
    return 1;
  }
  
  // Subscribe to timer interrupts
  if (timer_subscribe_int(&irq_set_timer))
  {
    printf("Timer subscription failed.\n");
    return 1;
  }

  // Subscribe to keyboard interruptsCLICK
  if (keyboard_subscribe_int(&irq_set_kb))
  {
    printf("Keyboard subscription failed.\n");
    return 1;
  }

  //Subscribe to mouse interrupts
  if (mouse_subscribe_int(&irq_set_mouse))
  {
    printf("Mouse subscription failed.\n");
    return 1;
  }

  disable_mouse_interrupts();

  //Enable data reporting
  if (mouse_command(KBC_EN_DATA))
    return 1;

  enable_mouse_interrupts();

  lm_init(false);
  vg_init(OUR_VIDEO_MODE);

  init_project();

  interrupt_handler();

  disable_mouse_interrupts();

  //Disable data reporting
  if (mouse_command(KBC_DIS_DATA))
    return 1;

  enable_mouse_interrupts();

  //Unsubscribe from mouse interrupts
  if (mouse_unsubscribe_int())
  {
    printf("Mouse unsubscription failed.\n");
    return 1;
  }

  // Unsubscribe to keyboard interrupts
  if (keyboard_unsubscribe_int())
  {
    printf("Keyboard unsubscription failed.\n");
    return 1;
  }
  // Unsubscribe to timer interrupts
  if (timer_unsubscribe_int())
  {
    printf("Timer unsubscription failed.\n");
    return 1;
  }

  if(serial_port_unsubscribe_int())
  {
    printf("Serial Port unsubscription failed.\n");
    return 1;
  }

  vg_exit();
  free_buffer();
  return 0;
}
