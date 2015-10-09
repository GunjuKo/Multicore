#include <bakery_lock.h>
#include <pthread.h>
#include <my_list.h>
#include <unistd.h>
#include <getopt.h>
#include <math.h>
#include <vector>
#include <read_view.h>
#include <math.h>
#include <string.h>

using namespace std;
    
#define CONSTANT 50

unsigned long global_count = 1;      // this value is for global execution order
unsigned int  thread_id    = 0;        
thread *all_thread;                  // array of all thread
myList<thread> active_thread_list;   // list of active thread
Bakery_lock *b_lock;                 // Bakery_lock 

int num_thread = 1;                  // the number of thread, default value is 1
int duration   = 0;                  // duration of execution
bool running   = true;               // after duration, set running false. so all_thread is terminated.
bool verify    = false;              // if verify set true, verify the invariant
void *UPDATE(void *data);
void *garbage_collector(void *data);
unsigned long find_target_version(unsigned long min_version);
bool verify_invariant(vector<r_thread>& read_view, unsigned long execution_order);

void *UPDATE(void *data)
{
	unsigned int tid = __sync_fetch_and_add(&thread_id, 1);
      	
	while(running){
		/* step 1 start */
		b_lock->lock(tid);
		unsigned long execution_order   = __sync_fetch_and_add(&global_count, 1);   // increments and get a global execution order
		all_thread[tid].execution_order = execution_order;
		active_thread_list.push_front(&all_thread[tid]);                            // insert thread into the active thread list
		vector<r_thread> read_view;
		/* creates read-view that has to capture a list of active threads */
		locally_copy_active_thread_list(active_thread_list, read_view);
		b_lock->unlock(tid);
		/* step1 finish */
		/* step2 start */
		/* select the random value */
		int random = rand() % num_thread;
		if(random == tid){
			random = (random + 1) % num_thread;
		}
		unsigned long search_version;   
		/* check whether random thread is active thread or not */
		unsigned long read_view_version = search_read_view(read_view, random);
		/* if random thread is in active thread list */
		if(read_view_version != 0)   
			search_version = read_view_version;
		/* if random thread is not in active thread list */
		else 
			search_version = execution_order;
		
		v_pair p;
		/* find the pair A and B that version is smaller than search_version */
		p = find_correct_pair((all_thread[random].pair_list), search_version);
		/* create the new pair and initialize it */
		v_pair *new_pair = (v_pair *)malloc(sizeof(v_pair));
		if(new_pair == NULL)
			exit(1);
		new_pair->A 		= rand() % CONSTANT;
  		new_pair->B 	    = CONSTANT - new_pair->A;
		new_pair->A         = new_pair->A + p.A;
		new_pair->B         = new_pair->B - p.A;
		new_pair->version   = execution_order;
		/* push new pair into list at front */
		all_thread[tid].pair_list.push_front(new_pair);
		all_thread[tid].count_update += 1;
		/* if veriry is true, verify the invariant */
		if(verify == true)
		{
			if(verify_invariant(read_view, execution_order) == true)
				printf("Verify!\n");
			else
				printf("Error : Verify invariant!\n");
		}
		/* step2 finish */
		/* step3 : remove thread(tid) from active_thread_list */
		b_lock->lock(tid);
		if(active_thread_list.erase(&all_thread[tid]) == false)
			printf("error!\n");
		b_lock->unlock(tid);
	} 
}
/* garbage collector function.
 memory leak can occur if duration is over while garbage collection is running 
 I want to wait until garbage collection is finish! but because of time over change my code
 */
