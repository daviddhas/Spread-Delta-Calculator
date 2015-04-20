#define _GNU_SOURCE

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <MaxSLiCInterface.h>
#include "FieldAccumulatorTCP.h"


typedef struct {
	uint32_t total_items;
	uint32_t sum;
} __attribute__ ((__packed__)) frame_t;


static int create_cpu_tcp_socket(struct in_addr *remote_ip, int port) {
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


static void exchangeItems(int sock, const uint8_t* items, uint8_t num_items) {
	static uint32_t total_items = 0;

	uint8_t data_to_send[1 + UINT8_MAX];
	data_to_send[0] = num_items;
	memcpy(data_to_send + 1, items, num_items);

	send(sock, &data_to_send, num_items + 1, 0);
	frame_t data_received;

	recv(sock, &data_received, sizeof(data_received), 0);

	printf("Received: total_items = %u, sum = %u\n", data_received.total_items, data_received.sum);

	total_items += num_items;

	uint32_t sum = 0;
	for (int i = 0; i < num_items; i++)
		sum += items[i];

	if (data_received.total_items != total_items || data_received.sum != sum) {
		printf("Error! Expected: total_items = %u, sum = %u\n", total_items, sum);
		exit(1);
	}
}


int main(int argc, char *argv[]) {
	if(argc != 4) {
		printf("Usage: %s <dfe_ip> <cpu_ip> <netmask>\n", argv[0]);
		return 1;
	}

	struct in_addr dfe_ip;
	inet_aton(argv[1], &dfe_ip);
	struct in_addr cpu_ip;
	inet_aton(argv[2], &cpu_ip);
	struct in_addr netmask;
	inet_aton(argv[3], &netmask);
	const int port = 5007;

	max_file_t *maxfile = FieldAccumulatorTCP_init();
	max_engine_t *engine = max_load(maxfile, "*");

	max_ip_config(engine, MAX_NET_CONNECTION_CH2_SFP1, &dfe_ip, &netmask);

	max_tcp_socket_t *dfe_socket = max_tcp_create_socket(engine, "tcp_ch2_sfp1");
	max_tcp_listen(dfe_socket, port);
	max_tcp_await_state(dfe_socket, MAX_TCP_STATE_LISTEN, NULL);

	int cpu_socket = create_cpu_tcp_socket(&dfe_ip, port);

	uint8_t items1[] = { 2, 5, 8, 1 };
	exchangeItems(cpu_socket, items1, sizeof(items1));

	uint8_t items2[] = { 50, 2 };
	exchangeItems(cpu_socket, items2, sizeof(items2));

	uint8_t items3[] = { 1, 2, 3, 4, 5, 6 };
	exchangeItems(cpu_socket, items3, sizeof(items3));

	close(cpu_socket);

	max_tcp_close(dfe_socket);

	max_unload(engine);
	max_file_free(maxfile);

	return 0;
}
