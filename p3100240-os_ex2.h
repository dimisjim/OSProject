#ifndef FUCTION_H
#define FUCTION_H



struct array_s
{
	int first;
	int last;
	int thread_id;
};

void* sort(void *arg);
void mergeSort(int arr[], int l, int r);
void writeOutArray(FILE* fp, int arr[], int size);
int fixarg(char* str);
void printarray(struct array_s * argv);
void merge(int arr[], int l, int m, int r);
void mergeSort(int arr[], int l, int r);
void printArray(int A[], int size);
void writeOutArray(FILE* fp, int arr[], int size);


#endif