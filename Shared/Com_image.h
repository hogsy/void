#ifndef _COMMON_IMAGE_STUFF_
#define _COMMON_IMAGE_STUFF_

class CImageData
{
public:
	CImageData() { height = width = 0; data = 0; }
	~CImageData() { if(data) delete [] data; }

	void Reset() { height = width = 0; if(data) delete [] data; }

	int height;
	int width;
	unsigned char * data;
};


bool WritePCXPalette(const char *filename, CImageData *dest);

bool WriteTGAFromWal(const char *filename, CImageData *src);
bool WriteTGAFromQ1PCX(const char *filename, CImageData *src);
bool WriteTGAFromQ2PCX(const char *filename, CImageData *src);
bool WriteTGAFromDoomTex(const char *filename, CImageData *src);
bool WriteTGAFromPCX(const char *filename, CImageData *src);

#endif