#ifndef VOID_COM_RESOURCE_MANAGER
#define VOID_COM_RESOURCE_MANAGER

struct I_FileReader;

/*
================================================
All resources need to implement this interface;
Dont' have to inherit from it. I just need the 
implementation and a friend declarations.
See Snd_wave.h for an example.
================================================
*/
class CResource
{
	virtual bool Load(const char * szFileName, I_FileReader * pFile)=0;
	virtual void Unload()=0;
	virtual bool IsEmpty() const=0;

	int	   m_refs;
	char * m_filename;
};

/*
======================================
Resource manager ensures that only one 
copy of a given resource is ever loaded
======================================
*/
template <class TRes> 
class CResManager
{
public:

	/*
	================================================
	Constructor/Destructor
	================================================
	*/
	CResManager(int maxResources, I_FileReader * pFileReader) : 
				m_maxItems(maxResources), m_pFileReader(pFileReader)
	{
		m_resCache = new TRes[m_maxItems];
	}

	~CResManager()
	{
		//Print diagnostic warnings
		for(int i=0;i<m_maxItems;i++)
		{
			if(!m_resCache[i].IsEmpty())
			{
				if(m_resCache[i].m_refs > 0)
				{
					ComPrintf("Warning : %s still has %d references\n", 
							m_resCache[i].m_filename, m_resCache[i].m_refs);
				}
				m_resCache[i].Unload();
			}
		}
		delete [] m_resCache;

		if(m_pFileReader)
			m_pFileReader->Destroy();
	}

	/*
	================================================
	Get a pointer to a resource. 
	Will only load if doesn't already exist
	Will return pointed to existing data otherwise
	Will return 0 if failed.
	================================================
	*/
	TRes * Load(const char * szFileName)
	{
		//first check for duplicates in files which are in use
		int freeIndex = -1;
		for(int i=0; i<m_maxItems; i++)
		{
			if(m_resCache[i].IsEmpty())
			{
				if(freeIndex == -1)
					freeIndex = i;
			}
			else if(_stricmp(m_resCache[i].m_filename, szFileName) == 0)
			{
				m_resCache[i].m_refs ++;
				return &m_resCache[i];
			}
		}

		if(freeIndex == -1)
		{
			ComPrintf("CWaveManager::Create: No space to load %s\n", szFileName);
			return 0;
		}

		//Didnt find a matching file, load it now
		if(m_resCache[freeIndex].Load(szFileName, m_pFileReader))
		{
			m_resCache[freeIndex].m_refs ++;
			return &m_resCache[freeIndex];
		}
		return 0;
	}

	/*
	================================================
	Will Release a resource. 
	Auto delete if no more references
	================================================
	*/
	int Release(TRes * pRes)
	{
		if(pRes)
		{
			pRes->m_refs --;
			if(pRes->m_refs == 0)
				pRes->Unload();
			return pRes->m_refs;
		}
		return 0;
	}

private:

	int		m_maxItems;
	TRes *  m_resCache;
	
	I_FileReader * m_pFileReader;

};


#endif