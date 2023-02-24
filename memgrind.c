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

void testC() {

	struct timeval begin, end;
	gettimeofday(&begin, NULL);
	long int time_total = 0;

    char *arraytestC[MAX];
    int i = 0;
    int malloc_count = 0;
    int free_count = 0;
    int total_count = 0;

    time_t t;
    srand((unsigned) time(&t));

    for (i = 0; i < 50; i++) {  /*perform the task 50 times*/

        malloc_count = 0;
        free_count = 0;
        total_count = 0;

        while (total_count < 240) {

            int random_number = rand() % 2;

            if (random_number == 0 && malloc_count < 120) { /*malloc until called 120 times*/

                arraytestC[malloc_count] = (char *)malloc(1);
                malloc_count++;
            }

            if (random_number == 1) { /*free()*/

                if (malloc_count > free_count) {
                    free(arraytestC[free_count]);
                        
                    free_count++;
                    total_count++;
                }
                else {
                    random_number = 0;
                 }

            }

            total_count++;

        }

        gettimeofday(&end, NULL);
	  	time_total = time_total + ((end.tv_sec * 1000000 + end.tv_usec) - (begin.tv_sec * 1000000 + begin.tv_usec));
    }

        long int average_time = time_total/100;	
        printf("Test C - Average time for completion: %ld microseconds\n", average_time);	
}

    /*_______________________________Test_D_______________________________________*/

    /* 
        Test D:  
        3. Randomly choose between
            -> Allocating a 1-byte chunk and storing the pointer in an array
            -> Deallocating one of the chunks in the array (if any)
            Repeat until you have called malloc() 120 times, then free all remaining allocated chunks.

    */

void testD(){

// this is the one i'm working on rn but bc it's not done the code won't compile prop with it so i'll add it to github when im done done
}

    /*_______________________________Test_E_______________________________________*/

    /* 
        Test E:  
        Randomly choose between malloc() and free() 5000 times where each malloc stores a string in each character pointer.
        This tests the programming of malloc in regards to storing data and that free frees up space for more data.
        Also completes the task 50 times to maintain continuity.

    */

void testE(){

	struct timeval begin, end;
	gettimeofday(&begin, NULL);
	long int time_total = 0;

    int i,j;
    int malloc_count = 0;
    int free_count = 0;
    char *arraytestE[MAX];

    time_t t;
    srand((unsigned) time(&t));    

    for (i = 0; i < 50; i++){ /*perform the task 50 times*/

        for (j = 0; j < 5000; j++) { /*perform the task 5000 times*/

            int random_number = rand() % 2;
            char* word = "word";

            if (random_number == 1 && free_count < 2500) {

                if (free_count < malloc_count) {
                    free(arraytestE[free_count]);
                    free_count++;
                }
                else {
                    random_number = 0;
                }
            }

            if(random_number == 0 && malloc_count < 2500) {
                
                arraytestE[malloc_count] = malloc(strlen(word) + 1);
                strcpy(arraytestE[malloc_count], word);
                malloc_count++;

                }
        }

        gettimeofday(&end, NULL);
        time_total = time_total + ((end.tv_sec * 1000000 + end.tv_usec) - (begin.tv_sec * 1000000 + begin.tv_usec));

    }

        long int average_time = time_total/100;	
        printf("Test E - Average time for completion: %ld microseconds\n", average_time);	

}


int main( int number_of_args, char* arg_list[] ) {

    testA();
    testB();
    testC();
    testD();
    testE();

    return (0);
}
