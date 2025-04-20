#include <lcom/lcf.h>

#include "graphics.h"
#include "../lab3/keyboard.h"
#include "../lab2/timer.c"

extern vbe_mode_info_t mode_info;
extern uint8_t scancode;
extern int timer_counter;

int main(int argc, char *argv[]) {

  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/Labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/Labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int (ESC_key_wait)() {

  int ipc_status;
  message msg;
  uint8_t irq_set_keyboard;

  if (keyboard_subscribe_int(&irq_set_keyboard) != 0) {
    return 1;
  }

  while (scancode != ESC_BREAKCODE) {
    if (driver_receive(ANY, &msg, &ipc_status) != 0) {
      printf("error");
      continue;
    }
    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set_keyboard)
            kbc_ih();
            keyboard_process_scancode();
            break;
        default:
          break;
      }
    }
  }

  if (keyboard_unsubscribe_int() != 0) {
    return 1;
  }
  return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay) {

  if (vg_set_mode(mode) != 0) {
    return 1;
  }

  sleep(delay);

  if (vg_exit() != 0) {
    return 1;
  }

  return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height, uint32_t color) {

  if (vg_set_frame_buffer(mode) != 0) return 1;

  if (vg_set_mode(mode) != 0) return 1;

  uint32_t new_color;
  if (normalize_color(color, &new_color) != 0) {
    return 1;
  }

  if (vg_draw_rectangle(x, y, width, height, new_color) != 0) {
    return 1;
  }

  swap_buffers();

  if (ESC_key_wait() != 0) {
    return 1;
  }

  if (vg_exit() != 0) {
    return 1;
  }

  return 0;
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {

  if (vg_set_frame_buffer(mode) != 0) {
    return 1;
  }

  if (vg_set_mode(mode) != 0) {
    return 1;
  }

  int v = mode_info.YResolution / no_rectangles;
  int h = mode_info.XResolution / no_rectangles;

  for (int i = 0 ; i < no_rectangles ; i++) {
    for (int j = 0 ; j < no_rectangles ; j++) {

      uint32_t color = calculate_color(j, i, step, first, no_rectangles, mode_info.MemoryModel);

      if (vg_draw_rectangle(j * h, i * v, h, v, color) != 0) {
        return 1;
      }
    }
  }

  swap_buffers();

  if (ESC_key_wait() != 0) {
    return 1;
  }

  if (vg_exit() != 0) {
    return 1;
  }

  return 0;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {

  if (vg_set_frame_buffer(INDEXED_COLOR) != 0) {
    return 1;
  }

  if (vg_set_mode(INDEXED_COLOR) != 0) {
    return 1;
  }

  if (print_xpm(xpm, x, y) != 0) {
    return 1;
  }

  if (ESC_key_wait() != 0) {
    return 1;
  }

  if (vg_exit() != 0) {
    return 1;
  }

  return 0;
}

int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf,
                     int16_t speed, uint8_t fr_rate) {

  uint8_t vertical_direction;
  if (xi == xf && yi < yf) vertical_direction = 1;
  else if (yi == yf && xi < xf) vertical_direction = 0;
  else return 1;

  int ipc_status;
  message msg;
  uint8_t irq_set_keyboard, irq_set_timer;

  if (keyboard_subscribe_int(&irq_set_keyboard) != 0) {
    return 1;
  }

  if (timer_subscribe_int(&irq_set_timer) != 0) {
    return 1;
  }

  if (vg_set_frame_buffer(INDEXED_COLOR) != 0) {
   return 1;
  }

  if (vg_set_mode(INDEXED_COLOR) != 0) {
   return 1;
  }

  if (print_xpm(xpm, xi, yi) != 0) {
   return 1;
  }

  while (scancode != ESC_BREAKCODE && (xi < xf || yi < yf)) {
    if (driver_receive(ANY, &msg, &ipc_status) != 0) {
      printf("error");
      continue;
    }
    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:

          if (msg.m_notify.interrupts & irq_set_keyboard) {
            kbc_ih();
            keyboard_process_scancode();
          }

          if (msg.m_notify.interrupts & irq_set_timer) {

          	if (vg_draw_rectangle(0, 0, mode_info.XResolution, mode_info.YResolution, 0x000000) != 0) {
              return 1;
            }

            if (vertical_direction) {
                yi += speed;
                if (yi > yf) yi = yf;
            } else {
                xi += speed;
                if (xi > xf) xi = xf;
            }

            if (print_xpm(xpm, xi, yi) != 0) {
              return 1;
            }

            swap_buffers();
          }
      }
    }
  }

  if (vg_exit() != 0) {
    return 1;
  }

  if (keyboard_unsubscribe_int() != 0) {
    return 1;
  }

  if (timer_unsubscribe_int() != 0) {
    return 1;
  }

  return 0;

}
