#ifndef RTL8139_H
#define RTL8139_H

#include <stdint.h>

void rtl8139_init(void);
void rtl8139_send_packet(void *data, uint16_t len);
int rtl8139_receive_packet(uint8_t *buf, uint32_t max_size);
void rtl8139_handler(void);

#endif

