#ifndef VOID_UTIL_LIST
#define VOID_UTIL_LIST

/*
===========================================
Simple Template List with Public Members
-holds pointers to a given data type
-Destructor also deletes following items
===========================================
*/
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


/*
===========================================
Similar to above, but "items" are not deleted
===========================================
*/
template <class T> class CPRefList
{
public:
	CPRefList() { next = 0; item = 0; }
	~CPRefList(){ item = 0;
				  if(next) delete next; next = 0; 
				}
	T * item;
	CPRefList * next;
};



/*
===========================================
StringList, 
-client code manually allocs strings
-string are deleted on destruction of list
===========================================
*/
class CStringList
{
public:
	CStringList()  {  next = 0; } 
	~CStringList() { if(next) delete next; next =0; }

	char		string[COM_MAXPATH];
	CStringList *next;

	//Sort the list
	void QuickSortStringList(CStringList * const list,const int numitems)
	{
		if(numitems < 2)
			return;

		CStringList *	sorted = new CStringList[numitems];		//dest array
		CStringList *	pivot = list;							//let the first one be the pivot
		CStringList *	iterator = list->next;					
		
		int maxindex = numitems-1;
		int left=0, right = maxindex; 
		
		//loop one less time since the first item is the pivot
		for(int i=0,comp=0;i<maxindex;i++)
		{
			comp = _stricmp(iterator->string,pivot->string);
			
			if(comp < 0)
			{
				strcpy(sorted[left].string,iterator->string);
				sorted[left].next = &sorted[(left+1)];
				left++;
			}
			else if(comp >= 0)
			{
				strcpy(sorted[right].string,iterator->string);
				sorted[(right-1)].next = &sorted[right];
				right--;
			}
			iterator = iterator->next;
		}
		
		//Copy the pivot point in the empty space
		strcpy(sorted[left].string, pivot->string);
		if(right == maxindex)
			sorted[left].next = 0;
		else
			sorted[left].next = &sorted[(left+1)];
			
		if(left > 1) 
			QuickSortStringList(sorted,left);								
		if((numitems - (right+1)) > 1) //starting from the one right after the pivot
			QuickSortStringList(&sorted[left+1],(numitems - (right+1)));	
					
		//List is sorted now copy it over
		iterator = list;
		for(i=0;i<numitems;i++)
		{
			strcpy(iterator->string,sorted[i].string);
			iterator=iterator->next;
			sorted[i].next = 0;			//get rid of the links so we dont have problems deleting the array
		}
		delete [] sorted;
	}
};


/*
===========================================
CList
Urgh, too ugly. Fix this
===========================================
*/
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
/*	void PrintAll()
	{
		QItem *temp= head->next;
		int i=0;
		
		while(temp && temp->data)
		{
g_pCons->dprintf("L Item :%d  - Data  %s\n",i,*temp->data);
			i++;
			temp=temp->next;
		}
	}
*/
};


#endif