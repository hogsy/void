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

	CParms(int len);	
	CParms(const CParms &parms);
	~CParms();

	CParms & operator = (const char * istr);

	//Return the number of tokens
	int NumTokens(char delim =' ') const;

	//Return token number x as a string Token 0 is the first one
	const char * StringTok(int num, char delim=' ') const;

	//Return token number x as an integer Token 0 is the first one
	int IntTok(int num, char delim =' ') const;

	//Return token number x as a float Token 0 is the first one
	float FloatTok(int num, char delim =' ') const;

	const char * String() const;
	int Length() const;

private:
	char * string;
	int	 length;
	static char szParmBuffer[1024];
};

#endif