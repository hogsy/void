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

	CParms(int len) : length(len)
	{	string = new char[length];
	}
	
	CParms(const CParms &parms)
	{
		length = strlen(parms.string) + 1;
		string = new char[length];
		strcpy(const_cast<char *>(string),parms.string);
	}

	~CParms() 
	{ 	if(string) 	delete [] const_cast<char *>(string); 
	}

	CParms & operator = (const char * istr)
	{
		int len = strlen(istr) + 1;
		if(len > length)
		{
			delete [] const_cast<char *>(string);
			length = len;
			string = new char [length];
		}
		strcpy(const_cast<char *>(string),istr);
		return *this;
	}

	//Return the number of tokens
	int NumTokens(char delim =' ') const
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

	//Return token number. Token 0 is the first one
	const char * GetToken(int num, char delim=' ') const
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

	const char * string;
	int			 length;

private:
	static char szParmBuffer[1024];
};

char CParms::szParmBuffer[1024];

#endif