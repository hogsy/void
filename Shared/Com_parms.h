#ifndef VOID_COM_PARMS
#define VOID_COM_PARMS

#include "Com_defs.h"

/*
======================================
Util class which can parse a 
string for arguments
======================================
*/
class CParms
{
public:

	enum
	{
		INVALID_VALUE = -1
	};

	explicit CParms(int len);	
	CParms(const CParms &parms);
	CParms(const char * buf);
	~CParms();

	void Set(const char * buf);

	CParms & operator = (const char * istr);

	//Return the number of tokens
	int NumTokens(char delim =' ') const;

	//Return token number x as a string Token 0 is the first one
	char * StringTok(int num, char * outString, int stringlen,  char delim=' ') const;

	//Get pointer to string. 
	//This is unsafe because string contents might change is another command
	//is executed
	const char * UnsafeStringTok(int num, char delim=' ') const;

	//Return token number x as an integer Token 0 is the first one
	int IntTok(int num, char delim =' ') const;

	//Return token number x as a float Token 0 is the first one
	float FloatTok(int num, char delim =' ') const;

	const char * String() const;
	int Length() const;

private:
	char * string;
	int	 length;
	mutable int  numTokens;
	
	static char szParmBuffer[1024];
};

#endif