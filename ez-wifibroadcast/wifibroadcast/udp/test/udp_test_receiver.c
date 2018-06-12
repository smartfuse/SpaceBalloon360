#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "udp_test_util.h"
#include "../udp_client.h"

int main(int argc, char **argv) {
    UdpSession session = start_session(LOCALHOST, TEST_PORT, 1);
    struct TestStruct test_struct;

    char *buffer = create_buffer();

    while (1) {
        receive_data(session, buffer, MAX_BUFFER_LEN);
        read_buffer(buffer, &test_struct);
        printf("receiving ");
        print_struct(test_struct);
    }
}
