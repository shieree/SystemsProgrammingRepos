#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#define MAX 4096

    /*_______________________________Test_A_______________________________________*/

    /* 
        Test A: 
        1. malloc() and immediately free() a 1-byte chunk, 120 times.

    */

void testA() {

	struct timeval begin, end;
	gettimeofday(&begin, NULL);
	long int time_total = 0;
    
    int i,j;
    char *arraytestA;
    
    for (i = 0; i < 50; i++) /*perform the task 50 times*/
    {   
        for (j = 0; j < 120; j++) /*allocate and free 120 bytes*/
        {
            arraytestA = (char *)malloc(1);
            if(arraytestA == NULL){
                printf("Oops, Not enough memory to complete this request.");
            }
            free(arraytestA);
        }

		gettimeofday(&end, NULL);
	  	time_total = time_total + ((end.tv_sec * 1000000 + end.tv_usec) - (begin.tv_sec * 1000000 + begin.tv_usec));

    }

    	long int average_time = time_total/100;	
        printf("Test A - Average time for completion: %ld microseconds\n", average_time);	
}

    /*_______________________________Test_B_______________________________________*/

    /* 
        Test B:  
        2. Use malloc() to get 120 1-byte chunks, storing the pointers in an array, then use free() to
        deallocate the chunks.

    */

void testB() {

	struct timeval begin, end;
	gettimeofday(&begin, NULL);
	long int time_total = 0;
    
    int i,j;
    char *arraytestB[MAX];

    for (i = 0; i < 50; i++) { /*perform the task 50 times*/

        for (j = 0; j < 120; j++) { /*malloc 120 1-byte chunks*/
            arraytestB[j] = (char *)malloc(1);
        }

        for (j = 0; j < 120; j++) /*free the malloc'd 120 byte chunks*/
        {
            free(arraytestB[j]);
        }

		gettimeofday(&end, NULL);
	  	time_total = time_total + ((end.tv_sec * 1000000 + end.tv_usec) - (begin.tv_sec * 1000000 + begin.tv_usec));

	}

        long int average_time = time_total/100;	
        printf("Test B - Average time for completion: %ld microseconds\n", average_time);	
}

    /*_______________________________Test_C_______________________________________*/

    /* 
        Test C:  
        3. Randomly choose between
            -> Allocating a 1-byte chunk and storing the pointer in an array
            -> Deallocating one of the chunks in the array (if any)
            Repeat until you have called malloc() 120 times, then free all remaining allocated chunks.

    */
/*
void TestC() {

	struct timeval begin, end;
	gettimeofday(&begin, NULL);
	long int time_total = 0;


}
*/

int main( int number_of_args, char* arg_list[] ) {

    testA();
    testB();
    
    return (0);
}
