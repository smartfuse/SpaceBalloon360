#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "udp_test_util.h"

char *create_buffer() {
    return malloc(sizeof(char) * MAX_BUFFER_LEN);
}

void free_buffer(char **buffer_ref) {
    free(*buffer_ref);
    *buffer_ref = NULL;
}

size_t write_test_buffer(char *buffer, struct TestStruct test_struct) {
    size_t data_len = sizeof(float) + sizeof(uint32_t) + sizeof(char) * test_struct.string_len;
    memset(buffer, 0, MAX_BUFFER_LEN);
    *((float *) buffer) = test_struct.decimal;
    *((uint32_t *) (buffer + sizeof(float))) = test_struct.string_len;
    memcpy(buffer + sizeof(float) + sizeof(uint32_t), test_struct.string, test_struct.string_len);
    return data_len;
}

void read_test_buffer(char *buffer, struct TestStruct *test_struct) {
    test_struct->decimal = *((float *) buffer);
    buffer += sizeof(float);
    test_struct->string_len = *((uint32_t *) buffer);
    buffer += sizeof(uint32_t);
    memset(test_struct->string, 0, MAX_STRING_LEN);
    memcpy(test_struct->string, buffer, test_struct->string_len);
}

void print_test_struct(struct TestStruct test_struct) {
    printf("Test structure: %f %u %s\n", test_struct.decimal, test_struct.string_len, test_struct.string);
}
