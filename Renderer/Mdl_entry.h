#ifndef MDL_ENTRY_H
#define MDL_ENTRY_H


const char MODEL_DEFAULT_NAME [] = "Default";


class CModelCacheEntry
{
public:
	CModelCacheEntry();
	virtual ~CModelCacheEntry();

	virtual void LoadModel(I_FileReader * pFile, const char * szFileName)=0;
	virtual void Draw(int skin, int fframe, int cframe, float frac)=0;
	virtual void LoadFail(void)=0;	// default model
	
	void LoadSkins();
	void UnLoadSkins();
	
	void AddRef()  { mRefCount++;	}
	int  Release() { return --mRefCount; }
	
	const char * GetFileName() const 
	{ return modelfile; 
	}

	bool IsFile(const char *file) const	
	{	
		if(!modelfile) 
			return 0;
		return (_stricmp(file, modelfile)==0); 	
	}
	int  GetNumSkins()  const { return num_skins;	}
	int	 GetNumFrames() const { return num_frames;	}

protected:
	
	// skin info
	
	int num_skins;
	int mShaderBin;
//	int	skin_bin;		// rasterizer texture names
	char **skin_names;	// text texture names

	// filename of model
	char *modelfile;

	// frame data
	int		 num_frames;

private:
	int mRefCount;
};


#endif

