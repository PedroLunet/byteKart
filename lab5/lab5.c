// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>
#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>

#include "graphics.h"
#include "../lab3/keyboard.h"
#include "i8254.h"

// Any header files included below this line should have been created by you

extern vbe_mode_info_t mode_info;
extern uint8_t scancode;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(ESC_pressed)() {

  int ipc_status;
  message msg;

  uint8_t bitno_keyboard = 0;

  if (kbc_subscribe_int(&bitno_keyboard) != 0) {
    printf("Error in kbd_subscribe_int\n");
    return 1;
  }

  while (scancode != ESC_BREAKCODE) {
      if (driver_receive(ANY, &msg, &ipc_status) != 0) {
        printf("Error in driver_receive\n");
        return 1;
      }

      if (is_ipc_notify(ipc_status)) {
        switch (_ENDPOINT_P(msg.m_source)) {
          case HARDWARE:
            if (msg.m_notify.interrupts & bitno_keyboard) {
              kbc_ih();
              break;
            }
          default:
            break;
        }
      }
  }

  if (kbc_unsubscribe_int() != 0) {
     printf("Error in kbd_unsubscribe_int\n");
     return 1;
  }

  return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay) {

  //set video mode
  if (vg_set_mode(mode) != 0) {
    printf("Error setting video mode\n");
    return 1;
  }

  // wait for delay seconds
  sleep(delay);

  // back to text mode
  if (vg_exit() != 0) {
    printf("Error exiting video mode\n");
    return 1;
  }

  return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height, uint32_t color) {

  // map the video memory
  if (vg_set_frame_buffer(mode) != 0) {
    printf("Error mapping video memory\n");
    return 1;
  }

  // set the video mode
  if (vg_set_mode(mode) != 0) {
    printf("Error setting video mode\n");
    return 1;
  }

  uint32_t new_color;
  if (normalize_color(color, &new_color) != 0) {
    printf("Error normalizing color\n");
    vg_exit();
    return 1;
  }

  // draw rectangle
  if (vg_draw_rectangle(x, y, width, height, new_color) != 0) {
     printf("Error drawing rectangle\n");
     vg_exit();
     return 1;
  }

  if (ESC_pressed() != 0) {
    printf("Error waiting for ESC key\n");
    return 1;
  }

  // back to text mode
  if (vg_exit() != 0) {
    printf("Error exiting video mode\n");
    return 1;
  }

  return 0;
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {
  /* To be completed */
  printf("%s(0x%03x, %u, 0x%08x, %d): under construction\n", __func__,
         mode, no_rectangles, first, step);

  return 1;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {
  /* To be completed */
  printf("%s(%8p, %u, %u): under construction\n", __func__, xpm, x, y);

  return 1;
}

int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf,
                     int16_t speed, uint8_t fr_rate) {
  /* To be completed */
  printf("%s(%8p, %u, %u, %u, %u, %d, %u): under construction\n",
         __func__, xpm, xi, yi, xf, yf, speed, fr_rate);

  return 1;
}

int(video_test_controller)() {
  /* To be completed */
  printf("%s(): under construction\n", __func__);

  return 1;
}
