// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you
#include "UTIL.h"
#include "i8042.h"
#include "keyboard.h"
#include "sprites.h"
#include "video.h"
#include "VIDEO_MACRO.h"

extern uint16_t timer_counter;
extern uint32_t scancode;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  // lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  // lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay) {
  lm_init(false);
  vg_init(mode);
  sleep(delay);
  vg_exit();
  return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y, uint16_t width,
                          uint16_t height, uint32_t color) {
  uint8_t irq_set_kb;
  int ipc_status, r;
  message msg;

  vg_vbe_contr_info_t cont_info;
  void * base_pointer;
  while((base_pointer = lm_init(false)) == NULL){};
  parse_vbe_controller(&cont_info, base_pointer);
  if(!isValidMode(&cont_info, mode))
  {
    printf("Invalid mode.\n");
    return 1;
  }
  vg_init(mode);

  vg_draw_rectangle(x, y, width, height, color);

  // Subscribe to keyboard interrupts
  if (keyboard_subscribe_int(&irq_set_kb)) {
    printf("Keyboard subscription failed.\n");
    return 1;
  }

  // Interrupt check cycle
  while (scancode != ESC_BREAKCODE) {
    // Get a request message
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { // Received notification
      switch (_ENDPOINT_P(msg.m_source)) {
      case HARDWARE: // Hardware interrupt notification
        if (msg.m_notify.interrupts & BIT(irq_set_kb)) {
          kbc_ih();
        }
        break;
      default:
        break;
      }
    }
  }

  // Unsubscribe to keyboard interrupts
  if (keyboard_unsubscribe_int()) {
    vg_exit();
    printf("Keyboard unsubscription failed.\n");
    return 2;
  }

  vg_exit();

  return 0;
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first,
                        uint8_t step) {
  uint8_t irq_set_kb;
  int ipc_status, r;
  message msg;

  vg_vbe_contr_info_t cont_info;
  void * base_pointer;
  while((base_pointer = lm_init(false)) == NULL){};
  parse_vbe_controller(&cont_info, base_pointer);
  if(!isValidMode(&cont_info, mode))
  {
    printf("Invalid mode.\n");
    return 1;
  }
  vg_init(mode);
  vbe_mode_info_t info;
  parse_vbe_mode(mode, &info);
  uint16_t width = get_h_res() / no_rectangles;
  uint16_t height = get_v_res() / no_rectangles;

  if (get_bits_per_pixel() == 8) {
    uint32_t color;
    for (unsigned int row = 0; row < no_rectangles; row++) {
      for (unsigned int col = 0; col < no_rectangles; col++) {
        color = (first + (row * no_rectangles + col) * step) %
                (1 << get_bits_per_pixel());
        vg_draw_rectangle(col * width, row * height, width, height, color);
      }
    }
  } else {
    uint8_t R, G, B;
    uint8_t Rfirst = first >> info.RedFieldPosition % BIT(info.RedMaskSize);
    uint8_t Gfirst = first >> info.GreenFieldPosition % BIT(info.GreenMaskSize);
    uint8_t Bfirst = first >> info.BlueFieldPosition % BIT(info.BlueMaskSize);

    for (unsigned int row = 0; row < no_rectangles; row++) {
      for (unsigned int col = 0; col < no_rectangles; col++) {
        R = (Rfirst + col * step) % (1 << info.RedMaskSize);
        G = (Gfirst + row * step) % (1 << info.GreenMaskSize);
        B = (Bfirst + (col + row) * step) % (1 << info.BlueMaskSize);
        vg_draw_rectangle(col * width, row * height, width, height,
                          (R << info.RedFieldPosition) +
                              (G << info.GreenFieldPosition) +
                              (B << info.BlueFieldPosition));
      }
    }
  }

  // Subscribe to keyboard interrupts
  if (keyboard_subscribe_int(&irq_set_kb)) {
    printf("Keyboard subscription failed.\n");
    return 1;
  }

  // Interrupt check cycle
  while (scancode != ESC_BREAKCODE) {
    // Get a request message
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { // Received notification
      switch (_ENDPOINT_P(msg.m_source)) {
      case HARDWARE: // Hardware interrupt notification
        if (msg.m_notify.interrupts & BIT(irq_set_kb)) {
          kbc_ih();
        }
        break;
      default:
        break;
      }
    }
  }

  // Unsubscribe to keyboard interrupts
  if (keyboard_unsubscribe_int()) {
    vg_exit();
    printf("Keyboard unsubscription failed.\n");
    return 2;
  }

  vg_exit();

  return 0;
}

