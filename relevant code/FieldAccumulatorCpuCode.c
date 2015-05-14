/* Ticker Plant System Implemented in Max Compiler 
 * Columbia University: CSEE 4840, Spring 2015
 * May 14th 2015
 *
 *   - Gabriel Blanco
 *   - Suchith Vasudevan
 *   - Brian Bourn
 *   - David Naveen Dhas Arthur
 */
#define _GNU_SOURCE

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "Maxfiles.h"
#include <MaxSLiCInterface.h>

#define BUFFERSIZE 1024
#define FIELDS 5

struct input_data
{
    int32_t instrument_id;
    int32_t level; 
    int32_t side;             // 0 is Bidding, 1 is Asking
    int32_t quantity;
    int32_t price;
};

typedef struct output_data
{
    struct input_data a_bid;
    struct input_data ai_bid;
    struct input_data b_ask;
    struct input_data bi_ask;
    struct input_data ab_bid;
    struct input_data abi_bid;
} __attribute__ ((__packed__)) frame_t;

static void calculateDeltas(int, struct input_data *);
static void validateData(struct input_data *, struct output_data *);
static int  create_cpu_udp_socket(struct in_addr *, struct in_addr *, int);
static void parse(char *, struct input_data *);
static int isEqual(struct input_data *, struct input_data *);

int 
main(int argc, char *argv[]) 
{
    if(argc != 4) 
    {
        printf("Usage: %s <dfe_ip> <cpu_ip> <netmask>\n", argv[0]);
        return 1;
    }

    struct in_addr dfe_ip, cpu_ip, netmask;
    const int port = 5008;
    char line[BUFFERSIZE];

    inet_aton(argv[1], &dfe_ip);
    inet_aton(argv[2], &cpu_ip);
    inet_aton(argv[3], &netmask);
    
    // Create DFE Socket, then listen
    max_file_t *maxfile = FieldAccumulator_init();
    max_engine_t *engine = max_load(maxfile, "*");
    max_ip_config(engine, MAX_NET_CONNECTION_QSFP_TOP_10G_PORT1, &dfe_ip, &netmask);
    max_udp_socket_t *dfe_socket = max_udp_create_socket(engine, "udp_ch2_sfp1");
    max_udp_bind(dfe_socket, port);
    max_udp_connect(dfe_socket, &cpu_ip, port);
    
    int cpu_socket = create_cpu_udp_socket(&cpu_ip, &dfe_ip, port);


    // CSV Input Data Parsing
    FILE *stream = fopen("./source_data2.csv", "r");
    
    if(stream == NULL)
    {
        printf("fopen() failed ");
        return -1 ;
    }
    
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
    
    max_udp_close(dfe_socket);
    max_unload(engine);
    max_file_free(maxfile);
    
    return 0;
}

/* 
 * Parses a CSV file into an input structs
 */
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

/* 
 * Sending and Receiving input data via UDP, then validating and printing results
 */
