#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include "udp_test_util.h"
#include "../udp_client.h"

int main(int argc, char **argv) {
    UdpSession session = start_session(LOCALHOST, TEST_PORT, 0);
    struct TestStruct test_struct;

    char *buffer = create_buffer();

    for (int i = 0; i < 100; i++) {
        sprintf(test_struct.string, "This is a string. Here is a random number %d", i * 1000);
        test_struct.string_len = strlen(test_struct.string);
        test_struct.decimal = 0.5f * i;
        printf("sending ");
        print_struct(test_struct);

        size_t data_size = write_buffer(buffer, test_struct);
        send_data(session, buffer, data_size);

        sleep(1);
    }

    free_buffer(&buffer);
    stop_session(&session);

    return 0;
}
