#ifndef VOID_COM_PARMS
#define VOID_COM_PARMS

const int COM_INVALID_VALUE = -1;

/*
================================================
Very basic const char* wrapper for CVar
validation funcs etc
================================================
*/
class CStringVal
{
public:
	CStringVal(const char * pString) : szString(pString) {}
	~CStringVal() { szString = 0; }

	int IntVal() const 
	{	return atoi(szString);
	}

	bool BoolVal() const
	{	return IntVal() ? true : false;
	}

	float FloatVal() const
	{	return atof(szString);
	}

	const char * String() const { return szString; }

private:
	const char * szString;
};


/*
================================================
Util class which can parse a 
string for arguments
================================================
*/
class CParms
{
public:

	explicit CParms(int len);	
	CParms(const CParms &parms);
	CParms(const char * buf);
	~CParms();

	CParms & operator = (const char * istr);
	void Set(const char * buf);

	//Return token number x, Token 0 is the first one
	char* StringTok(int num, char * outString, int stringlen,  char delim=' ') const;
	int   IntTok(int num, char delim =' ') const;
	float FloatTok(int num, char delim =' ') const;

	//Return the number of tokens
	int NumTokens(char delim =' ') const;

	const char * String() const { return string; }
	int Length() const { return length; }

private:
	
	char *	string;
	int		length;
	mutable int numTokens;
	
	static char szParmBuffer[1024];
};

#endif