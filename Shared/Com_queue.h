#ifndef _UTIL_QUEUE_
#define _UTIL_QUEUE_

//the queue is the opposite of the list
//keeps only pointers to previous items

/*
#ifdef _DEBUG
#include <crtdbg.h>
#define MALLOC(size) _malloc_dbg(size, _NORMAL_BLOCK, __FILE__, __LINE__);
//#define FREE(void _free_dbg( void *userData, int blockType );
#else
#define MALLOC(size) malloc(size)
#endif

*/
template <class T>class CQueue
{
private:
	class QItem
	{
	public:
		QItem(QItem *p, const T *d, int len)
		{ 
			data = new T[len];
			memcpy(data,d,len);
			prev = p; 
		};
		
		QItem(){ prev = 0; data = 0;};
		
		~QItem() 
		{	
			if(data!=0) 
				delete [] data;
			data = 0;
			prev = 0;
		}

		QItem *prev;
		T	*  data;
	};
	
	QItem *latest;		//the newest item
	QItem *oldest;		//the oldest item
	QItem *current;		//used by GetPrev, Getnext etc
	
	int max_size;
	int	cur_size;

public:
	
	CQueue(int maxsize=32)
	{
		latest = new QItem();

		oldest = latest;
		current = 0;

		cur_size = 0;
		max_size = maxsize;
	}

	~CQueue()
	{
		while(Pop()!=NULL);
		delete latest;
		current = 0;
		oldest = 0;
	}


	bool Add(const T *item, int len)	//add one item to the tail
	{
		if(cur_size >= max_size)
			if(!Pop())			//something is very wrong
				return false;
		
		QItem *qitem = new QItem(latest,item,len);
		
		latest = qitem;							//move tail forward to new item
		current = latest;
		cur_size++;
		return true;
	}

	const T* GetPrev()
	{
		if(current == oldest)
			return current->data;

		if(!current)
			current = latest;
		T*	data = current->data;
		if(current->prev)
			current = current->prev;
		return data;
	}

	const T* GetNext()
	{
		//there is a next item
		if(!current)
			current = oldest;
		
		if(current != latest)
		{
			QItem *t1 = latest;
			
			while(t1->prev != current)
				t1 = t1->prev;
			current = t1;
		}
		return current->data;
	}

	//remove the oldest item
	T * Pop()			
	{
		//is not the last item
		if(latest != oldest) 
		{
			QItem *t1 = latest;
			current = latest;

			while(t1->prev && t1->prev != oldest)
			{
				t1 = t1->prev;
			}

			//delete oldest;
			delete oldest;
			t1->prev = 0;
			oldest = t1;
			cur_size--;	
			return t1->data;
		}
		return 0;
	}

	//Print out the queue
	//starting from the newest
/*	void PrintAll()
	{
		QItem *temp= latest;
		int i=cur_size;
		
		while(temp && temp->data)
		{
//g_pCons->dprintf("Queue Item %d : %s\n",i,temp->data);
			i--;
			temp=temp->prev;
		}
	}
*/
};

#endif

