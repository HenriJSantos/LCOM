// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you
#include "mouse.h"
#include "i8042.h"
#include "i8254.h"

extern uint32_t packet_byte;
extern uint16_t timer_counter;
extern bool mouse_ih_return;

int main(int argc, char *argv[])
{
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(mouse_test_packet)(uint32_t cnt)
{
  int ipc_status, r;
  uint8_t irq_set_mouse;
  message msg;
  uint8_t packetCounter = 0;
  uint8_t packetArray[3];

  //Subscribe to mouse interrupts
  if (mouse_subscribe_int(&irq_set_mouse))
  {
    printf("Mouse subscription failed.\n");
    return 1;
  }

  disable_mouse_interrupts();

  //Enable mouse data reporting
  if (mouse_command(KBC_EN_DATA))
    return 2;

  enable_mouse_interrupts();

  //Interrupt check cycle
  while (cnt > 0)
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
        if (msg.m_notify.interrupts & BIT(irq_set_mouse))
        {
          //Read packet byte
          mouse_ih();
          if (mouse_ih_return != 0)
            continue;
          //Sort packet byte into array
          handle_packet_byte(&packetCounter, packetArray);
          //Once packet is ready, print it
          if (packetCounter == 3)
          {
            packetCounter = 0;
            struct packet pp;
            parse_packet(packetArray, &pp);
            mouse_print_packet(&pp);
            cnt--;
          }
        }
        break;
      default:
        break;
      }
    }
  }

  disable_mouse_interrupts();

  //Disable data reporting
  if(mouse_command(KBC_DIS_DATA))
    return 3;

  enable_mouse_interrupts();

  //Unsubscribe from mouse interrupts
  if (mouse_unsubscribe_int())
  {
    printf("Mouse unsubscription failed.\n");
    return 4;
  }

  return 0;
}

int(mouse_test_remote)(uint16_t period, uint8_t cnt)
{
  uint8_t packetArray[3];
  uint8_t packetCounter = 0;

  while (cnt > 0)
  {
    //Once every packet, ask mouse for new packet 
    if (packetCounter == 0)
      mouse_command(KBC_READ_MOUSE);
    //Read packet byte
    mouse_ih();
    if (mouse_ih_return != 0)
      continue;
    //Sort packet byte into array
    handle_packet_byte(&packetCounter, packetArray);
    //Once packet is complete, print it
    if (packetCounter == 3)
    {
      packetCounter = 0;
      struct packet pp;
      parse_packet(packetArray, &pp);
      mouse_print_packet(&pp);
      cnt--;
      tickdelay(micros_to_ticks(period * 1000));
    }
  }

  //Reset mouse to default configuration
  if (mouse_command(KBC_SET_STREAM))
    return 1;
  if (mouse_command(KBC_DIS_DATA))
    return 2;
  if (reset_kbc_cmd())
    return 3;

  return 0;
}

int(mouse_test_async)(uint8_t idle_time)
{
  int ipc_status, r;
  uint8_t irq_set_mouse, irq_set_timer;
  message msg;
  uint8_t packetCounter = 0;
  uint8_t packetArray[3];

  //Subscribe to mouse interrupts
  if (mouse_subscribe_int(&irq_set_mouse))
  {
    printf("Mouse subscription failed.\n");
    return 1;
  }

  //Subscribe to timer interrupts
  if (timer_subscribe_int(&irq_set_timer))
  {
    printf("Timer subscription failed.\n");
    return 2;
  }

  disable_mouse_interrupts();

  //Set stream mode and enable data reporting
  if (mouse_command(KBC_SET_STREAM))
    return 3;
  if (mouse_command(KBC_EN_DATA))
    return 4;

  enable_mouse_interrupts();

  //Check for interrupts while there are interrupts with less than idle_time seconds between them
  while (timer_counter <= idle_time * (uint8_t)sys_hz())
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
        if (msg.m_notify.interrupts & BIT(irq_set_mouse))
        {
          //Reset timer
          timer_counter = 0;
          //Read packet byte
          mouse_ih();
          if (mouse_ih_return != 0)
            continue;
          //Sort packet byte into array
          handle_packet_byte(&packetCounter, packetArray);
          //Once packet is ready, print it
          if (packetCounter == 3)
          {
            packetCounter = 0;
            struct packet pp;
            parse_packet(packetArray, &pp);
            mouse_print_packet(&pp);
          }
        }
        if (msg.m_notify.interrupts & BIT(irq_set_timer))
        {
          //Increment timer value
          timer_int_handler();
        }
        break;
      default:
        break;
      }
    }
  }

  disable_mouse_interrupts();

  //Disable data reporting
  if (mouse_command(KBC_DIS_DATA))
    return 5;

  enable_mouse_interrupts();

  //Unsubscribe from mouse interrupts
  if (mouse_unsubscribe_int())
  {
    printf("Mouse unsubscription failed.\n");
    return 6;
  }

  //Unsubscribe from timer interrupts
  if (timer_unsubscribe_int())
  {
    printf("Timer unsubscription failed.\n");
    return 7;
  }

  return 0;
}

int(mouse_test_gesture)(uint8_t x_len, uint8_t tolerance)
{
  int ipc_status, r;
  uint8_t irq_set_mouse;
  message msg;
  uint8_t packetCounter = 0;
  uint8_t packetArray[3];
  struct packet pp;
  state_t gesture_state = INIT;
  struct mouse_ev mouse_event;
  uint8_t event;

  //Subscribe to mouse interrupts
  if (mouse_subscribe_int(&irq_set_mouse))
  {
    printf("Mouse subscription failed.\n");
    return 1;
  }

  disable_mouse_interrupts();

  //Enable data reporting
  if (mouse_command(KBC_EN_DATA))
    return 2;

  enable_mouse_interrupts();

  //Interrupt check cycle
  while (gesture_state != COMP)
  {
    event = 0; //Clear event byte
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
        if (msg.m_notify.interrupts & BIT(irq_set_mouse))
        {
          //Read packet byte
          mouse_ih();
          if (mouse_ih_return != 0)
            continue;
          //Sort packet byte into array
          handle_packet_byte(&packetCounter, packetArray);
          //Once packet is ready, print it and check for events
          if (packetCounter == 3)
          {
            packetCounter = 0;
            parse_packet(packetArray, &pp);
            mouse_event = detect_mouse_evt(&pp, &event); //Sets MOUSE_EVT bit in event byte to 1 if there's a relevant event
            mouse_print_packet(&pp);
          }
        }
        break;
      default:
        break;
      }
    }
    //If an event is flagged, handle it according to current status
    if (event & MOUSE_EVT)
    {
      handle_mouse_evt(&mouse_event, &gesture_state, x_len, tolerance);
    }
  }

  printf("Inverted V detected.\n");

  disable_mouse_interrupts();

  //Disable data reporting
  if (mouse_command(KBC_DIS_DATA))
    return 3;

  enable_mouse_interrupts();

  //Unsubscribe from mouse interrupts
  if (mouse_unsubscribe_int())
  {
    printf("Mouse unsubscription failed.\n");
    return 4;
  }

  return 0;
}
