#include "Com_parms.h"

char CParms::szParmBuffer[1024];

/*
======================================
Constructor/Destructor
======================================
*/
CParms::CParms(int len) : length(len)
{	string = new char[length];
}
	
CParms::CParms(const CParms &parms)
{
	length = strlen(parms.string) + 1;
	string = new char[length];
	strcpy(string,parms.string);
}

CParms::~CParms() 
{ 	if(string) 	delete [] string; 
}

/*
======================================
Assigment
======================================
*/
CParms & CParms::operator = (const char * istr)
{
	int len = strlen(istr) + 1;
	if(len > length)
	{
		delete [] string;
		length = len;
		string = new char [length];
	}
	strcpy(string,istr);
	return *this;
}

/*
======================================
Return the number of tokens
======================================
*/
int CParms::NumTokens(char delim) const
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
	return tokens;
}

/*
======================================
Return token number x as a string 
Token 0 is the first one
======================================
*/
const char * CParms::StringTok(int num, char delim) const
{
	const char * s = string;
	bool found = false;
	bool intoken = false;
	int tok = 0;

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

	if(!found)
		return 0;
	
	//copy it to the buffer
	int toklen = 0;
	while(*s && *s!='\0' && *s != delim)
	{
		szParmBuffer[toklen] = *s;
		toklen ++;
		s++;
	}
	szParmBuffer[toklen] = 0;
	return szParmBuffer;
}

/*
======================================
Return token number x as an integer 
Token 0 is the first one
======================================
*/
int CParms::IntTok(int num, char delim) const
{
	const char * s = StringTok(num,delim);
	if(!s)
		return INVALID_VALUE;
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
	const char * s = StringTok(num,delim);
	if(!s)
		return INVALID_VALUE;
	return ::atof(s);
}

/*
======================================
Access funcs
======================================
*/
const char * CParms::String() const { return string; }
int CParms::Length() const { return length; }

