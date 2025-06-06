// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>

#include "video_card.h"
#include "video_card_macros.h"
#include "kbc.h"

// Any header files included below this line should have been created by you

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  //lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  //lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

extern uint8_t scancode;
extern void* video_memory;
extern unsigned int vram_size;
extern uint32_t counter;

int(video_test_init)(uint16_t mode, uint8_t delay) {

  // mudar para o modo gráfico
  int ret = change_VBE_mode(mode);
  if (ret != 0) {
    printf("Error changing to specified mode.\n");
    return 1;
  }

  sleep(delay);

  // voltar para o modo de texto
  ret = vg_exit();
  if (ret != 0) {
    printf("Error returning to text mode.\n");
    return 1;
  }
  return 1;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {

  uint8_t kbc_mask;
  int ret = kbc_subscribe_int(&kbc_mask);
  if (ret != 0) {
    printf("Error subscribing keyboard.\n");
    return 1;
  }
  int irq_set = BIT(kbc_mask);

  ret = start_VBE_mode(mode);
  if (ret != 0) {
    printf("Error changing mode.\n");
    return 1;
  }

  printf("Screen resolution: %ux%u\n", get_hres(), get_vres());
  if (x + width > get_hres() || y + height > get_vres()) {
    printf("Rectangle dimensions exceed screen boundaries.\n");
    return 1;
  }

  ret = vg_draw_rectangle(x, y, width, height, color);
  if (ret != 0) {
    printf("Error drawing rectangle in video_test_rectangle().\n");
    return 1;
  }

  int r, ipc_status;
  message msg;

  while ( scancode != ESC_BREAKCODE ) { /* You may want to use a different condition */
    /* Get a request message. */
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
      printf("driver_receive failed with: %d", r);
        continue;
    }
      if (is_ipc_notify(ipc_status)) { /* received notification */
        switch (_ENDPOINT_P(msg.m_source)) {
          case HARDWARE: /* hardware interrupt notification */				
            if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
              kbc_ih();
            }
            break;
          default:
            break; /* no other notifications expected: do nothing */	
        }
      } else { /* received a standard message, not a notification */
          /* no standard messages expected: do nothing */
      }
  }

  if (video_memory != NULL) {
    memset(video_memory, 0, vram_size);
  }
  
  if (vg_exit() != 0) {
    printf("Error returning to text mode.\n");
    return 1;
  }

  ret = kbc_unsubscribe_int();
  if (ret != 0) {
    printf("Error unsubscribing keyboard.\n");
    return 1;
  }
                            
  return 0;
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {

  uint8_t kbc_mask;
  int ret = kbc_subscribe_int(&kbc_mask);
  if (ret != 0) {
    printf("Error subscribing keyboard.\n");
    return 1;
  }
  int irq_set = BIT(kbc_mask);

  ret = start_VBE_mode(mode);
  if (ret != 0) {
    printf("Error changing mode.\n");
    return 1;
  }

  printf("Screen resolution: %ux%u\n", get_hres(), get_vres());

  if (vg_draw_matrix(no_rectangles, first, step) != 0) {
    printf("Error drawing matrix.\n");
    return 1;
  }

  int r, ipc_status;
  message msg;

  while ( scancode != ESC_BREAKCODE ) { /* You may want to use a different condition */
    /* Get a request message. */
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
      printf("driver_receive failed with: %d", r);
        continue;
    }
      if (is_ipc_notify(ipc_status)) { /* received notification */
        switch (_ENDPOINT_P(msg.m_source)) {
          case HARDWARE: /* hardware interrupt notification */				
            if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
              kbc_ih();
            }
            break;
          default:
            break; /* no other notifications expected: do nothing */	
        }
      } else { /* received a standard message, not a notification */
          /* no standard messages expected: do nothing */
      }
  }

  if (video_memory != NULL) {
    memset(video_memory, 0, vram_size);
  }
  
  if (vg_exit() != 0) {
    printf("Error returning to text mode.\n");
    return 1;
  }

  ret = kbc_unsubscribe_int();
  if (ret != 0) {
    printf("Error unsubscribing keyboard.\n");
    return 1;
  }

  return 0;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {

  uint8_t kbc_mask;
  int ret = kbc_subscribe_int(&kbc_mask);
  if (ret != 0) {
    printf("Error subscribing keyboard.\n");
    return 1;
  }
  int irq_set = BIT(kbc_mask);

  // Mode has to be 0x105
  if (start_VBE_mode(0x105) != 0) {
    printf("Error starting in VBE mode 0x105.\n");
    return 1;
  }

  // map -> pixmap in XPM format; image -> image pixmap
  xpm_image_t image;
  uint8_t* map = xpm_load(xpm, XPM_INDEXED, &image);
  if (map == NULL) {
    printf("Null map.\n");
    return 1;
  }

  ret = vg_draw_xpm(map, &image, x, y);
  if (ret != 0) {
    printf("Error drawing XPM.\n");
    return 1;
  }
  
  int r, ipc_status;
  message msg;

  while ( scancode != ESC_BREAKCODE ) { /* You may want to use a different condition */
    /* Get a request message. */
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
      printf("driver_receive failed with: %d", r);
        continue;
    }
      if (is_ipc_notify(ipc_status)) { /* received notification */
        switch (_ENDPOINT_P(msg.m_source)) {
          case HARDWARE: /* hardware interrupt notification */				
            if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
              kbc_ih();
            }
            break;
          default:
            break; /* no other notifications expected: do nothing */	
        }
      } else { /* received a standard message, not a notification */
          /* no standard messages expected: do nothing */
      }
  }

  if (video_memory != NULL) {
    memset(video_memory, 0, vram_size);
  }
  
  if (vg_exit() != 0) {
    printf("Error returning to text mode.\n");
    return 1;
  }

  ret = kbc_unsubscribe_int();
  if (ret != 0) {
    printf("Error unsubscribing keyboard.\n");
    return 1;
  }

  return 0;
}

