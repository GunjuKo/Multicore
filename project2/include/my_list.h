#ifndef __PAIR_LIST
#define __PAIR_LIST
/* This header file include struct pair and struct thread 
 and class myList. myList is implemented using template.
 */

/* myList class is singly linked list. */
template <typename T>
class myList{

	private:
		T   *head;      // head's next indicate head of list.
	public:
		myList();
		~myList();
		bool push_front(T *node);             // insert node in front of head
		bool erase(T *node);                  // erase node from list
		void free_all_list_elem();            // free all node in list 
		void reset();
		T*   pop_head();                      // return node which located in head of list
		T*   pop_back();                      // return node which located in tail of list
};

/* struct pair */
typedef struct _pair{
	long int A;
	long int B;
	unsigned long version;
	struct _pair *next;
} v_pair;

/* struct thread. 
   struct _thread include list of pair. 
 */
typedef struct _thread{
	int thread_ID;                             // tid of thread
	unsigned long execution_order;             
	unsigned long count_update;                // count how many thread is updated. this value is used for calculate fairness
	struct _thread *next;

	myList<v_pair> pair_list;                  // history of pair 
} thread;

template <typename T>
myList<T>::myList()
{
	head = (T *)malloc(sizeof(T));
	if(head == NULL)
		exit(1);
	/* initialize the variable */
	head->next = NULL;
}

template<typename T>
myList<T>::~myList()
{
	free_all_list_elem();
	free(head);
}

/* push node into list of head */
template<typename T>
bool myList<T>::push_front(T *node)
{
	if(node == NULL)
		return false;
	node->next = head->next;
	head->next = node; 
	return true;
}
/* return head->next */
template<typename T>
T* myList<T>::pop_head()
{
	if(head != NULL){
		return head->next;
	}
	else{
		return NULL;
	}
}
/* return tail of list */
template<typename T>
T* myList<T>::pop_back()
{
	if(head != NULL){
		T* p   = head->next;
		T* pre = NULL;
		while(p != NULL)
		{
			pre = p;
			p   = p->next;
		}
		return pre;
	}
	else
		return NULL;
}
/* erase tail of list and return */
template<typename T>
bool myList<T>::erase(T *node)
{
	if(head != NULL)
	{
		T *p   = head->next;
		T *pre = head;
		/* find the node */
		while(p != NULL)
		{
			if(node == p)
				break;
			else
			{
				pre  = p;
				p    = p->next;
			}
		}
		/* if find node, erase node from list and return true */
		if(p != NULL)
		{
			pre->next = p->next;
			p->next   = NULL;
			return true;
		} 
		// if can't find node, return false.
		else
			return false;
	}
	else
		return false;
}
/* free all node in the list and reset the list*/
template <typename T>
void myList<T>::free_all_list_elem()
{
	if(head != NULL)
	{
		T* p   = pop_head();
		T* pre = NULL;
		while(p != NULL)
		{
			pre = p;
			p   = p->next;
			if(pre != NULL){
				free(pre);
			}
		}
	}
	reset();
}

template<typename T>
void myList<T>::reset()
{
	if(head != NULL){
		head->next = NULL;
	}
}

#endif
