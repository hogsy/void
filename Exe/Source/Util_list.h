#ifndef _UTIL_LIST_
#define _UTIL_LIST_

template <class T> class CPtrList
{
public:
	CPtrList()  { next = 0; item = 0; }
	~CPtrList() { if(item) delete item; item = 0;
			      if(next) delete next; next = 0;
				}
	T * item;
	CPtrList * next;
};


template <class T> class CPRefList
{
public:
	CPRefList()  { next = 0; item = 0; }
	~CPRefList() { item = 0;
				   if(next) delete next; next = 0;
				}

	T * item;
	CPRefList * next;
};



template <class T>class CList
{
private:
	struct QItem
	{
		QItem *next;
		T	*  data;
		QItem(T *d)
		{ 
			next= 0; 
			data = d;
		}
	};
	QItem *head;	//the oldest item
	QItem *tail;	//the newest item
	int max_size;
	int	cur_size;

public:
	
	CList(T *first,int msize=32)
	{
		tail = new QItem(0);
		head = tail;
		cur_size = 0;
		max_size = msize;

		//add first item
		Add(first);

	}

	CList(int msize=32)
	{
		tail = new QItem(0);
		head = tail;
		cur_size = 0;
		max_size = msize;
	}

	~CList()
	{
		while(Pop()!=NULL);
		head  =0;
		delete tail;
	}


	//Add item to list and advance, current pointer
	bool Add(T *item)
	{
		if(cur_size < max_size)
		{
			tail->next = new QItem(item);
			tail = tail->next;
			cur_size++;
			return true;
		}
		return false;
	}

	//give the previous item in the list
	const T * GetPrevItem()
	{
		QItem *temp = head;
		while(temp->next != tail)
			temp = temp->next;
		return temp->data;
	}

	//give the next item in the list
	const T* GetNextItem()
	{
		if(tail->next)
			return tail->next->data;
		return 0;
	}

	
	//give oldest item and delete it move tail back one item
	T * Pop()	
	{
		//last item, no more to give
		if(head != tail)
		{
			T	*out = tail->data;

			QItem *t1 = head;
			QItem *t2 = tail;

			while(t1->next != t2)
				t1 = t1->next;
			
			delete t2;
			tail = t1;
			cur_size--;					
			return out;
		}
		return 0;
	}

	//starting from the oldest

	void PrintAll()
	{
		QItem *temp= head->next;
		int i=0;
		
		while(temp && temp->data)
		{
//g_pCons->dprintf("L Item :%d  - Data  %s\n",i,*temp->data);
			i++;
			temp=temp->next;
		}
	}

};


#endif