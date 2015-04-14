/**
 * Document: MaxCompiler tutorial (maxcompiler-tutorial.pdf)
 * Chapter: 4      Example: 2      Name: Simple
 * MaxFile name: Simple
 * Summary:
 * 	 Takes a stream and for each value x calculates x^2 + x.
 */
#include <stdint.h>
#include <stdio.h>
#include <MaxSLiCInterface.h>
#include "Maxfiles.h"

#define SPREADCOUNT 8

void GenerateTestData(int, float *);
void ExpectedValue(int, float *, float *);

/* DataIn Format:
 *   [ SpreadVal1, BuyLeg1, SellLeg1, SpreadVal2, BuyLeg2, ... etc. ]
 */
float dataIn [3*SPREADCOUNT];
float dataOut [3*SPREADCOUNT];
float expected [3*SPREADCOUNT];
const int size = 3*SPREADCOUNT;

int
main()
{
    GenerateTestData(size, dataIn);
    ExpectedValue(size,dataIn,expected);
    Simple(size, dataIn, dataOut);

    for(int i=0; i<SPREADCOUNT; i++)
    {
    	printf("Delta: %f Expected %f\n ", dataOut[i*3], expected[i*3]);
    }

    printf("Running DFE.\n");
    return 0;
}

void
GenerateTestData(int size, float *dataIn)
{
    float a, b, d;

    a = 43;
    b = 29;
    d = -.5;

    for (int i=0; i<size; i=i+3)
    {
    	dataIn[i] = a-b+d;
    	dataIn[i+1] = a;
    	dataIn[i+2] = b;

    	a = a + b*.3;
    	b = b + a*.1;
    	d = d + .1;
    }
}

void
ExpectedValue(int size, float *dataIn, float *dataOut)
{
    float spreadval, buyval, sellval;

    for (int i=0 ; i<size ; i=i+3)
    {
    	spreadval = dataIn[i];
    	buyval = dataIn[i+1];
    	sellval = dataIn[i+2];

    	dataOut[i] = spreadval - buyval + sellval;
    	dataOut[i+1] = -999;
    	dataOut[i+2] = -999;
    }
}
