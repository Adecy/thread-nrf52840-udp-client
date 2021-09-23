#include "udp_client.h"

#include <net/socket.h>
#include <net/net_if.h>

#include <stdio.h>
#include <stdint.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(udp_client, LOG_LEVEL_DBG);

K_THREAD_DEFINE(udp_client, 0x1000, udp_client_entry,
    NULL, NULL, NULL, K_PRIO_PREEMPT(8), 0, 0);

void udp_client_entry(void* _a, void* _b, void* _c)
{
    udp_client_reset();

    for (;;) {
        udp_client_sm();
        k_sleep(K_MSEC(100));
    }
}

enum state { INIT = 1, SENDING = 2, WAITING = 3, RESET = 4 };

enum state udp_state = RESET;

static const char* state_str(enum state state)
{
    static const char* states_str[] = {
        "UNKNOWN",
        "INIT",
        "SENDING",
        "WAITING",
        "RESET",
    };

    if (state < ARRAY_SIZE(states_str)) {
        return states_str[state];
    }

    return states_str[0];
}

static void set_state(enum state new_state)
{
    LOG_INF("UDP client state change %s > %s",
        state_str(udp_state), state_str(new_state));

    udp_state = new_state;
}

void udp_client_sm(void)
{
    switch (udp_state) {
    case INIT:
        if (udp_client_init() == 0) {
            set_state(SENDING);
        } else {
            set_state(RESET);
        }
        break;

    case SENDING:
        if (udp_client_send() > 0) {
            set_state(WAITING);
        } else {
            set_state(RESET);
        }
        break;

    case WAITING:
        k_sleep(K_SECONDS(5));
        set_state(SENDING);
        break;

    case RESET:
        if (udp_client_reset() == 0) {
            set_state(INIT);
            k_sleep(K_SECONDS(1));
        } else {
            k_sleep(K_SECONDS(5));
        }
        break;

    default:
        LOG_ERR("unkown state=%d", udp_state);
    }
}

/*___________________________________________________________________________*/

static struct sockaddr_in6 server_addr;

static int sock;

int udp_client_init(void)
{
    server_addr.sin6_port = htons(UDP_SERVER_PORT);
    server_addr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, UDP_SERVER_IPV6, server_addr.sin6_addr.s6_addr);

    sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock < 0) {
        LOG_ERR("failed to create udp socket, errno=%d", sock);
        return sock;
    }

    int rc = connect(sock, (struct sockaddr*)&server_addr,
        sizeof(struct sockaddr_in6));
    if (rc < 0) {
        LOG_ERR("failed to connect to udp server, errno=%d", sock);
        return rc;
    }

    LOG_INF("UDP client ready to send to %s:%d",
        UDP_SERVER_IPV6, UDP_SERVER_PORT);

    return rc;
}

static uint8_t buffer[128];

int udp_client_send(void)
{
    static uint32_t counter = 0;
    
    const char * message = "Hello from Lucas' Application" \
        "on nrf52840dk(Zephyr RTOS + OpenThread)";
    sprintf(buffer, "Message °%d : %s", counter++, message);

    int sent = send(sock, buffer, strlen(buffer),  0);
    if (sent <= 0) {
        LOG_ERR("failed to send udp message, errno=%d", sent);
    } else {
        LOG_INF("successfully sent %d bytes", sent);
    }
    return sent;
}

int udp_client_reset(void)
{
    close(sock);

    memset(buffer, 0x00, sizeof(buffer));

    return 0;
}