static void
calculateDeltas(int sock, struct input_data *data)
{
    frame_t instruments, instruments_exp;
    int32_t bytesRecv, num_instr;
    
    num_instr = (int32_t)sizeof(struct output_data) / (int32_t)sizeof(struct input_data);
    
    // Send Data to Engine via TCP
    send(sock, data, sizeof(struct input_data), 0);

    // Receive Data from Engine via TCP
    validateData(data, &instruments_exp);
    bytesRecv = recv(sock, &instruments, sizeof(struct output_data), 0);
    if (bytesRecv == -1)
    {
        printf("No bytes recv\n");
        exit(0);
    }
    else if (bytesRecv < (int32_t)sizeof(struct output_data))
    {
        printf("WARNING: Received less bytes than expected\n");
    }

    printf("===== Bytes Received: %d =====\n", bytesRecv);

    char valid [num_instr];

    valid[0] = isEqual(&instruments.a_bid, &instruments_exp.a_bid) ?     'v' : 'x';
    valid[1] = isEqual(&instruments.ai_bid, &instruments_exp.ai_bid) ?   'v' : 'x';
    valid[2] = isEqual(&instruments.b_ask, &instruments_exp.b_ask) ?     'v' : 'x';
    valid[3] = isEqual(&instruments.bi_ask, &instruments_exp.bi_ask) ?   'v' : 'x';
    valid[4] = isEqual(&instruments.ab_bid, &instruments_exp.ab_bid) ?   'v' : 'x';
    valid[5] = isEqual(&instruments.abi_bid, &instruments_exp.abi_bid) ? 'v' : 'x';

    printf("[%c] Instr A   BID:  Q = %d, Price = %d\n", valid[0], instruments.a_bid.quantity, instruments.a_bid.price);
    printf("[%c] Instr Ai  BID:  Q = %d, Price = %d\n", valid[1], instruments.ai_bid.quantity, instruments.ai_bid.price);
    printf("[%c] Instr B   ASK:  Q = %d, Price = %d\n", valid[2], instruments.b_ask.quantity, instruments.b_ask.price);
    printf("[%c] Instr Bi  ASK:  Q = %d, Price = %d\n", valid[3], instruments.bi_ask.quantity, instruments.bi_ask.price);
    printf("[%c] Instr AB  BID:  Q = %d, Price = %d\n", valid[4], instruments.ab_bid.quantity, instruments.ab_bid.price);
    printf("[%c] Instr ABi BID:  Q = %d, Price = %d\n", valid[5], instruments.abi_bid.quantity, instruments.abi_bid.price);
}


/*
 * Runs the same calculations as in the DFE Engine to Validate the received results
 */
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

    // Implied Instrument
    int32_t ai_bidquant = ab_bidquant < b_askquant ? ab_bidquant : b_askquant;
    int32_t ai_bidprice = ab_bidprice + b_askprice;
    int32_t bi_askquant = a_bidquant < ab_bidquant ? a_bidquant : ab_bidquant;
    int32_t bi_askprice = a_bidprice - ab_bidprice;
    int32_t abi_bidquant = a_bidquant < b_askquant ? a_bidquant : b_askquant;
    int32_t abi_bidprice = a_bidprice - b_askprice;

    // Output Parameters
    out->a_bid.instrument_id = 0;
    out->a_bid.level = 0;
    out->a_bid.side = 0;
    out->a_bid.quantity = a_bidquant;
    out->a_bid.price = a_bidprice;

    out->ai_bid.instrument_id = 0;
    out->ai_bid.level = 0;
    out->ai_bid.side = 0;
    out->ai_bid.quantity = ai_bidquant;
    out->ai_bid.price = ai_bidprice;

    out->b_ask.instrument_id = 1;
    out->b_ask.level = 0;
    out->b_ask.side = 1;
    out->b_ask.quantity = b_askquant;
    out->b_ask.price = b_askprice;

    out->bi_ask.instrument_id = 1;
    out->bi_ask.level = 0;
    out->bi_ask.side = 1;
    out->bi_ask.quantity = bi_askquant;
    out->bi_ask.price = bi_askprice;

    out->ab_bid.instrument_id = 2;
    out->ab_bid.level = 0;
    out->ab_bid.side = 0;
    out->ab_bid.quantity = ab_bidquant;
    out->ab_bid.price = ab_bidprice;

    out->abi_bid.instrument_id = 2;
    out->abi_bid.level = 0;
    out->abi_bid.side = 0;
    out->abi_bid.quantity = abi_bidquant;
    out->abi_bid.price = abi_bidprice;
}


/*
 * Create a UDP Socket on the CPU
 */
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

/*
 * Comparison between input data structs, used for validation
 */
static int
isEqual(struct input_data *a, struct input_data *b)
{
    if (a->instrument_id != b->instrument_id)
    {
        printf("ID MISMATCH\n");
        return 0;
    }
    else if (a->level != b->level)
    {
        printf("LEVEL MISMATCH\n");
        return 0;
    }
    else if (a->side != b->side)
    {
        printf("SIDE MISMATCH\n");
        return 0;
    }
    else if (a->quantity != b->quantity)
    {
        printf("QUANTITY MISMATCH\n");
        return 0;
    }
    else if (a->price != b->price)
    {
        printf("PRICE MISMATCH\n");
        return 0;
    }
    return 1;
}
