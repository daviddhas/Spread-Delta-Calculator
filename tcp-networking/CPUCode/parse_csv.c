#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFERSIZE 1024
#define FIELDS 5

struct input_data
{
    int instrument_id;
    int level; 
    int side;             // 0 is Bidding, 1 is Asking
    int quantity;
    int price;
};


int main()
{    
    FILE *stream = fopen("./inputdata.csv", "r");
    
    if(stream == NULL)
    {
	printf("fopen() failed ");
	return -1 ;
    }
    
    char buffer [BUFFERSIZE];
    
    int linum = 0;

    while (fgets (buffer, sizeof(buffer), stream)) 
    {
	printf("<<<%s>>>", buffer);
	linum++;
    }
    
    printf("lines: %d\n", linum);

    return 0;
}

