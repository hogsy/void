#include "Com_buffer.h"
#include <memory.h>
#include <string.h>

/*============================================================================
============================================================================*/


/*
======================================
Constructor
======================================
*/

CSBuffer :: CSBuffer(int size):m_size(size)
{
	m_psz = new char[m_size+1];
	memset(m_psz,'\0',m_size+1);
	m_cursize = 0;
}


/*
======================================
Destructor
======================================
*/

CSBuffer :: ~CSBuffer()
{
	delete [] m_psz;
}


/*
======================================
Reset the Buffer
======================================
*/
void CSBuffer:: Reset()
{
	memset(m_psz,'\0',m_size+1);
	m_cursize = 0;
}

/*
======================================
Set the buffer to a string
======================================
*/
void CSBuffer:: Set(const char *string)
{
	if(!string)
		return;

	unsigned int len= strlen(string);
	memset(m_psz,'\0',m_size+1);
	if(len >= m_size)
		memcpy(m_psz,string,(m_size-1));
	else
		strcpy(m_psz,string);
	m_cursize=len;
}

/*
======================================
Copy the following to the end
======================================
*/
bool CSBuffer ::Append(const char &c)
{
	if(m_cursize < m_size)
	{
		m_psz[m_cursize]= c;
		m_cursize++;
		return true;
	}
	return false;
}

/*
======================================
add the following to the end
======================================
*/


bool CSBuffer::Append(const char *c)
{
	int len = strlen(c);
	if((len + m_cursize) < m_size)
	{
		strcat(m_psz,c);
		m_cursize += len;
		return true;
	}
	return false;
}

/*
======================================
Get rid of x amount of chars at the end
======================================
*/

void CSBuffer::Pop(int num)
{
	int diff= m_cursize-num;
	if(diff < 0)
		return;
	m_cursize=diff;
	memset(m_psz+m_cursize,'\0',num);
}



/*
======================================
Tokenize the passed string
======================================
*/

#define CON_MAXARGSIZE 80

int	CSBuffer::Parse(char ** szargv)
{
	char *p = m_psz;
	char *last = m_psz;
	bool	inquotes=false;
	int		numargs=0;
	int		arglen=0;

	//stuff enclosed in " " is treated as 1 arg
	while((*p || *p=='\0') && numargs < BMAX_ARGS)
	{
		//are we in quotes
		if(*p == '\"')
		{
			if(inquotes==false) 
				inquotes=true;
			else
				inquotes=false;
		}
		else
		{
			if(((*p == ' ') && 	!(inquotes)) 
				|| (*p == '\0')) // || (*p == '\0'))
			{
				memset(szargv[numargs],0,CON_MAXARGSIZE);
				strncpy(szargv[numargs],last,arglen);
				szargv[numargs][arglen+1] = '\0';
				
				last = p;
				last++;
				arglen =0;
				numargs++;

				if(*p=='\0')
					break;
			}
			else if(arglen < CON_MAXARGSIZE)
			{
				arglen++;
			}
		}
		p++;
		
	}
	return numargs;
}



/*============================================================================
============================================================================*/


/*
======================================
Buffer Copy Funcs
======================================
*/

char * BufCopy(char *to,			//dest
			   const char *from,	//from
			   int len)				//lenght to copy
{
	if(!from || !to)
		return 0;
	
	const char *f = from;
	char *t = to;
	int i = 0;

	while(i < len &&
		 (t && t != '\0') && 
		 (f && f != '\0'))
	{
		*t = *f;
		t++; f++;
		i++;
	}

	if(i) return to;
	return 0;

/*	if(to)
		delete [] to;
	to = new char[len];
	strcpy(to,from);
	return to;
*/
	
}

char * BufCopy(char *to,
			   const char *from)	//from
{
	if(!from)
		return 0;

	return BufCopy(to,from,99999);

/*	if(to)
		delete [] to;
	to = new char[strlen(from)+1];
	strcpy(to,from);
	return to;
*/
}


/*
======================================
Buffer Copy+Alloc Funcs
======================================
*/

char * BufAlloc(const char *from)	//from
{
	if(!from)
		return 0;
	char *to = new char[strlen(from)+1];
	BufCopy(to,from);
	return to;
}


/*
======================================
Buffer Parsing
======================================
*/

int	BufParse(const char *string, //in
			  char ** szargv)	 //out- arg list
{
	const char *p = string;
	const char *last = string;
	bool	inquotes=false;
	int		numargs=0;
	int		arglen=0;

	//stuff enclosed in " " is treated as 1 arg
	while((*p || *p=='\0') && numargs < BMAX_ARGS)
	{
		//are we in quotes
		if(*p == '\"')
		{
			if(inquotes==false) 
				inquotes=true;
			else
				inquotes=false;
		}
		else
		{
			if(((*p == ' ') && !(inquotes)) //space not in quotes
				|| (*p == '\0'))// || (*p == '\0'))
			{
				//memset(szargv[numargs],'\0',arglen+1);
				//memset(szargv[numargs],0,arglen+1);
				strncpy(szargv[numargs],last,arglen);
				szargv[numargs][arglen+1] = '\0';
				last = p;
				last++;
				arglen =0;
				numargs++;

				if(*p=='\0')
					break;
			}
			else if(arglen <CON_MAXARGSIZE)
			{
					arglen++;
			}
		}
		p++;
		
	}
	return numargs;
}


/*
=====================================
Get next token in the string
when the delimited is this
=====================================
*/

char * BufNextToken(char *string, char token)
{
	if(!string)
		return 0;

	char *p = string;
	while(*p && (*p!='\0') && (*p!=token))
		p++;
	if(*p ==token)
		return ++p;
	return 0;
}

/*
=====================================
Comparision functions
=====================================
*/

//compare the entire string
int    BufCmp(const char *s1, const char *s2)			
{
	return BufnCmp(s1,s2,99999);
}


//compare num chars of each string
int	   BufnCmp(const char *s1, const char *s2, int num)
{
	int		c1, c2;
	
	do
	{
		c1 = *s1++;
		c2 = *s2++;

		if (!num--)
			return 0;		// strings are equal until end point
		
		if (c1 != c2)
		{
			if (c1 >= 'a' && c1 <= 'z')
				c1 -= ('a' - 'A');
			if (c2 >= 'a' && c2 <= 'z')
				c2 -= ('a' - 'A');
			if (c1 != c2)
				return -1;		// strings not equal
		}
	} while (c1);
	
	return 0;		// strings are equal
}



/*
=====================================
return the value of a key in a string
=====================================
*/

bool BufParseKey(const char *infostring, const char *key, char *dest, int destlen)
{
	if(!infostring || !key || !destlen)
	{
		dest = 0;
		return false;;
	}

	int keylen = strlen(key);
	const char *s = infostring;
	const char *k = key;
	int match = 0;
	
	if(*s == '\\')
		s++;

	while(s)
	{
		if(*k == *s)
		{
			k++;
			match++;

			if(match == keylen)
			{
				s++;
				if(*s == '\\')
				{
					int i=0;
					s++;
					while(s && *s && 
						 (*s!='\\') && 
						 (*s!=' ') &&		//is this necessary ?
						 (i<destlen))
					{
						dest[i] = *s;
						i++;
						s++;
					}
					dest[i] = 0;
					if(i)
						return true;
					return false;
				}
			}
		}
		else if(match)
		{
			k=key;
			match = 0;
		}
		s++;
	}

	return false;
}