void *garbage_collector(void *data)
{
	unsigned long delete_node = 0;
	while(running){
		while(running)
		{
			/* every 10 second garbage collector checks whether the number of pair */
			sleep(10);
			unsigned long number_of_node = global_count - delete_node;

			if(number_of_node > 4000000) // if memory allocation size is bigger than 120MB, free unused pair 
				break;
		}
		if(running == false){      
			// finish the GC
			break;
		}
		b_lock->lock(num_thread);
		thread *t    		          = active_thread_list.pop_back();     // pop the oldest active thread
		unsigned long min_version     = t->execution_order;                // copy version of the oldest active thread
		unsigned long target_version  = find_target_version(min_version);  // find the version that are not necessary anymore
		b_lock->unlock(num_thread);
		for(int i=0; i<num_thread; i++)
		{
			v_pair *h = all_thread[i].pair_list.pop_head();
			while(h != NULL)
			{
				/* erase all pairs which version is smaller than target_version */
				if(h->version < target_version)
				{
					v_pair *erase = h;
					h = h->next;
					if(erase != NULL){
						if(all_thread[i].pair_list.erase(erase) == true){
							delete_node++;
							free(erase);
						}
					}
				}
				else
				{
					h = h->next;
				}
			}
		}
	}
}
int main(int argc, char* argv[])
{
	int opt;
	int index;
	struct option options[] = {
		{"num_thread", 1, 0, 0},     // option 0
		{"duration"  , 1, 0, 0},     // option 1
		{"verify"    , 0, 0, 0}      // option 2
	};

	while(1)
	{
		opt = getopt_long(argc, argv, "n:d:v", options, &index);
		
		if(opt == -1)
			break;
		
		switch(opt){
			case 0:
				switch(index)
				{
					case 0:          // option1(num_thread)
						num_thread = atoi(optarg);
						break;
					case 1:          // option2(duration)
						duration   = atoi(optarg);
						break;
					case 2:
						verify     = true;
						break;
				}
				break;
			case 'n':
				num_thread = atoi(optarg);
				break;
			case 'd':
				duration   = atoi(optarg);
				break;
			case 'v':
				verify     = true;
				break;
		}
	}
	/* check the arguments */
	if(num_thread < 0 || duration < 0)
	{
		printf("please provide correct arguments\n");
		return 0;
	}
	/* create the array_of all thread */
	all_thread = new thread[num_thread];
	if(all_thread == NULL)
		return 0;
	
	pthread_t *_thread;
	_thread = (pthread_t *)malloc(sizeof(pthread_t) * num_thread);
	if(_thread == NULL){
		delete[] all_thread;
		return 0;
	}
	
	b_lock = new Bakery_lock(num_thread + 1);
	
	/* initialize the array_of all thread and create thread */
	for(int i=0; i<num_thread; i++)
	{
		all_thread[i].thread_ID    = i;
		all_thread[i].count_update = 0;

		v_pair *p = (v_pair *)malloc(sizeof(v_pair));
		p->A = rand() % CONSTANT;
		p->B = (CONSTANT - p->A);
		p->version = 0;
		all_thread[i].pair_list.push_front(p);
	} 

	for(int i=0; i<num_thread; i++)
		pthread_create(&_thread[i], NULL, UPDATE, NULL);

	pthread_t GC;
	pthread_create(&GC, NULL, garbage_collector, NULL);
	/* sleep the thread */
	if(duration > 0){
		sleep(duration);
		running = false;
	}
	for(int i=0; i<num_thread; i++)
		pthread_join(_thread[i], NULL);
	double throughput = 0;
	double fairness   = 0;

	if(duration > 0)
		throughput = global_count / (double)duration;
	/* caculate fairness */
	unsigned long sum		 = 0;
	unsigned long double_sum = 0;
	for(int i=0; i<num_thread; i++)
	{
		unsigned long count   = all_thread[i].count_update;
		sum 	   += count;
		double_sum += count * count;
	}

	fairness = (sum * sum) / (double)(double_sum * num_thread);

	printf("throughput:[%0.2f] fairness:[%0.2f]\n",throughput,fairness);
	
	free(_thread);                  // free pthread_t
	delete b_lock;                  // delete the lock
	delete []all_thread;            // delete structure of all thread
	active_thread_list.reset();     
	
	return 0;
}
/* suppose all threads are active thread. find version that are not used anymore */
unsigned long find_target_version(unsigned long min_version)
{
	unsigned long target_version = min_version;
	for(int i=0; i<num_thread; i++)
	{
		v_pair *h = all_thread[i].pair_list.pop_head();
		while(h != NULL)
		{
			if(h->version > min_version)
				h = h->next;
			else
				break;
		}
		if(h != NULL && h->next != NULL)
			h = h->next;
		if(h != NULL && target_version > h->version)
			target_version = h->version;
	}
	return target_version;
}
/* verify invariant */
bool verify_invariant(vector<r_thread>& read_view, unsigned long execution_order)
{
	long int sum = 0;
	/* index is used whether thread i's value is added */
	bool *index = (bool *)malloc(sizeof(bool) * num_thread);
	if(index == NULL)
		exit(1);
	/* set all index is false */
	memset(index, 0, sizeof(bool) * num_thread);
	
	/* if thread is in read_view, find correct version based on read view */
	vector<r_thread>::iterator i = read_view.begin();
	for(; i != read_view.end(); i++)
	{
		int thread_id         = i -> thread_ID;
		unsigned long version = i -> newest_version;
		v_pair p 			  = find_correct_pair(all_thread[thread_id].pair_list, version);
		
		sum = sum + p.A + p.B;
		index[thread_id] = true;
	}
	
	for(int i=0; i<num_thread; i++)
	{
		/* if index[i] is false, that means thread i is not in read_view */
		if(index[i] == false)    
		{
			v_pair p = find_correct_pair(all_thread[i].pair_list, execution_order);
			sum      = sum + p.A + p.B ;
			index[i] = true;
		}
	}
	free(index);
	if(sum == CONSTANT * num_thread)
		return true;
	else
		return false;
}
