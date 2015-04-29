#define _GNU_SOURCE

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

//#include <MaxSLiCInterface.h>
#include "FieldAccumulatorTCP.h"

typedef struct output_data
{
    int32_t spread_quantity;
    int32_t delta;
} __attribute__ ((__packed__)) frame_t;

struct input_data
{
    int32_t instrument_id;
    int32_t level; 
    int32_t side;             // 0 is Bidding, 1 is Asking
    int32_t quantity;
    int32_t price;
};

static void calculateDeltas(int, struct input_data *);

static int create_cpu_udp_socket(struct in_addr *local_ip, struct in_addr *remote_ip, int port) {
	int sock = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in cpu;
	memset(&cpu, 0, sizeof(cpu));
	cpu.sin_family = AF_INET;
	cpu.sin_port = htons(port);

	cpu.sin_addr = *local_ip;
	bind(sock, (struct sockaddr *)&cpu, sizeof(cpu));

	cpu.sin_addr = *remote_ip;
    connect(sock, (const struct sockaddr*) &cpu, sizeof(cpu));

	return sock;
}


int 
main(int argc, char *argv[]) 
{
    if(argc != 4) 
    {
	printf("Usage: %s <dfe_ip> <cpu_ip> <netmask>\n", argv[0]);
	return 1;
    }

	struct in_addr dfe_ip;
	inet_aton(argv[1], &dfe_ip);
	struct in_addr cpu_ip;
	inet_aton(argv[2], &cpu_ip);
	struct in_addr netmask;
	inet_aton(argv[3], &netmask);
	const int port = 5008;
    
    /* Create DFE Socket, then listen */
    max_file_t *maxfile = FieldAccumulatorTCP_init();
    max_engine_t *engine = max_load(maxfile, "*");
    max_ip_config(engine, MAX_NET_CONNECTION_CH2_SFP1, &dfe_ip, &netmask);

	max_udp_socket_t *dfe_socket = max_udp_create_socket(engine, "udp_ch2_sfp1");
	max_udp_bind(dfe_socket, port);
	max_udp_connect(dfe_socket, &cpu_ip, port);

	int cpu_socket = create_cpu_udp_socket(&cpu_ip, &dfe_ip, port);

    
    /* Send data */
    struct input_data data;

    /* Set Value */
    data.instrument_id = 0;
    data.level         = 0;
    data.side          = 0;
    data.quantity      = 1;
    data.price         = 2;
    calculateDeltas(cpu_socket, &data);

    /* Hold */
    data.instrument_id = 1;
    data.level         = 0;
    data.side          = 0;
    data.quantity      = 3;
    data.price         = 4;
    calculateDeltas(cpu_socket, &data);

    /* Hold */
    data.instrument_id = 1;
    data.level         = 0;
    data.side          = 1;
    data.quantity      = 5;
    data.price         = 6;
    calculateDeltas(cpu_socket, &data);
    
    /* Set */
    data.instrument_id = 2;
    data.level         = 0;
    data.side          = 0;
    data.quantity      = 7;
    data.price         = 8;
    calculateDeltas(cpu_socket, &data);


	max_udp_close(dfe_socket);
    max_unload(engine);
    max_file_free(maxfile);
    
    return 0;
}

static void
calculateDeltas(int sock, struct input_data *data)
{
    /* Send Data to Engine via TCP */
    send(sock, data, sizeof(struct input_data), 0);

    /* Receive Data from Engine via TCP */
    frame_t data_received;

    int e = recv(sock, &data_received, sizeof(struct output_data), 0);

    printf("Received: Quantity = %d, Delta = %d\n", data_received.spread_quantity, data_received.delta);
    if (e == -1)
    {
    	printf("No bytes recieved\n");
    	exit(0);
    }
}
