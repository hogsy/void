#ifndef VOID_COM_REGISTRY
#define VOID_COM_REGISTRY

/*
================================================
Void registry helper funcs
implicitly manipulates the HKEY_CURRENT_USER 
key only.
================================================
*/

namespace VoidReg {

//Check if key is present
bool DoesKeyExist(const char * keyname);

//Add item-val pair in the given key
bool AddKeyValuePair(const char *keyname,
					const char *itemname,
					const char *valuename);

//Add item-val pair in the given key
bool GetKeyValue(const char *keyname, 
				 const char *itemname, 
				 char *buffer,
				 int   bufflen);
}

#endif