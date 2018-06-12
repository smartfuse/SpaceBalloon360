#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "udp_client.h"


struct UdpSessionStruct {
    int sockfd;
    struct sockaddr_in addr;
    socklen_t slen;
} UdpSessionStruct;

UdpSession start_session(char *server_address, int server_port, int is_server) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        return NULL;
    }

    UdpSession session = malloc(sizeof(struct UdpSessionStruct));
    session->sockfd = sockfd;
    session->slen = sizeof(struct sockaddr_in);

    struct sockaddr_in *addrRef = &(session->addr);
    memset(addrRef, 0, sizeof(struct sockaddr_in));
    addrRef->sin_family = AF_INET;
    addrRef->sin_port = htons(server_port); // NOLINT
    if (inet_aton(server_address, &addrRef->sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        return NULL;
    }

    if (is_server && bind(sockfd, (const struct sockaddr *) addrRef, sizeof(struct sockaddr)) == -1) {
        fprintf(stderr, "bind() failed\n");
        return NULL;
    }

    return session;
}

void stop_session(UdpSession *session_ref) {
    free(*session_ref);
    *session_ref = NULL;
}

int send_data(UdpSession session, char buffer[], size_t buffer_len) {
    int sockfd = session->sockfd;
    struct sockaddr *addr = (struct sockaddr *) &session->addr;
    socklen_t slen = session->slen;
    if (sendto(sockfd, buffer, buffer_len, 0, addr, slen) < 0) {
        fprintf(stderr, "sendto() failed\n");
        return -1;
    }
    return 0;
}

ssize_t receive_data(UdpSession session, char buffer[], size_t buffer_len) {
    int sockfd = session->sockfd;
    ssize_t recv_len;
    memset(buffer, 0, buffer_len);
    if ((recv_len = (ssize_t) recvfrom(sockfd, buffer, buffer_len, 0, NULL, 0)) < 0) {
        fprintf(stderr, "recvfrom() failed\n");
        return -1;
    }
    return recv_len;
}
