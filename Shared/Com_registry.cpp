#include "Com_defs.h"
#include "Com_registry.h"


namespace VoidReg {


bool DoesKeyExist(const char * keyname)
{
	HKEY key;
	if(::RegOpenKeyEx(HKEY_CURRENT_USER, keyname, 0, KEY_READ,  &key) == ERROR_SUCCESS)
	{
		::RegCloseKey(key);
		return true;
	}
	return false;
}


bool GetKeyValue(const char *keyname,  const char *itemname, char *buffer, int bufflen)
{
	HKEY key;
	
	if(::RegOpenKeyEx(HKEY_CURRENT_USER, keyname, 0, KEY_READ,  &key) != ERROR_SUCCESS)
		return false;

	ulong type = 0;
	ulong len = bufflen;

	if(::RegQueryValueEx(key, itemname, 0, &type, (byte*)buffer, &len)  != ERROR_SUCCESS)
	{
		::RegCloseKey(key);
		return false;
	}
	::RegCloseKey(key);
	return true;
}


bool AddKeyValuePair(const char *keyname,
					const char *itemname,
					const char *data)
{
	HKEY key;
	DWORD op;
	
//	if(::RegOpenKeyEx(HKEY_CURRENT_USER, keyname, 0, KEY_READ,  &key) != ERROR_SUCCESS)
//		return false;

	if(::RegCreateKeyEx(HKEY_CURRENT_USER,keyname,0,"Void", REG_OPTION_NON_VOLATILE,
						KEY_ALL_ACCESS,0,&key,&op) != ERROR_SUCCESS)
						return false;
/*
  LPTSTR lpClass,           // address of class string
  DWORD dwOptions,          // special options flag
  REGSAM samDesired,        // desired security access
  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                            // address of key security structure
  PHKEY phkResult,          // address of buffer for opened handle
  LPDWORD lpdwDisposition   // address of disposition value buffer
);
 */


	if(::RegSetValueEx(key,itemname,0,REG_SZ,(const byte*)data,strlen(data)) != ERROR_SUCCESS)
	{
		::RegCloseKey(key);
		return false;
	}
	::RegCloseKey(key);
	return true;
}

}



