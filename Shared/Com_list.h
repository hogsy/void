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


#endif