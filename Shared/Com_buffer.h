
#ifndef _UTIL_CBUFFER
#define _UTIL_CBUFFER

#define BMAX_ARGS	5

/*
=======================================================================
Static Buffer
allocates itself upon init
=======================================================================
*/

class CSBuffer
{
public:
	CSBuffer(int size=256);
	~CSBuffer();
	bool Append(const char &c);
	bool Append(const char *c);
	
	void Pop(int num);				//remove number of chars
	void Reset();					//reset contents
	void Set(const char *string);	//set to string

	//return no. args and pointer to parsed string ?
	int	Parse(char ** szargv);

	const char * GetString() {if(m_psz)return m_psz; return 0;};
	const int GetSize()		 {return m_cursize; };
	
private:
	unsigned int   m_size;
	unsigned int   m_cursize;
	char *m_psz;
};

//========================================================================================
//========================================================================================


void   GetArg(const int numarg,
			  const char ** szarg,
			  int &val);	  

void   GetArg(const int numarg,
			  const char ** szarg,
			  bool &val);	  

void   GetArg(const int numarg,
			  const char ** szarg,
			  float &val);

//========================================================================================
//========================================================================================


char * BufCopy(char *to,			//from
			   const char *from,	//dest
			   int len);			//lenght to copy
char * BufCopy(char *to,
			   const char *from);	//from

char * BufAlloc(const char *from,	//from
 			    int len);			//lenght to copy
char * BufAlloc(const char *from);	//from

int	   BufParse(const char *string,  //in
			    char ** szargv);	 //out- arg list

char * BufNextToken(char *string, const char *token);	//tokenize the string, using this token
char * BufNextToken(char *string, char token);

int    BufCmp(const char *s1, const char *s2);			//compare the entire string
int	   BufnCmp(const char *s1, const char *s2, int num);//compare num chars of each string


//Infostrings
bool BufParseKey(const char *infostring, const char *key, char *dest, int destlen);

#endif