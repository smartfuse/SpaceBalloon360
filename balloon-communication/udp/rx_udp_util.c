#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rx_udp_util.h"

char *create_buffer() {
    return malloc(sizeof(char) * MAX_BUFFER_LEN);
}

void free_buffer(char **buffer_ref) {
    free(*buffer_ref);
    *buffer_ref = NULL;
}

size_t write_buffer(char *buffer, struct RxStruct rx_struct) {
    size_t data_len = sizeof(int) + sizeof(size_t) + sizeof(char) * rx_struct.data_len;
    memset(buffer, 0, MAX_BUFFER_LEN);

    // Pack the checksum field
    *((int *) buffer) = rx_struct.crc_correct;
    buffer += sizeof(int);

    // Pack the data length field
    *((size_t *) buffer) = rx_struct.data_len;
    buffer += sizeof(size_t);

    memcpy(buffer, rx_struct.data, rx_struct.data_len);
    return data_len;
}

void read_buffer(char *buffer, struct RxStruct *rx_struct) {
    // Unpack checksum field
    rx_struct->crc_correct = *((int *) buffer);
    buffer += sizeof(int);

    // Unpack data length field
    rx_struct->data_len = *((size_t *) buffer);
    buffer += sizeof(size_t);

    // Zero out data field
    memset(rx_struct->data, 0, MAX_PACKET_LEN);
    memcpy(rx_struct->data, buffer, rx_struct->data_len);
}
