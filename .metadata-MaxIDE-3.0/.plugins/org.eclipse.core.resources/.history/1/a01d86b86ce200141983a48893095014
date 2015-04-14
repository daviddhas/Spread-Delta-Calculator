/**
 * Document: MaxCompiler tutorial (maxcompiler-tutorial.pdf)
 * Chapter: 4      Example: 2      Name: Simple
 * MaxFile name: Simple
 * Summary:
 * 	 Takes a stream and for each value x calculates x^2 + x.
 */
#include <stdint.h>
#include <MaxSLiCInterface.h>
#include "Maxfiles.h"

int check(float *dataOut, float *expected, int size)
{
	int status = 0;
	for(int i=0; i < size; i++)
	{
		if(dataOut[i] != expected[i]) {
			fprintf(stderr, "Output data @ %d = %1.8g (expected %1.8g)\n",
				i, dataOut[i], expected[i]);
			status = 1;
		}
	}
	return status;
}

void SimpleCPU(int size, float *dataIn, float *dataOut)
{
	for (int i=0 ; i<size ; i++) {
		dataOut[i] = dataIn[i]*dataIn[i] + dataIn[i];
	}
}

float dataIn[1024];
float dataOut[1024];
float expected[1024];
const int size = 1024;

int main()
{
	for(int i = 0; i < size; i++) {
		dataIn[i] = i + 1;
		dataOut[i] = 0;
	}

	SimpleCPU(size, dataIn, expected);
	Simple(size, dataIn, dataOut);

	printf("Running DFE.\n");
	int status = check(dataOut, expected, size);
	if (status)
		printf("Test failed.\n");
	else
		printf("Test passed OK!\n");
	return status;
}
