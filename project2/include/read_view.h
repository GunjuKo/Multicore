#ifndef __READ_VIEW
#define __READ_VIEW
/* This header file is related to read_view.
   r_thread is used to make a read_view. */
#include <vector>
#include <my_list.h>

using namespace std;


typedef struct read_view_thread{
	int thread_ID;                       // thread's id
	unsigned long newest_version;        // this value is thread's newest version
} r_thread;

/* This fuction copy active thread list into read view */
void locally_copy_active_thread_list(myList<thread> &active_thread_list, vector<r_thread> &read_view)
{
	thread *t = active_thread_list.pop_head();
	while(t != NULL)
	{
		r_thread copy;
		copy.thread_ID      = t->thread_ID;	
		copy.newest_version = t->execution_order;
		/* insert copy into read_view */
		read_view.push_back(copy);
		t = t->next;
	}
}
/* this function search read view whether read view is include thread which thread'id is thread_id*/
unsigned long search_read_view(vector<r_thread> &read_view, int thread_id)
{
	vector<r_thread>::iterator i = read_view.begin();
	for( ; i != read_view.end(); i++)
	{
		/* if find the thread which thread's id is thread_id, return thread's newest version */
		if(i->thread_ID == thread_id){
			return i->newest_version;
		}	
	}
	return 0;
}

/* find the pair which version is smaller than execution_order */
v_pair find_correct_pair(myList<v_pair> &pair_list, unsigned long execution_order)
{
	v_pair *p = pair_list.pop_head();
	while(p != NULL)
	{
		if(p->version < execution_order)
		{
			v_pair correct_pair;
			correct_pair.A	   = p->A;
			correct_pair.B	   = p->B;
			correct_pair.version = p->version;
			return correct_pair;
		}
		p = p->next;
	}
}
#endif
