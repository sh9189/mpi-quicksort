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

int binary_search(int a[], int low, int high, int target) {
	register int middle;
	while (low <= high) {
		middle = low + (high - low)/2;
		if (target < a[middle])
			high = middle - 1;
		else
			low = middle + 1;

	}
	return low;
}

int merge(int * inputArr,int inputElements, int *mergeArr,int mergeElements, int * outputArr)
{
	int inputIndex = 0;
	int mergeIndex  = 0;
	int outputIndex = 0;
	while(inputIndex < inputElements && mergeIndex < mergeElements)
	{
		if(inputArr[inputIndex] <= mergeArr[mergeIndex])
			outputArr[outputIndex++] = inputArr[inputIndex++];
		else
			outputArr[outputIndex++] = mergeArr[mergeIndex++];

	}
	while(inputIndex < inputElements)
		outputArr[outputIndex++] = inputArr[inputIndex++];
	while(mergeIndex < mergeElements)
		outputArr[outputIndex++] = mergeArr[mergeIndex++];
	return outputIndex;



}



int* mpiqsort(int* input, int globalNumElements, int* dataLengthPtr, MPI_Comm comm, int commRank, int commSize) {

	int numElements = *dataLengthPtr;
#ifdef DEBUG
	printf("Before Sorting Rank is %d\n",commRank);
	printArray(input,0,numElements-1);
#endif
	qsort(input,numElements,sizeof(int),compare);

	int *medianArray;
	int *mirrorArr = (int *)calloc(globalNumElements,sizeof(int));
	int *mergeArr = (int *)calloc(globalNumElements,sizeof(int));
	int *currentArr;
	int *currentMirrorArr;
	int ping = 1;
	int numGreaterElements;
	int numLessElements;
	MPI_Comm newComm;
	while(1)
	{
		if(ping)
		{
			currentArr = input;
			currentMirrorArr = mirrorArr;
		}
		else
		{
			currentArr = mirrorArr;
			currentMirrorArr = input;
		}


		if(commSize == 1) // terminating condition
		{
			free(currentMirrorArr);
			free(mergeArr);
			*dataLengthPtr = numElements;
			return currentArr;
		}

		int localMedianIndex,globalMedianIndex;
		if(numElements % 2 == 0)
			localMedianIndex = numElements/2 -1;
		else
			localMedianIndex = numElements/2;

		int localMedian = currentArr[localMedianIndex];
#ifdef DEBUG
		printf("Rank is %d localMedian is %d\n",commRank,localMedian);
#endif
		medianArray = (int *)malloc(sizeof(int)*commSize);
		MPI_Allgather(&localMedian,1,MPI_INT, medianArray, 1,MPI_INT,comm);

		qsort(medianArray,commSize,sizeof(int),compare);
#ifdef DEBUG
		printf("Rank is %d median Array is\n",commRank);
		printArray(medianArray,0,commSize-1);
#endif
		if(commSize % 2 == 0)
			globalMedianIndex = commSize/2 -1;
		else
			globalMedianIndex = commSize/2;
		int globalMedian = medianArray[globalMedianIndex];
#ifdef DEBUG
		printf("Rank is %d GlobalMedian is %d\n",commRank,globalMedian);
#endif
		int lowIndex = binary_search(currentArr, 0, numElements-1, globalMedian);
#ifdef DEBUG
		printf("Rank is %d LowIndex is %d\n",commRank,lowIndex);
#endif

		if(commRank < commSize/2)
		{
			numGreaterElements = numElements-lowIndex;
			MPI_Send(&numGreaterElements,1,MPI_INT,commSize-1-commRank,0,comm);
			MPI_Send(currentArr+lowIndex,numGreaterElements,MPI_INT,commSize-1-commRank,0,comm);
#ifdef DEBUG
			printf("Rank is %d Sending Greater Arr\n",commRank);
			printArray(currentArr+lowIndex,0,numGreaterElements-1);
#endif
			MPI_Recv(&numLessElements,1,MPI_INT,commSize-1-commRank,0,comm,NULL);
			MPI_Recv(mergeArr,numLessElements,MPI_INT,commSize-1-commRank,0,comm,NULL);
#ifdef DEBUG
			printf("Rank is %d Received Lesser Arr\n",commRank);
			printArray(mergeArr,0,numLessElements-1);
#endif
			numElements = merge(currentArr,lowIndex,mergeArr,numLessElements,currentMirrorArr);
		}
		else
		{
			MPI_Recv(&numGreaterElements,1,MPI_INT,commSize-1-commRank,0,comm,NULL);
			MPI_Recv(mergeArr,numGreaterElements,MPI_INT,commSize-1-commRank,0,comm,NULL);
#ifdef DEBUG
			printf("Rank is %d Received Greater Arr\n",commRank);
			printArray(mergeArr,0,numGreaterElements-1);
#endif
			numLessElements = lowIndex;
			MPI_Send(&numLessElements,1,MPI_INT,commSize-1-commRank,0,comm);
			MPI_Send(currentArr,numLessElements,MPI_INT,commSize-1-commRank,0,comm);
#ifdef DEBUG
			printf("Rank is %d Sending Lesser Arr\n",commRank);
			printArray(currentArr,0,numLessElements-1);
#endif

			numElements = merge(currentArr+lowIndex,numElements-lowIndex,mergeArr,numGreaterElements,currentMirrorArr);
		}
#ifdef DEBUG
		printf("Rank is %d NumElements is %d After Merging Arr is\n",commRank,numElements);
		printArray(currentMirrorArr,0,numElements-1);
#endif


		if(commRank < commSize/2)
			MPI_Comm_split(comm,0,commRank,&newComm);
		else
			MPI_Comm_split(comm,1,commRank,&newComm);

		comm = newComm;
		MPI_Comm_rank( comm, &commRank );
		MPI_Comm_size( comm, &commSize );
#ifdef DEBUG
		printf("After Splitting Rank is %d CommSize is %d\n",commRank,commSize);
#endif
		ping = !ping;


	}

}

