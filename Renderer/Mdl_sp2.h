#ifndef MDL_SP2_H
#define MDL_SP2_H

#include "I_clientRenderer.h"
#include "Mdl_entry.h"


class CModelSp2 : public CModelCacheEntry
{
public:
	CModelSp2();
	~CModelSp2();

	void Draw(int skin, int fframe, int cframe, float frac);
	
	// load am sp2
	void LoadModel(I_FileReader * pFile, const char * szFileName);	

	// default model
	void LoadFail(void);	

private:

	struct sp2_frame_t
	{
		int width, height;
		int origin_x, origin_y;
		char skin_name[64];
	};

	// frame data
	sp2_frame_t *frames;
};


#endif
