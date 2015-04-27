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
    uint8_t spread_quantity;
    int32_t delta;
} __attribute__ ((__packed__)) frame_t;

struct input_data
{
    uint8_t instrument_id;
    uint8_t level; 
    uint8_t side;             // 0 is Bidding, 1 is Asking
    uint8_t quantity;
    int32_t price;
};

static int create_cpu_tcp_socket(struct in_addr *, int);
static void calculateDeltas(int, struct input_data *);

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

    data.aBidQuantity = 15;
    data.bAskQuantity = 10;
    data.abSpreadAskQuantity = 10;
    data.aBidPrice = 74150;
    data.bAskPrice = 75500;
    data.abSpreadAskPrice = -1300; 

    calculateDeltas(cpu_socket, &data);
    
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
calculateDeltas(int sock, struct input_data *data)
{
    /* Send Data to Engine via TCP */
    send(sock, &data, sizeof(data), 0);

    /* Receive Data from Engine via TCP */
    frame_t data_received;
    recv(sock, &data_received, sizeof(data_received), 0);


    /* -------- Expected Value -------- */
    
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
    regAbidprice  = (data.instrument_id==0 && data.side==0) ? data.price : regAbidprice;
    regAaskprice  = (data.instrument_id==0 && data.side==1) ? data.price : regAaskprice;

    regBbidprice  = (data.instrument_id==1 && data.side==0) ? data.price : regBbidprice;
    regBaskprice  = (data.instrument_id==1 && data.side==1) ? data.price : regBaskprice;

    regABbidprice = (data.instrument_id==2 && data.side==0) ? data.price : regABbidprice;
    regABbidprice = (data.instrument_id==2 && data.side==1) ? data.price : regABaskprice;

    /* Quantities */
    regAbidquant  = (data.instrument_id==0 && data.side==0) ? data.quant : regAbidquant;
    regAaskquant  = (data.instrument_id==0 && data.side==1) ? data.quant : regAaskquant;

    regBbidquant  = (data.instrument_id==1 && data.side==0) ? data.quant : regBbidquant;
    regBaskquant  = (data.instrument_id==1 && data.side==1) ? data.quant : regBaskquant;

    regABbidquant = (data.instrument_id==2 && data.side==0) ? data.quant : regABbidquant;
    regABbidquant = (data.instrument_id==2 && data.side==1) ? data.quant : regABaskquant;	                   

    /* Nested Ternaries?
     regAbidprice = data.instrument_id == 0 ? 
                       data.side == 0 ? data.price : regAbidprice : 
                       regAbidprice; */

    /*
    if (data.instrument_id == 0)      // Instrument A
    {
	if (!data.side)           // Bidding
	{
	    regAbidprice = data.price;
	    regAbidquant = data.quant;
	}	   
	else                      // Asking
	{
	    regAaskprice = data.price;
	    regAaskquant = data.quant;
	}	
    }
    else if (data.instrument_id == 1) // Instrument B
    {
	if (!data.side)           // Bidding
	{
	    regAbidprice = data.price;
	    regAbidquant = data.quant;
	}	   
	else                      // Asking
	{
	    regAaskprice = data.price;
	    regAaskquant = data.quant;
	}
    }
    else if (data.instrument_id == 2) // Spread A-B
    {
	if (!data.side)           // Bidding
	{
	    regABbidprice = data.price;
	    regABbidquant = data.quant;
	}	   
	else                      // Asking
	{
	    regABaskprice = data.price;
	    regABaskquant = data.quant;
	}
    }
    */

    /*
    switch(data.instrument_id) 
    {
    case 0:                   // Instrument A

	if (!data.side)           // Bidding
	{
	    regAbidprice = data.price;
	    regAbidquant = data.quant;
	}	   
	else                      // Asking
	{
	    regAaskprice = data.price;
	    regAaskquant = data.quant;
	}
	break;

    case 1:                   // Instrument B

	if (!data.side)           // Bidding
	{
	    regAbidprice = data.price;
	    regAbidquant = data.quant;
	}	   
	else                      // Asking
	{
	    regAaskprice = data.price;
	    regAaskquant = data.quant;
	}
	break;

    case 2:                   // Spread AB

	if (!data.side)           // Bidding
	{
	    regAbidprice = data.price;
	    regAbidquant = data.quant;
	}	   
	else                      // Asking
	{
	    regAaskprice = data.price;
	    regAaskquant = data.quant;
	}
	break;	
    }
    */

    uint8_t impliedQuantity = regAbidquant < regBaskquant ? regAbidquant : regBaskquant;
    int32_t impliedBidPrice = regAbidprice - regBaskprice;

    /* Output Parameters */
    int32_t delta = regABaskprice - impliedBidPrice;
    uint8_t spread_quantity = regABaskquant < impliedQuantity ? regABaskquant : impliedQuantity;


    printf("Received: Quantity = %d, Delta = %d\n", data_received.spread_quantity, data_received.delta);
    printf("Expected: Quantity = %d, Delta = %d\n", spread_quantity, delta);
}
