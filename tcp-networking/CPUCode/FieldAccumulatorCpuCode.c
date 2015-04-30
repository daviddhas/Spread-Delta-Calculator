#define _GNU_SOURCE

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <MaxSLiCInterface.h>
#include "FieldAccumulatorTCP.h"

#define BUFFERSIZE 1024
#define FIELDS 5

typedef struct output_data
{
    int32_t spread_quantity;
    int32_t spread_delta;
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
static void validateData(struct input_data *, struct output_data *);
static int create_cpu_udp_socket(struct in_addr *, struct in_addr *, int);
static void parse(char *, struct input_data *);
static void sendDataPacket(int sock, int32_t f1, int32_t f2, int32_t f3, int32_t f4, int32_t f5)

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
    
    /*
    FILE *stream = fopen("./inputdata.csv", "r");
    if(stream == NULL)
    {
	printf("fopen() failed ");
	return -1 ;
    }

    char line[BUFFERSIZE];

    char *to_be_free = line;

    // Ignore Header File 
    fgets(line, BUFFERSIZE, stream);

    int linum = 0;

    while (fgets(line, sizeof(line), stream))
    {
    	struct input_data data; 
	parse(line, &data);
    	calculateDeltas(cpu_socket, &data);
	linum++;
    }

    printf("number of lines: %d\n",linum);
    */

    sendDataPacket(cpu_socket,0,0,1,3,1293);
    sendDataPacket(cpu_socket,1,0,0,46,1140);
    sendDataPacket(cpu_socket,2,0,1,12,-231);
    sendDataPacket(cpu_socket,0,0,1,21,1229);
    sendDataPacket(cpu_socket,1,0,0,19,1496);
    sendDataPacket(cpu_socket,2,0,1,49,228);
    sendDataPacket(cpu_socket,0,0,1,25,1272);
    sendDataPacket(cpu_socket,1,0,0,49,1470);
    sendDataPacket(cpu_socket,2,0,1,15,-242);
    sendDataPacket(cpu_socket,0,0,1,13,1114);
    sendDataPacket(cpu_socket,1,0,0,40,1159);
    sendDataPacket(cpu_socket,2,0,1,16,63);
    sendDataPacket(cpu_socket,0,0,1,48,1229);
    sendDataPacket(cpu_socket,1,0,0,34,1325);
    sendDataPacket(cpu_socket,2,0,1,19,365);
    sendDataPacket(cpu_socket,0,0,1,41,1153);
    sendDataPacket(cpu_socket,1,0,0,28,1481);
    sendDataPacket(cpu_socket,2,0,1,16,-162);
    sendDataPacket(cpu_socket,0,0,1,34,1151);
    sendDataPacket(cpu_socket,1,0,0,0,1246);
    sendDataPacket(cpu_socket,2,0,1,18,165);
    sendDataPacket(cpu_socket,0,0,1,2,1276);
    sendDataPacket(cpu_socket,1,0,0,10,1371);
    sendDataPacket(cpu_socket,2,0,1,48,-530);
    sendDataPacket(cpu_socket,0,0,1,48,1477);
    sendDataPacket(cpu_socket,1,0,0,32,1336);
    sendDataPacket(cpu_socket,2,0,1,31,-142);
    sendDataPacket(cpu_socket,0,0,1,34,1135);
    sendDataPacket(cpu_socket,1,0,0,29,1315);
    sendDataPacket(cpu_socket,2,0,1,1,-296);

    max_udp_close(dfe_socket);
    max_unload(engine);
    max_file_free(maxfile);
    
    return 0;
}

static void
parse(char *line, struct input_data *in)
{
    int i;
    int fv[FIELDS];
    
    char *element = strtok(line, ",");
    fv[0] = atoi(element);
    
    for (i=1; i<FIELDS; i++)
    {
	char *element = strtok(NULL, ",");
	fv[i] = atoi(element);
    }
    
    in->instrument_id = fv[0];
    in->level         = fv[1];
    in->side          = fv[2];
    in->quantity      = fv[3];
    in->price         = fv[4];
}

static void
calculateDeltas(int sock, struct input_data *data)
{
    // Send Data to Engine via TCP
    send(sock, data, sizeof(struct input_data), 0);

    // Receive Data from Engine via TCP
    frame_t data_received, data_expected;
    validateData(data, &data_expected);
    int e = recv(sock, &data_received, sizeof(struct output_data), 0);

    printf("Received: Quantity = %d, Delta = %d\n", data_received.spread_quantity, data_received.spread_delta);
    printf("Expected: Quantity = %d, Delta = %d\n", data_expected.spread_quantity, data_expected.spread_delta);
    if (e == -1)
    {
        printf("No bytes recv\n");
        exit(0);
    }
}

static void
validateData(struct input_data *in, struct output_data *out)
{
    // -------- Expected Value, Calculated in Software --------

    // Instrument A
    static int32_t a_bidprice = 0;
    static int32_t a_bidquant = 0;
    static int32_t a_askprice = 0;
    static int32_t a_askquant = 0;

    // Instrument B
    static int32_t b_bidprice = 0;
    static int32_t b_bidquant = 0;
    static int32_t b_askprice = 0;
    static int32_t b_askquant = 0;

    // Instrument A-B Spread
    static int32_t ab_bidprice = 0;
    static int32_t ab_bidquant = 0;
    static int32_t ab_askprice = 0;
    static int32_t ab_askquant = 0;

    // Update Correct Register
    if(in->instrument_id==0 && in->side==0 && in->level==0) { a_bidprice = in->price;  a_bidquant = in->quantity; }
    if(in->instrument_id==0 && in->side==1 && in->level==0) { a_askprice = in->price;  a_askquant = in->quantity; }
    if(in->instrument_id==1 && in->side==0 && in->level==0) { b_bidprice = in->price;  b_bidquant = in->quantity; }
    if(in->instrument_id==1 && in->side==1 && in->level==0) { b_askprice = in->price;  b_askquant = in->quantity; }
    if(in->instrument_id==2 && in->side==0 && in->level==0) { ab_bidprice = in->price; ab_bidquant = in->quantity; }
    if(in->instrument_id==2 && in->side==1 && in->level==0) { ab_askprice = in->price; ab_askquant = in->quantity; }

    // Implied AB Bid
    int32_t impliedBidPrice = a_bidprice - b_askprice;
    int32_t impliedQuantity = a_bidquant < b_askquant ? a_bidquant : b_askquant;

    // Output Parameters
    out->spread_delta = ab_askprice - impliedBidPrice;
    out->spread_quantity = ab_askquant < impliedQuantity ? ab_askquant : impliedQuantity;
}

static int 
create_cpu_udp_socket(struct in_addr *local_ip, struct in_addr *remote_ip, int port) 
{
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

static void
sendDataPacket(int sock, int32_t f1, int32_t f2, int32_t f3, int32_t f4, int32_t f5)
{
    struct input_data data;
    data.instrument_id = f1;
    data.level         = f2;
    data.side          = f3;
    data.quantity      = f4;
    data.price         = f5;
    calculateDeltas(cpu_socket, &data);
}
