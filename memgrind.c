#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "mymalloc.h"

#define MAX 4096

void workloadA() {

	char* testA[MAX];
	
	int i;
	for(i = 0; i < 120; i++) {
		testA[i] = (char*)malloc(1); //malloc 1 byte
		printf("Malloc %p \n", &testA[i]);		

		free(testA[i]); //free immediately
		printf("Free %p \n", &testA[i]); 
	}
}

