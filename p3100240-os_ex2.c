#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include "p3100240-os_ex2.h"


int *array;
pthread_t *tids;  //pinakas me ta threads
int threads;
int mode; // 1 = solo r/w. 2 = solo w. 3 = everyone can r/w
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char** argv){
	//Sti prwti thesi tou pinaka orismatwn argv swzetai to onoma tou ektelesimou.
	// printf("Program call:\t\"%s\"\n", argv[0]);
	struct timeval start, end; //for count time
	gettimeofday(&start, NULL);

	int numbers, seed;

	if (argc != 5) {
		printf("\nUsage:\n%s -numbers=X -threads=Y -seed=Z -mode=I\n", argv[0]);
		return 1;
	}

	int n;
	for (n = 1; n < argc; n++) {
		char* tmp;
		if ((tmp = strstr(argv[n], "-numbers=")) != NULL)
		{
			numbers = fixarg(tmp);
		}
		else if ((tmp = strstr(argv[n], "-threads=")) != NULL)
		{
			threads = fixarg(tmp);
		}
		else if ((tmp = strstr(argv[n], "-seed=")) != NULL)
		{
			seed = fixarg(tmp);
		}
		else if ((tmp = strstr(argv[n], "-mode=")) != NULL)
		{
			mode = fixarg(tmp);
		}
		else
		{
			fprintf(stderr, "Error in arguments. The program will now exit.\n");
			return 1;
		}
	}

	printf("Numbers = %d ,Threads = %d , Seed = %d , Mode = %d\n",numbers,threads,seed,mode);

	FILE *results = fopen("./results.dat", "w");

	srand(seed);

	array=(int*)malloc(sizeof(int)*numbers);
	int i;
	for (i = 0; i < numbers; ++i) //get the random numbers to the array
	{
		array[i]=rand();
		// printf("%d\n",array[i] );
	}
	
	int sizeofarray = numbers / threads; //sto struct apo8ikevonte h arxi kai to telos tou pinaka pou 8a ta3inomisei to ka8e thread
	printf("Size of each thread's array is %d\n",sizeofarray );
	int temp = 0;
	struct array_s args[threads];
	for (i = 0; i < threads; ++i)
	{
		args[i].thread_id=i+1; // first thread has id 1
		args[i].first=temp;
		if (i == threads-1)
		{
			args[i].last = numbers;
		}
		else {
			args[i].last = temp + sizeofarray;
		}
		temp += sizeofarray;

		// printf("Gia to thread %d to first = %d kai to last= %d \n",i, args[i].first,args[i].last);
	}

	tids=(pthread_t*)malloc(sizeof(pthread_t)*threads);

	for (i = threads-1; i >= 0; i--) {
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&tids[i], &attr, sort, &args[i]);
		// printf("the thread with num %d is created\n",i );
	}
	
	pthread_join(tids[0], NULL); // this is the last thread to finish, and the only one that is not joined.
	
	fprintf(results, "Initial array=");
	writeOutArray(results, array, numbers);
	// printf("Given array is \n");
	// printArray(array, numbers);

	mergeSort(array, 0, numbers - 1);

	fprintf(results, "\nSorted array=");
	writeOutArray(results, array, numbers);
	// printf("\nSorted array is \n");
	// printArray(array, numbers);

	fclose(results);
	free(array);
	free(tids);

	gettimeofday(&end, NULL);
	printf("The total time of the program is %ld microseconds\n", ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));

	return 0;	
}


int fixarg(char* str){
	//take a string and return the part after the = 
	int i;
	char * pch;
	pch = strtok (str,"=");
	pch = strtok (NULL, "=");
	i = atoi(pch);
	return i;
}


void printarray(struct array_s * argv){
	int i;
	printf("\tThread %d:\n", argv->thread_id);
	for (i = argv->first; i < argv->last; ++i)
	{
		printf("%d\n",array[i]);
	}
}


