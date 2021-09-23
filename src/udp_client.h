#ifndef _UDP_CLIENT_H
#define _UDP_CLIENT_H

#include <zephyr.h>

#define UDP_SERVER_PORT 1234
#define UDP_SERVER_IPV6 "fd4a:616d:f57:3b81:31ff:3c7e:5961:ac51"

void udp_client_entry(void*, void*, void*);

void udp_client_sm(void);

int udp_client_init(void);

int udp_client_send(void);

int udp_client_reset(void);


#endif