#ifndef VOID_COM_KEYFIELDS
#define VOID_COM_KEYFIELDS

#include "Com_buffer.h"

/*
======================================================================================
To support creation of a new entity type
Derive class from CBaseEntityMaker
create an object of that class
======================================================================================
*/
enum KeyType
{
	KEY_NONE = 0,
	KEY_BYTE,
	KEY_SHORT,
	KEY_INT,
	KEY_VECTOR,
	KEY_STRING,
	KEY_ANGLE,
	KEY_FLOAT
};

struct KeyField
{
	//======================================================================================
	//Constructor/Destructor/Assignment
	KeyField(const char *ikey, int ioffset, KeyType itype) : 
			offset(ioffset), type(itype)
	{
		name = new char[strlen(ikey)+1];
		strcpy(name,ikey);
	}

	KeyField(const KeyField &key) : offset(key.offset), type(key.type)
	{
		name = new char[strlen(key.name)+1];
		strcpy(name,key.name);
	}

	~KeyField(){ if(name) delete [] name; }

	KeyField & operator = (const KeyField &key)
	{
		if(&key == this)
			return *this;

		delete [] name;
		name = new char[strlen(key.name)+1];
		strcpy(name,key.name);
		offset = key.offset;
		type= key.type;
		return *this;
	}

	//======================================================================================
		
	//byte offset in Source struct to corret field
	int		 offset;
	KeyType  type;
	char *   name;

	//Write Field value to given dest
	static void ReadField(const KeyField &field, CBuffer &buf, byte * dest)
	{
		switch(field.type)
		{
		case KEY_BYTE:
			break;
		case KEY_SHORT:
			break;
		case KEY_INT:
			break;
		case KEY_VECTOR:
			break;
		case KEY_STRING:
			{
				void * p = (void *)(dest + field.offset);
				strcpy((char*)p, buf.ReadString());
				break;
			}
		case KEY_ANGLE:
			break;
		case KEY_FLOAT:
			break;
		}
	}

};

#endif