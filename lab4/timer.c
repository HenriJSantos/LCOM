#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>
#include "i8254.h"

int timer_hook_id = 0;
uint16_t timer_counter = 0;

int (timer_set_frequency)(uint8_t timer, uint32_t freq)  {
  //Ruling out invalid frequency values
  if (freq < 19 || freq > TIMER_FREQ)
  {
    printf("Invalid frequency value.\n");
    return 1;
  }

  //Getting current timer configuration
  uint8_t curr_st;
  timer_get_conf(timer, &curr_st);  //Get current status so the mode and bcd won't be changed

  //Creating respective control word
  uint8_t curr_mode = curr_st & TIMER_CNT_MODE_MASK;
  uint8_t curr_base = curr_st & TIMER_BCD;
  uint8_t cw_cmd_byte = TIMER_LSB_MSB | curr_mode | curr_base | TIMER_SEL(timer);

  //Setting up correct divisor for frequency
  uint16_t div;
  div = TIMER_FREQ / freq;

  //Getting divisor LSB and MSB from util functions
  uint8_t div_lsb, div_msb;
  if (util_get_LSB(div, &div_lsb) || util_get_MSB(div, &div_msb))
  {
    printf("Error creating divisor LSB and MSB.\n");
    return 2;
  }

  //Sending control word to timer control
  if (sys_outb(TIMER_CTRL, cw_cmd_byte))
  {
    printf("Error in sending cw_cmd_byte to timer control.\n");
    return 3;
  }

  //Sending div LSB and MSB to timer
  if (sys_outb(TIMER_ADDRESS(timer), div_lsb) || sys_outb(TIMER_ADDRESS(timer), div_msb))
  {
    printf("Error sending LSB and MSB to timer.\n");
    return 4;
  }

  return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no) {

  //Sets "irq_set" in timer_test_int function to a mask of the correct interrupt bit in notification
  *bit_no = timer_hook_id;

  if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &timer_hook_id) == OK)
    return 0;
  else
    return 1;
}

int (timer_unsubscribe_int)() {
  if(sys_irqrmpolicy(&timer_hook_id) == OK)
    return 0;
  else
    return 1;
}

void (timer_int_handler)() {
  timer_counter++;
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {
  uint8_t rb_cmd_byte = TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer);
  uint32_t st_temp; //Temporary 32-bit variable to pass as sys_inb argument
    
  if (sys_outb(TIMER_CTRL, rb_cmd_byte))
  {
    printf("Error sending read-back byte to timer control.\n");
    return 1;
  }

  if (sys_inb(TIMER_ADDRESS(timer), &st_temp))
  {
    printf("Error getting status from timer.\n");
    return 2;
  }

  *st = (uint8_t) st_temp; //Casting 32-bit variable into the original 8-bit st variable
  return 0;
}

int (timer_display_conf)(uint8_t timer, uint8_t st,
                        enum timer_status_field field) {

  union timer_status_field_val val;

  //Each case sets a different type of the union depending on what the field specifies to be printed
  switch (field)
  {
    case all:
      // 7 6 5 4 3 2 1 0
      val.byte = st;
      break;

    case initial:
      // x x 5 4 x x x x
      {
      uint8_t init_config = (st & TIMER_LSB_MSB) >> 4;
      val.in_mode = init_config;
      break;
      }

    case mode:
      // x x x x 3 2 1 x
      val.count_mode = (st & TIMER_CNT_MODE_MASK) >> 1;
      if (val.count_mode >= 6)
        val.count_mode ^= BIT(2);
      break;

    case base:
      // x x x x x x x 0
      val.bcd = st & TIMER_BCD;
      break;

    default:
      printf("Invalid field specification.\n");
      return 1;
  }

  //Once val contains what we want to print, pass it on to timer_print_config
  if(timer_print_config(timer, field, val))
  {
    printf("Error printing specified configuration.\n");
    return 2;
  }

  return 0;
}
