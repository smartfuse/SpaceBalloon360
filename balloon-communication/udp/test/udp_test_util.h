#ifndef _UDP_TEST_UTIL_H
#define _UDP_TEST_UTIL_H 1

#define LOCALHOST "127.0.0.1"
#define TEST_PORT 6000
#define MAX_STRING_LEN 500
#define MAX_BUFFER_LEN 8392

struct TestStruct {
    float decimal;
    size_t string_len;
    char string[MAX_STRING_LEN];
} TestStruct;

char *create_buffer();
void free_buffer(char **buffer_ref);

size_t write_test_buffer(char *buffer, struct TestStruct test_struct);
void read_test_buffer(char *buffer, struct TestStruct *test_struct);
void print_test_struct(struct TestStruct test_struct);

#endif