int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf, int16_t speed, uint8_t fr_rate) {

  // Interrupts for kbd
  uint8_t kbc_mask; 
  int ret = kbc_subscribe_int(&kbc_mask);
  if (ret != 0) {
    printf("Error subscribing keyboard.\n");
    return 1;
  }
  int kbd_irq_set = BIT(kbc_mask);

  // Interrupts for timer
  uint8_t timer_mask;
  ret = timer_subscribe_int(&timer_mask);
  if (ret != 0) {
    printf("Error subscribing timer.\n");
    return 1;
  }

  ret = timer_set_frequency(0, fr_rate); // Set timer frequency
  if (ret != 0) {
    printf("Error setting time frequency.\n");
    return 1;
  }
  int timer_irq_set = BIT(timer_mask);

  // Mode has to be 0x105
  if (start_VBE_mode(0x105) != 0) {
    printf("Error starting in VBE mode 0x105.\n");
    return 1;
  }

  // Load XPM
  xpm_image_t image;
	uint8_t* map = xpm_load(xpm, XPM_INDEXED, &image);
  if (map == NULL) {
    printf("Failed to load XPM image.\n");
    return 1;
  }

  uint16_t x = xi, y = yi; // Starting position
  // Calculate how many pixels to move each frame
  int16_t dx = (xf > xi) ? speed : (xf < xi ? -speed : 0);
  int16_t dy = (yf > yi) ? speed : (yf < yi ? -speed : 0);

  if (speed < 0) { // Negative speed -> slower movement
    dx = (xf > xi) ? 1 : (xf < xi ? -1 : 0);
    dy = (yf > yi) ? 1 : (yf < yi ? -1 : 0);
  }

  // Draw initial position
  if (vg_draw_xpm(map, &image, x, y) != 0) {
    printf("Error drawing XPM in initial position.\n");
    return 1;
  }
  uint32_t frame_counter = 0;

  int r, ipc_status;
  message msg;

  while ( scancode != ESC_BREAKCODE) { /* You may want to use a different condition */
    /* Get a request message. */
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
      printf("driver_receive failed with: %d", r);
        continue;
    }
      if (is_ipc_notify(ipc_status)) { /* received notification */
        switch (_ENDPOINT_P(msg.m_source)) {
          case HARDWARE: /* hardware interrupt notification */				
            if (msg.m_notify.interrupts & kbd_irq_set) { /* subscribed interrupt */
              kbc_ih();
            }

            if (msg.m_notify.interrupts & timer_irq_set) {
              timer_int_handler(); 

              if (speed < 0 && (frame_counter % (-speed)) != 0) 
                break;

              for (uint16_t yi = 0; yi < image.height; yi++) {
                for (uint16_t xi = 0; xi < image.width; xi++) {
                  uint8_t color = map[yi * image.width + xi];
                  // Clears just the image, not the background (draws black pixel)
                  if (color != xpm_transparency_color(image.type)) {
                    vg_draw_pixel(x + xi, y + yi, 0x00); 
                  }
                }
              }

              // New position
              if (x != xf)
                x += dx;
              if (y != yf)
                y += dy;

              if ((dx > 0 && x > xf) || (dx < 0 && x < xf))
                x = xf;
              if ((dy > 0 && y > yf) || (dy < 0 && y < yf)) 
                y = yf;

              vg_draw_xpm(map, &image, x, y); // Draw in new position

              frame_counter++;
            }
            break;
          default:
            break; /* no other notifications expected: do nothing */	
        }
      } else { /* received a standard message, not a notification */
          /* no standard messages expected: do nothing */
      }
  }

  if (video_memory != NULL) {
    memset(video_memory, 0, vram_size);
  }
  
  if (vg_exit() != 0) {
    printf("Error returning to text mode.\n");
    return 1;
  }

  ret = kbc_unsubscribe_int();
  if (ret != 0) {
    printf("Error unsubscribing keyboard.\n");
    return 1;
  }

  ret = timer_unsubscribe_int();
  if (ret != 0) {
    printf("Error unsubscribing timer.\n");
    return 1;
  }

  return 1;
}


int(video_test_controller)() {
  /* To be completed */
  printf("%s(): under construction\n", __func__);

  return 1;
}

