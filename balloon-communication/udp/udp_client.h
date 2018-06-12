
#ifndef _UDP_CLIENT_H
#define _UDP_CLIENT_H 1

typedef struct UdpSessionStruct* UdpSession;

UdpSession start_session(char *remote_address, int remote_port, int receive);
int send_data(UdpSession session, char buffer[], size_t buffer_len);
ssize_t receive_data(UdpSession session, char buffer[], size_t buffer_len);
void stop_session(UdpSession *session_ref);

#endif