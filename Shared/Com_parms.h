#ifndef VOID_COM_PARMS
#define VOID_COM_PARMS

/*
This class is used to handle parameters 
*/

#define COM_DEFAULTPARMS	5
#define COM_MAXARGLEN		80

class CParms
{
public:

	//Constructor, accepts max num of arguments as parameter
	CParms(int imaxargs=COM_DEFAULTPARMS) : m_maxargs(imaxargs), 
											m_numargs(0)
	{ 
		m_szargv = new char * [imaxargs];
		for(int i=0;i<imaxargs;i++)
			m_szargv[i] = new char[COM_MAXARGLEN+1];
	}
	
	~CParms() 
	{	
		for(int i=0;i<m_maxargs;i++)
		{
			delete [] m_szargv[i];
			m_szargv[i] =0;
		}
		delete [] m_szargv;
		m_szargv =0;
		m_numargs = 0;
	}

	void Reset()
	{
		for(int i=0;i<m_maxargs;i++)
			memset(m_szargv[i],0,COM_MAXARGLEN);
		m_numargs = 0;
	}

	void Set(const char * const * iargv, int inumargs)
	{
		if(m_numargs || m_szargv)
			Reset();

		for(int i=0;i<inumargs;i++)
		{	strcpy(m_szargv[i],iargv[i]);
		}
		m_numargs = inumargs;
	}

	void Set(const char *string)
	{
		if(m_numargs || m_szargv)
			Reset();

		const char *p = string;
		const char *last = string;
		bool  inquotes=false;
		int	  arglen=0;

		//stuff enclosed in " " is treated as 1 arg
		while((*p || *p=='\0') && m_numargs < m_maxargs)
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
					|| (*p == '\0')) 
				{
					strncpy(m_szargv[m_numargs],last,arglen);
					m_szargv[m_numargs][arglen+1] = '\0';
					
					last = p;
					last++;
					arglen =0;
					m_numargs++;

					if(*p=='\0')
						break;
				}
				else if(arglen < CON_MAXARGSIZE)
					arglen++;
			}
			p++;
		}
	}
	
	int	  GetNumArgs() const { return m_numargs;}
	const char * const * GetArgs() const { return m_szargv; }
	
	const char * operator [] (int i) const
	{
		if(i<=m_numargs)
			return m_szargv[i];
		return 0;
	}

private:

	int		m_maxargs;
	int		m_numargs;
	char ** m_szargv;
};


#endif