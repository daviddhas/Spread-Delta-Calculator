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

static int create_cpu_tcp_socket(struct in_addr *, int);
static void calculateDeltas(int, struct input_data *);
static void calculateDeltas2(int, struct input_data *);

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
    struct input_data data;

    /* A Bid Quantity */
    data.instrument_id = 0;
    data.level         = 0;
    data.side          = 0;
    data.quantity      = 15;
    data.price         = 74150;

    calculateDeltas(cpu_socket, &data);

    /* B Ask Quantity */
    data.instrument_id = 1;
    data.level         = 0;
    data.side          = 1;
    data.quantity      = 10;
    data.price         = 75500;

    calculateDeltas(cpu_socket, &data);

    /* AB Ask Quantity */
    data.instrument_id = 2;
    data.level         = 0;
    data.side          = 1;
    data.quantity      = 10;
    data.price         = -1300;

    calculateDeltas2(cpu_socket, &data);
    
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
calculateDeltas2(int sock, struct input_data *data)
{
    /* Send Data to Engine via TCP */
    send(sock, &data, sizeof(data), 0);

    /* Receive Data from Engine via TCP */
    frame_t data_received;
    recv(sock, &data_received, sizeof(data_received), 0);

    printf("Received: Quantity = %d, Delta = %d\n", data_received.spread_quantity, data_received.delta);
}


static void
calculateDeltas(int sock, struct input_data *data)
{
  
    /* Send Data to Engine via TCP */
    send(sock, &data, sizeof(data), 0);

    /* Receive Data from Engine via TCP */
    frame_t data_received;
    recv(sock, &data_received, sizeof(data_received), 0);


    /* -------- Expected Value, Calculated in Software -------- */
    
    static int32_t regAbidprice = 0;
    static uint8_t regAbidquant = 0;

    static int32_t regAaskprice = 0;
    static uint8_t regAaskquant = 0;

    static int32_t regBbidprice = 0;
    static uint8_t regBbidquant = 0;

    static int32_t regBaskprice = 0;
    static uint8_t regBaskquant = 0;

    static int32_t regABbidprice = 0;
    static uint8_t regABbidquant = 0;

    static int32_t regABaskprice = 0;
    static uint8_t regABaskquant = 0;

    /* Prices */
    regAbidprice  = (data->instrument_id==0 && data->side==0) ? data->price : regAbidprice;
    regAaskprice  = (data->instrument_id==0 && data->side==1) ? data->price : regAaskprice;

    regBbidprice  = (data->instrument_id==1 && data->side==0) ? data->price : regBbidprice;
    regBaskprice  = (data->instrument_id==1 && data->side==1) ? data->price : regBaskprice;

    regABbidprice = (data->instrument_id==2 && data->side==0) ? data->price : regABbidprice;
    regABaskprice = (data->instrument_id==2 && data->side==1) ? data->price : regABaskprice;

    /* Quantities */
    regAbidquant  = (data->instrument_id==0 && data->side==0) ? data->quantity : regAbidquant;
    regAaskquant  = (data->instrument_id==0 && data->side==1) ? data->quantity : regAaskquant;

    regBbidquant  = (data->instrument_id==1 && data->side==0) ? data->quantity : regBbidquant;
    regBaskquant  = (data->instrument_id==1 && data->side==1) ? data->quantity : regBaskquant;

    regABbidquant = (data->instrument_id==2 && data->side==0) ? data->quantity : regABbidquant;
    regABaskquant = (data->instrument_id==2 && data->side==1) ? data->quantity : regABaskquant;	                   

    uint8_t impliedQuantity = regAbidquant < regBaskquant ? regAbidquant : regBaskquant;
    int32_t impliedBidPrice = regAbidprice - regBaskprice;

    /* Output Parameters */
    int32_t delta = regABaskprice - impliedBidPrice;
    uint8_t spread_quantity = regABaskquant < impliedQuantity ? regABaskquant : impliedQuantity;

    printf("Received: Quantity = %d, Delta = %d\n", data_received.spread_quantity, data_received.delta);
    printf("Expected: Quantity = %d, Delta = %d\n", spread_quantity, delta);
}
