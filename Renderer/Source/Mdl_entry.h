
#ifndef MDL_ENTRY_H
#define MDL_ENTRY_H

#include <string.h>
#include "3dmath.h"


class CModelCacheEntry
{
public:
	CModelCacheEntry();
	virtual ~CModelCacheEntry();


	virtual void LoadModel(const char *file)=0;
	virtual void Draw(int skin, int fframe, int cframe, float frac)=0;
	bool IsFile(const char *file) { return (strcmp(file, modelfile)==0); }

	void LoadSkins(void);
	void UnLoadSkins(void);

	int GetNumSkins(void)	{ return num_skins;		}
	int	GetNumFrames(void)	{ return num_frames;	}

	int  Release(void)	{ return --mRefCount; }
	void AddRef(void)	{ mRefCount++;	}

private:

	int mRefCount;

protected:
	// skin info
	int num_skins;
	int	skin_bin;		// rasterizer texture names
	char **skin_names;	// text texture names

	// filename of model
	char *modelfile;

	// frame data
	int		 num_frames;
};


#endif

