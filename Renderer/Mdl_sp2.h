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
	void LoadModel(const char *file);	// load a md2

private:

	struct sp2_frame_t
	{
		int width, height;
		int origin_x, origin_y;
		char skin_name[64];
	};

	void LoadFail(void);	// default model

	// frame data
	sp2_frame_t *frames;

	// the glcommand list
	void *cmds;
};


#endif
