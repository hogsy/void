#ifndef MDL_ENTRY_H
#define MDL_ENTRY_H

class CModelCacheEntry
{
public:
	CModelCacheEntry();
	virtual ~CModelCacheEntry();

	virtual void LoadModel(const char *file)=0;
	virtual void Draw(int skin, int fframe, int cframe, float frac)=0;
	
	void LoadSkins();
	void UnLoadSkins();
	int  Release()	{ return --mRefCount; }
	void AddRef()	{ mRefCount++;	}

	bool IsFile(const char *file) const	{	return (strcmp(file, modelfile)==0); 	}
	int  GetNumSkins() const	 { return num_skins;	}
	int	 GetNumFrames() const { return num_frames;	}

protected:
	// skin info
	int num_skins;
	int mShaderBin;
	int	skin_bin;		// rasterizer texture names
	char **skin_names;	// text texture names

	// filename of model
	char *modelfile;

	// frame data
	int		 num_frames;

private:
	int mRefCount;
};


#endif

