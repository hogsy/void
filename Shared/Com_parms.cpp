#include "Com_defs.h"
#include "Com_parms.h"

char CParms::szParmBuffer[1024];

/*
======================================
Constructor/Destructor
======================================
*/
CParms::CParms(int len) : length(len)
{	
	string = new char[length];
	memset(string,0,length);
	numTokens = 0;
}

CParms::CParms(const char * buf)
{
	length = 0;
	numTokens = 0;
	string = 0;
	Set(buf);
}
	
CParms::CParms(const CParms &parms)
{
	length = 0;
	numTokens = 0;
	string = 0;
	Set(parms.string);
}

CParms::~CParms() 
{ 	if(string) 	delete [] string; 
}

void CParms::Set(const char * buf)
{
	int len = strlen(buf) + 1;
	if(len > length)
	{
		if(string)
			delete [] string;
		length = len;
		string = new char [length];
	}
	strcpy(string,buf);
	numTokens = 0;
}

/*
======================================
Assigment
======================================
*/
CParms & CParms::operator = (const char * istr)
{
	Set(istr);
	return *this;
}

/*
======================================
Return the number of tokens
======================================
*/
int CParms::NumTokens(char delim) const
{
	if(!numTokens || delim != ' ')
	{
		int tokens = 1;
		bool intoken = true;
		bool leading = true;
		const char * s = string;

		while(*s && *s != '\0')
		{
			if(*s != delim)
			{
				leading = false;
				if(!intoken)
				{
					tokens ++;
					intoken = true;
				}
			}
			else if(!leading)
				intoken = false;
			s++;
		}
		numTokens = tokens;
		return tokens;
	}
	return numTokens;
}

/*
======================================
Return token number x as a string 
Token 0 is the first one
======================================
*/
char * CParms::StringTok(int num, char * outString, 
						 int stringlen,  char delim) const
{
	if(num < 0 || (num > NumTokens(delim)))
		return 0;

	bool found = false,
		 intoken = false;
	int  tok = 0;
	const char * s = string;
	char * p = outString;

	//Find the appropriate token
	while(*s && *s != '\0')
	{
		if(*s != delim)
		{
			if(!intoken)
			{
				if(tok == num)
				{
					found = true;
					break;
				}
				tok++;
				intoken = true;
			}
		}
		else
			intoken = false;
		s++;
	}

	if(found)
	{
		//copy it to the buffer
		int toklen = 0;
		if(*s == '"')
		{
			delim = '"';
			s++;
		}

		while(*s && *s!='\0' && toklen+1 < stringlen && *s != delim)
		{
			*p++ = *s;
			toklen ++;
			s++;
		}
		*p = 0;
		return outString;
	}
	return 0;
}

/*
======================================
Return token number x as an integer 
Token 0 is the first one
======================================
*/
int CParms::IntTok(int num, char delim) const
{
	const char * s = StringTok(num,szParmBuffer,1024,delim);
	if(!s)
		return COM_INVALID_VALUE;
	return ::atoi(s);
}

/*
======================================
Return token number x as a float 
Token 0 is the first one
======================================
*/
float CParms::FloatTok(int num, char delim) const
{
	const char * s = StringTok(num,szParmBuffer,1024,delim);
	if(!s)
		return COM_INVALID_VALUE;
	return ::atof(s);
}



