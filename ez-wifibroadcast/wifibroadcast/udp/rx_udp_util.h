#ifndef _WIFIBROADCAST_RX_UDP_H
#define _WIFIBROADCAST_RX_UDP_H

#define MAX_PACKET_LEN 4192
#define MAX_BUFFER_LEN 8384

#include <stdlib.h>
#include <inttypes.h>

struct RxStruct {
    int crc_correct;
    uint32_t data_len;
    uint8_t data[MAX_PACKET_LEN];
} DataStruct;


char *create_buffer();
void free_buffer(char **buffer_ref);
size_t write_buffer(char *buffer, struct RxStruct rx_struct);
void read_buffer(char *buffer, struct RxStruct *rx_struct);

#endif