void* sort(void *arg){
	//printf("to thread 3ekinise \n");
	struct array_s *arg_struct = (struct array_s*) arg;
	// printf("--created thread %d\n", arg_struct->thread_id);
	int x,y;
	pthread_mutex_lock(&mutex);
	printf("Started thread %d for sorting cells from %d to %d.\n", arg_struct->thread_id, arg_struct->first, arg_struct->last-1);
	pthread_mutex_unlock(&mutex);
	if (mode == 1)
	{
		pthread_mutex_lock(&mutex); // lock for reading + writing
	}
	for(x=arg_struct->first; x< arg_struct->last; x++)	//start the sort
	{
		//printf("to thread mpike sto proto loop\n");
		for(y=arg_struct->first; y<arg_struct->last-1; y++)
		{
		//	printf("to tread mpike sto deftero loop\n");
			if(array[y]>array[y+1])
			{
				if (mode == 2)
				{
					pthread_mutex_lock(&mutex); // lock for writing
				}
				int temp = array[y+1];

				array[y+1] = array[y];

				array[y] = temp;
				if (mode == 2)
				{
					pthread_mutex_unlock(&mutex); // unlock for writing
				}
			}
		}

	}
	if (mode == 1)
	{
		pthread_mutex_unlock(&mutex); // unlock for reading + writing
	}
	int i = arg_struct->thread_id;
	// printf("--thread %d done sorting\n", i);

	if (i < threads)
	{
		// printf("--thread %d will join\n", i);
		int join = pthread_join(tids[i], NULL);
		if (join)
		{
			printf("Error code %d. Can't join to %lu\n", join, tids[i]);
			perror("can't join");
			getchar();
		}
		// printf("--thread %d finished waiting %d. Time to print...\n", i, i+1);
	}

	pthread_mutex_lock(&mutex);
	printarray(arg_struct);
	pthread_mutex_unlock(&mutex);

	//printf("to thread teleiose\n");
	// printf("--thread %d exiting\n", i);
	pthread_exit(0);

}


// Merges two subarrays of arr[].
// First subarray is arr[l..m]
// Second subarray is arr[m+1..r]
void merge(int arr[], int l, int m, int r)
{
	int i, j, k;
	int n1 = m - l + 1;
	int n2 =  r - m;

	/* create temp arrays */
	int L[n1], R[n2];

	/* Copy data to temp arrays L[] and R[] */
	for (i = 0; i < n1; i++)
		L[i] = arr[l + i];
	for (j = 0; j < n2; j++)
		R[j] = arr[m + 1+ j];

	/* Merge the temp arrays back into arr[l..r]*/
	i = 0; // Initial index of first subarray
	j = 0; // Initial index of second subarray
	k = l; // Initial index of merged subarray
	while (i < n1 && j < n2)
	{
		if (L[i] <= R[j])
		{
			arr[k] = L[i];
			i++;
		}
		else
		{
			arr[k] = R[j];
			j++;
		}
		k++;
	}

	/* Copy the remaining elements of L[], if there
	   are any */
	while (i < n1)
	{
		arr[k] = L[i];
		i++;
		k++;
	}

	/* Copy the remaining elements of R[], if there
	   are any */
	while (j < n2)
	{
		arr[k] = R[j];
		j++;
		k++;
	}
}

/* l is for left index and r is right index of the
   sub-array of arr to be sorted */
void mergeSort(int arr[], int l, int r)
{
	if (l < r)
	{
		// Same as (l+r)/2, but avoids overflow for
		// large l and h
		int m = l+(r-l)/2;

		// Sort first and second halves
		mergeSort(arr, l, m);
		mergeSort(arr, m+1, r);

		merge(arr, l, m, r);
	}
}

/* Function to print an array (debugging)*/
void printArray(int A[], int size)
{
	int i;
	for (i=0; i < size; i++){
		printf("%d ", A[i]);
		printf("\n");
	}
}

void writeOutArray(FILE* fp, int arr[], int size)
{
	int i;
	fprintf(fp, "{");
	for (i=0; i<size-1; i++){
		fprintf(fp, "%d, ", arr[i]);
	}
	fprintf(fp, "%d}\n", arr[i]);
}
