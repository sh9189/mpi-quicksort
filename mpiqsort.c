#include "header.h"

int compare (const void * a, const void * b)
{
	return ( *(int*)a - *(int*)b );
}

void printArray(int arr[],int start,int end)
{
	int i;
	for(i=start;i<=end;i++)
		printf("%d ",arr[i]);
	printf("\n");
}

int* mpiqsort(int* input, int globalNumElements, int* dataLengthPtr, MPI_Comm comm, int commRank, int commSize) {

	int numElements = * dataLengthPtr;
	printf("Before Sorting Rank is %d\n",commRank);
	printArray(input,0,numElements-1);

	qsort(input,numElements,sizeof(int),compare);
	printf("After Sorting Rank is %d\n",commRank);
	printArray(input,0,numElements-1);
	return input;
}

