/* This code is calculate the number of prime from start value to end value
   I use sieve of erathosthenus. so If your end_value is so big, you can be
   fail to malloc the array of char(which is used to check the not prime number)
   thank you.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <getopt.h>

int tn = 0;	                       // this value is for thread ID.
char *arr;                         /* this array is used for algorithm. 
								      if index i's value is not zero, this means i*2+1 is prime number */

int num_thread     			 =  1; // the number of thread.
unsigned long start_value    =  1; 
unsigned long end_value  	 =  0; 
unsigned long end_value_root;      // root value of end_value.
unsigned long array_size;          // size of char array.

/* I use algorithm named Sieve of Erathosthenes. so first make array that each index i mean value i*2+1.
   and If arr[i] is not zero, that means 2*i+1 is prime. value of thread_num means thread'ID so If thread \
   ID is zero, that thread check index 1, 1+num_thread, 1+2*num_thread... so I can check all index separately.
 */
void *check_multiple(void *data)
{
	int thread_num   = __sync_fetch_and_add(&tn, 1); // get thread ID.
	unsigned long i, j;
	
	for(i= 1 + thread_num  ; (2*i + 1) <= end_value_root; i = i + num_thread)
	{
		/* arr[i] is zero means that 2*i+1 is not prime number */
		if(arr[i] == 0)
			continue;
		/* if 2*i+1 is prime_number, make multiple of prime number's index zero */
		for(j = i+(2*i + 1); (j*2 + 1) <= end_value; j = j + (2*i + 1))
		{
			arr[j] = 0;
		}
	}

}


int main(int argc, char* argv[])
{
	int verbose    			  = -1;  // if verbose value is one, print prime number.
	unsigned long prime_count =  0;  

	unsigned long i;                 // value for iterator
	pthread_t *_thread;    
	
	int index;                       // option index 
	int opt;              

	if(argc == 1)
	{
		printf("please provide arguments\n");
		exit(1);
	}
	/* parsing arguments using getopt_long function. */
	struct option options[] = {
		{"num_thread",1, 0, 0},   // option 0
		{"start"     ,1, 0, 0},   // option 1
		{"end"       ,1, 0, 0},   // option 2 
		{"verbose"   ,0, 0, 0}    // option 3
	};

	while(1)
	{
		opt = getopt_long(argc, argv, "n:s:e:v", options, &index);
		
		if(opt == -1)
			break;
		switch(opt)
		{
			case 0:
				switch(index){
				case 0:    // --num_thread
					num_thread  = atoi(optarg);
					break;
				case 1:    // --start
					start_value = strtoul(optarg, NULL, 10);
					break;
				case 2:    // --end
					end_value   = strtoul(optarg, NULL, 10);
					break;
				case 3:    // --verbose
					verbose = 1;
					break;
				}
				break;
			case 'n':		// -n
				num_thread   = atoi(optarg);
				break;
			case 's':		// -s
				start_value  = strtoul(optarg, NULL, 10);
				break;
			case 'e':		// -e
				end_value    = strtoul(optarg, NULL, 10);
				break;
			case 'v':		// -v
				verbose      = 1;
				break;
		}
	}
	/* check argumens */
	if(num_thread < 1)
	{
		printf("please provide correct argument : \"thread\" should be bigger than 1\n");
	}
	if(start_value < 1)
	{
		printf("please provide correct argument : \"start\" should be bigger than 1\n");
		exit(1);
	}
	if(end_value < start_value)
	{
		printf("please provide correct argument : \"end\" should be bigger than \"start\"\n");
		exit(1);
	}

	/* make thread array */
	_thread      = (pthread_t *)malloc(sizeof(pthread_t) * num_thread);
	if(_thread == NULL){
		printf("can't malloc the array of thread\n");
		exit(1);
	}

	/* caculate size of array I need array that each index mean odd number(index*2 + 1) 
	   I don't need to malloc the array about even number */
	if(end_value % 2 == 0)
		array_size = end_value / 2;
	else
		array_size = end_value / 2 + 1;

	/* malloc array of char */
	arr          = (char *)malloc(sizeof(char) * array_size);
	if(arr == NULL)
	{
		free(_thread);
		printf("can't malloc the array of char\n");
		exit(1);
	}

	end_value_root = sqrt(end_value);

	/* initialize the array. If value of array index i is one, 2*i+1 is prime number. */
	// one is not prime number so init arr[0] zero.
	memset(arr, 1, sizeof(char)*array_size);
	arr[0] = 0;

	/* create thread for count and print prime numbers. */
	for(i=0; i < num_thread; i++){
		pthread_create(&_thread[i], NULL, check_multiple, NULL);	
	}
	/* wait thread is finish */
	for(i=0; i< num_thread; i++){
		pthread_join(_thread[i], NULL);
	}
	/* count and print prime */
	if(verbose == 1){
		if(start_value <= 2)
		{
			prime_count++;
			printf("2\n");
		}
		for(i = start_value/2 ; i < array_size; i++){
			if(arr[i] != 0){
				printf("%lu\n", 2*i + 1);
				prime_count++;
			}
		}
	}
	/* just count prime */
	else
	{
		if(start_value <= 2)
			prime_count++;
		for(i = start_value/2 ; i < array_size; i++){
			if(arr[i] != 0){
				prime_count++;
			}
		}
	}
	/* print Total prime count */
	printf("Total number of prime numbers between %lu and %lu is %lu\n",start_value, end_value, prime_count);

	/* free the array and thread array */
	free(arr);
	free(_thread);

	return 0;
}