int(video_test_xpm)(const char *xpm[], uint16_t xi, uint16_t yi) {
  uint8_t irq_set_kb;
  int ipc_status, r;
  message msg;

  lm_init(false);
  vg_init(VBE_INDEXED_MODE);
  draw_xpm(xpm, xi, yi);

  // Subscribe to keyboard interrupts
  if (keyboard_subscribe_int(&irq_set_kb)) {
    printf("Keyboard subscription failed.\n");
    return 2;
  }

  // Interrupt check cycle
  while (scancode != ESC_BREAKCODE) {
    // Get a request message
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { // Received notification
      switch (_ENDPOINT_P(msg.m_source)) {
      case HARDWARE: // Hardware interrupt notification
        if (msg.m_notify.interrupts & BIT(irq_set_kb)) {
          kbc_ih();
        }
        break;
      default:
        break;
      }
    }
  }

  // Unsubscribe to keyboard interrupts
  if (keyboard_unsubscribe_int()) {
    vg_exit();
    printf("Keyboard unsubscription failed.\n");
    return 2;
  }

  vg_exit();
  return 0;
}

int(video_test_move)(const char *xpm[], uint16_t xi, uint16_t yi, uint16_t xf,
                     uint16_t yf, int16_t speed, uint8_t fr_rate) {
  uint8_t irq_set_kb, irq_set_timer;
  int ipc_status, r;
  message msg;
  uint16_t frame_counter = 0;
  bool horizontal_movement = xf - xi;

  if (xi != xf && yi != yf)
    return 1;

  // Subscribe to timer interrupts
  if (timer_subscribe_int(&irq_set_timer)) {
    printf("Timer subscription failed.\n");
    return 2;
  }

  // Subscribe to keyboard interrupts
  if (keyboard_subscribe_int(&irq_set_kb)) {
    printf("Keyboard subscription failed.\n");
    return 3;
  }

  lm_init(false);
  vg_init(VBE_INDEXED_MODE);
  Sprite *spr;
  if (horizontal_movement) {
    if (xf > xi)
      spr = create_sprite(xpm, xi, yi, speed, 0);
    else
      spr = create_sprite(xpm, xi, yi, -speed, 0);
  } else if (yf > yi)
    spr = create_sprite(xpm, xi, yi, 0, speed);
  else
    spr = create_sprite(xpm, xi, yi, 0, -speed);
  draw_sprite(spr);

  while (scancode != ESC_BREAKCODE) {
    // Get a request message
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { // Received notification
      switch (_ENDPOINT_P(msg.m_source)) {
      case HARDWARE: // Hardware interrupt notification
        if (msg.m_notify.interrupts & BIT(irq_set_kb)) {
          kbc_ih();
        }
        if (msg.m_notify.interrupts & BIT(irq_set_timer)) {
          timer_int_handler();

          if (timer_counter % (sys_hz() / fr_rate) == 0) {
            if (!(horizontal_movement && spr->x == xf) &&
                !(!horizontal_movement && spr->y == yf)) {
              if (speed > 0) {
                if (horizontal_movement) {
                  if (xf - xi > 0) {
                    if (spr->x + spr->xspeed >
                        xf) {
                      blacken_sprite(spr);
                      spr->x = xf;
                      draw_sprite(spr);
                    } else {
                      blacken_sprite(spr);
                      spr->x += spr->xspeed;
                      draw_sprite(spr);
                    }
                  } else {
                    if (spr->x + spr->xspeed <
                        xf) {
                      blacken_sprite(spr);
                      spr->x = xf;
                      draw_sprite(spr);
                    } else {
                      blacken_sprite(spr);
                      spr->x += spr->xspeed;
                      draw_sprite(spr);
                    }
                  }
                } else {
                  if (yf - yi > 0) {
                    if (spr->y + spr->yspeed >
                        yf) {
                      blacken_sprite(spr);
                      spr->y = yf;
                      draw_sprite(spr);
                    } else {
                      blacken_sprite(spr);
                      spr->y += spr->yspeed;
                      draw_sprite(spr);
                    }
                  } else {
                    if (spr->y + spr->yspeed <
                        yf) {
                      blacken_sprite(spr);
                      spr->y = yf;
                      draw_sprite(spr);
                    } else {
                      blacken_sprite(spr);
                      spr->y += spr->yspeed;
                      draw_sprite(spr);
                    }
                  }
                }
              } else if (speed < 0) {
                frame_counter++;
                if (frame_counter == -speed) {
                  frame_counter = 0;
                  if (horizontal_movement) {
                    blacken_sprite(spr);
                    spr->x += (((xf - xi) > 0) - ((xf - xi) < 0)) * 1;
                    draw_sprite(spr);
                  } else {
                    blacken_sprite(spr);
                    spr->y += (((yf - yi) > 0) - ((yf - yi) < 0)) * 1;
                    draw_sprite(spr);
                  }
                }
              }
            }
          }
        }
        break;
      default:
        break;
      }
    }
  }

  // Unsubscribe to keyboard interrupts
  if (keyboard_unsubscribe_int()) {
    vg_exit();
    printf("Keyboard unsubscription failed.\n");
    return 2;
  }
  // Unsubscribe to timer interrupts
  if (timer_unsubscribe_int()) {
    printf("Timer unsubscription failed.\n");
    return 2;
  }

  vg_exit();
  return 0;
}

int(video_test_controller)() {
  vg_vbe_contr_info_t info_p;
  void * base_pointer;
  while((base_pointer = lm_init(false)) == NULL){};
  parse_vbe_controller(&info_p, base_pointer);
  vg_display_vbe_contr_info(&info_p);
  return 0;
}
