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
#define INSTRUMENTS 2

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
    struct input_data a;
    struct input_data ai;
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

    struct in_addr dfe_ip;
    inet_aton(argv[1], &dfe_ip);
    struct in_addr cpu_ip;
    inet_aton(argv[2], &cpu_ip);
    struct in_addr netmask;
    inet_aton(argv[3], &netmask);
    const int port = 5008;
    
    /* Create DFE Socket, then listen */
    max_file_t *maxfile = FieldAccumulator_init();
    max_engine_t *engine = max_load(maxfile, "*");
    max_ip_config(engine, MAX_NET_CONNECTION_QSFP_TOP_10G_PORT1, &dfe_ip, &netmask);
    max_udp_socket_t *dfe_socket = max_udp_create_socket(engine, "udp_ch2_sfp1");
    max_udp_bind(dfe_socket, port);
    max_udp_connect(dfe_socket, &cpu_ip, port);
    
    int cpu_socket = create_cpu_udp_socket(&cpu_ip, &dfe_ip, port);
    
    FILE *stream = fopen("./source_data2.csv", "r");

    if(stream == NULL)
    {
        printf("fopen() failed ");
        return -1 ;
    }

    char line[BUFFERSIZE];

    /* Ignore Header File */
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


//    printf(">>> Sending: id=%d, side=%d, level=%d, q=%d, p=%d\n",in->instrument_id,in->side,in->level,in->quantity, in->price);

}

static void
calculateDeltas(int sock, struct input_data *data)
{
    frame_t instruments, instruments_exp;
	int32_t bytesRecv;

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
    	printf("WARNING: Received less bytes than expected");
    }

    printf("===== Bytes Received: %d =====\n", bytesRecv);

    char valid [INSTRUMENTS];

    valid[0] = isEqual(&instruments.a, &instruments_exp.a) ? 'v' : 'x';
    valid[1] = isEqual(&instruments.ai, &instruments_exp.ai) ? 'v' :'x';

    printf("[%c] Instrument A:   Q = %d, Price = %d\n", valid[0], instruments.a.quantity, instruments.a.price);
    printf("[%c] Instrument Ai:  Q = %d, Price = %d\n", valid[1], instruments.ai.quantity, instruments.ai.price);
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

    // Implied Instrument
    int32_t ai_bidquant = ab_bidquant < b_askquant ? ab_bidquant : b_askquant;
    int32_t ai_bidprice = ab_bidprice + b_askprice;

    // Output Parameters
    out->a.instrument_id = 0;
    out->a.level = 0;
    out->a.side = 0;
    out->a.quantity = a_bidquant;
    out->a.price = a_bidprice;

    out->ai.instrument_id = 0;
    out->ai.level = 0;
    out->ai.side = 0;
    out->ai.quantity = ai_bidquant;
    out->ai.price = ai_bidprice;
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

static int
isEqual(struct input_data *a, struct input_data *b)
{
	if (a->instrument_id != b->instrument_id)
	{
		//printf("ID MISMATCH\n");
		return 0;
	}
	else if (a->level != b->level)
	{
		//printf("LEVEL MISMATCH\n");
		return 0;
	}
	else if (a->side != b->side)
	{
		//printf("SIDE MISMATCH\n");
		return 0;
	}
	else if (a->quantity != b->quantity)
	{
		//printf("QUANTITY MISMATCH\n");
		return 0;
	}
	else if (a->price != b->price)
	{
		//printf("PRICE MISMATCH\n");
		return 0;
	}
	return 1;
}

