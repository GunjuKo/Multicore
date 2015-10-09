#ifndef __BAKERY_LOCK__
#define __BAKERY_LOCK__
/* This header file implement Bakery_lock class. 
   Bakery lock class provides first come first served for n threads */
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

class Bakery_lock{
	private:
		bool *flag;                       // flag[i] is setted true if thread i  interested in lock
		unsigned long *label;             // label is userd for compare which thread is come first
		unsigned long max;                // This value is used for give the label to thread  
		unsigned int  size;                
		unsigned long now;                // This value indicatse label which holding a lock
	public :
		Bakery_lock(unsigned int n);
		~Bakery_lock();
		void lock(unsigned int n);
		void unlock(unsigned int n);
};

/* initialize the variables */
Bakery_lock::Bakery_lock(unsigned int n)
{
	flag  = (bool *)malloc(sizeof(bool) * n);
	if(flag == NULL)
		exit(1);
	
	label = (unsigned long *)malloc(sizeof(unsigned long) * n); 
	if(label == NULL)
	{
		free(flag);
		exit(1);
	}
	for(int i=0; i<n; i++){
		flag[i]  = false;
		label[i] = 0; 
	}
	max  = 0;
	now  = 0;
	size = n;
}

Bakery_lock::~Bakery_lock()
{
	free(flag);
	free(label);
}

void Bakery_lock::lock(unsigned int i)
{
	flag[i]  = true;                             // thread i is interested in lock 
	label[i] = __sync_fetch_and_add(&max, 1);    // receive label
	
	for(int k=0; k < size; k++)
	{
		while( (k != i) && (flag[k] == true) && (label[k] < label[i]) )
		{
			/* There are too many thread waiting a lock. so thread i go to sleep */
			if(label[i] - now > 180)            
				usleep(8000);
			else if(label[i] - now > 100)
				usleep(6000);
			else if(label[i] - now > 50)
				usleep(3000);
			else if(label[i] - now > 30)
				usleep(2000);
			else if(label[i] - now > 10)
				usleep(1000);
		}
	}
	now = label[i];
}

void Bakery_lock::unlock(unsigned int i)
{
	flag[i] = false;     // thread i is not interested in lock anymore.
}

#endif
