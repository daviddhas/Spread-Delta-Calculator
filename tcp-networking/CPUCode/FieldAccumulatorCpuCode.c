#define _GNU_SOURCE

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <MaxSLiCInterface.h>
#include "FieldAccumulatorTCP.h"

create_cpu_tcp_socket(struct in_addr *, int);
calculateDeltas(int, const int32_t *, int32_t *);

typedef struct 
{
    int32_t spread_quantity;
    int32_t delta;
} __attribute__ ((__packed__)) frame_t;

int 
main(int argc, char *argv[]) 
{
    if(argc != 4) 
    {
	printf("Usage: %s <dfe_ip> <cpu_ip> <netmask>\n", argv[0]);
	return 1;
    }

    struct in_addr dfe_ip, cpu_ip, netmask;
    const int port = 5007;

    /* Store DFE IP, CPU IP and Netmask */
    inet_aton(argv[1], &dfe_ip);
    inet_aton(argv[2], &cpu_ip);
    inet_aton(argv[3], &netmask);
    
    /* Create DFE Socket, then listen */
    max_file_t *maxfile = FieldAccumulatorTCP_init();
    max_engine_t *engine = max_load(maxfile, "*");
    max_ip_config(engine, MAX_NET_CONNECTION_CH2_SFP1, &dfe_ip, &netmask);
    max_tcp_socket_t *dfe_socket = max_tcp_create_socket(engine, "tcp_ch2_sfp1");
    max_tcp_listen(dfe_socket, port);
    max_tcp_await_state(dfe_socket, MAX_TCP_STATE_LISTEN, NULL);
    
    /* Create TCP Socket on CPU */
    int cpu_socket = create_cpu_tcp_socket(&dfe_ip, port);
    
    /* Send data */
    int32_t quantities[] = { 15, 10, 10 };
    int32_t prices[] = { 74150, 75500, -1300 };
    calculateDeltas(cpu_socket, quantities, prices);
    
    close(cpu_socket);
    
    max_tcp_close(dfe_socket);
    
    max_unload(engine);
    max_file_free(maxfile);
    
    return 0;
}

static int 
create_cpu_tcp_socket(struct in_addr *remote_ip, int port) 
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    
    int state = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &state, sizeof(state));
    
    struct sockaddr_in cpu;
    memset(&cpu, 0, sizeof(cpu));
    cpu.sin_family = AF_INET;
    cpu.sin_port = htons(port);
    cpu.sin_addr = *remote_ip;
    
    connect(sock, (const struct sockaddr*) &cpu, sizeof(cpu));
    
    return sock;
}

static void
calculateDeltas(int sock, const int32_t *quantities, int32_t *prices)
{
    int num_items = sizeof(quantities);
    int32_t data_to_send[2*num_items];
    memcpy(data_to_send, quantities, num_items);
    memcpy(data_to_send + num_items, prices, num_items);

    /* Send Data to Engine via TCP */
    send(sock, &data_to_send, num_items*2, 0);

    /* Receive Data from Engine via TCP */
    frame_t data_received;
    recv(sock, &data_received, sizeof(data_received), 0);

    printf("Received: Quantity = %d, Delta = %d\n", data_received.spread_quantity, data_received.delta);
}
