#ifndef VOID_COM_KEYFIELDS
#define VOID_COM_KEYFIELDS

#include "Com_buffer.h"
#include "Com_vector.h"

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

	~KeyField()
	{ if(name) delete [] name; 
	}

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
	char   *  name; //[32];

	//Write Field value to given dest
	static void ReadField(const KeyField &field, CBuffer &buf, byte * dest)
	{
		void * p = (void *)(dest + field.offset);

		switch(field.type)
		{
		case KEY_BYTE:
			{
				*(byte *)(p)= buf.ReadByte();
				break;
			}
		case KEY_SHORT:
			{
				*(short *)(p) = (short)atoi(buf.ReadString());
				break;
			}
		case KEY_INT:
			{
				*(int *)(p) = atoi(buf.ReadString());
				break;
			}
		case KEY_FLOAT:
			{
				*(float *)(p) = atof(buf.ReadString());
				break;
			}
		case KEY_VECTOR:
			{
				vector_t vec;
				char * key = buf.ReadString();
				sscanf(key,"%f %f %f", &vec.x, &vec.y, &vec.z);
				(*((vector_t *)p)).x = vec.x;
				(*((vector_t *)p)).y = vec.y;
				(*((vector_t *)p)).z = vec.z;
				break;
			}
		case KEY_ANGLE:
			{
				float f = atof(buf.ReadString());
				(*((vector_t *)p)).x = 0;
				(*((vector_t *)p)).y = f;
				(*((vector_t *)p)).z = 0;
				break;
			}
		case KEY_STRING:
			{
				strcpy((char*)p, buf.ReadString());
				break;
			}
		}
	}

};

#endif