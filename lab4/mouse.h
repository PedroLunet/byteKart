#ifndef MOUSE_H
#define MOUSE_H

int (mouse_subscribe_int)(uint8_t *bit_no);
int (mouse_unsubscribe_int)();
void (mouse_ih)();
void (mouse_process_scanbyte)();
int (send_mouse_command)(uint8_t command);
void (lcf_print_packet)(uint8_t *packet);

#endif //MOUSE_H
