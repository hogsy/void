#ifndef _V_WAVEFILES
#define _V_WAVEFILES

#define RES_CACHE_EMPTY		0
#define RES_CACHE_TEMP		1
#define RES_CACHE_LEVEL		2
#define RES_CACHE_GAME		3

#define MAX_SOUNDS	256


class 	CWavefile
{
public:
	CWavefile(){ filename=0; fpBuf=data=0;};
//	~CWavefile();

	operator bool(void) const { return fpBuf != 0; }
	unsigned long  samplesPerSecond;
	unsigned long  length;
	unsigned short blockAlign;	
	unsigned short numChannels;
	unsigned char* fpBuf;
	unsigned char* data;		// data stream, variable-sized
	
	char *filename;
	int	  cachetype;
};

class CWavemanager
{
public:
	CWavemanager();
	~CWavemanager();

	int		Register(char *filename,int cache_type);
	bool	Unregister(int index);
	
	CWavefile	* WaveIndex(int index);
	CWavefile	* WaveIndex(char *filename);
	
	int			GetIndex(char *filename);

private:
	CWavefile 	wavepool[MAX_SOUNDS];
	int			curwaves;
};


#endif