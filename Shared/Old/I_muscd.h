#ifndef INC_IMUSCD_
#define	INC_IMUSCD_


interface I_MusCD : IUnknown
{
	virtual bool pascal CDInit(DPRINT print,HWND	hwnd)=0;
	virtual bool pascal CDShutdown() = 0;
	
	virtual bool pascal CDPlay(int track) = 0;
	virtual bool pascal CDResume() = 0;
	virtual bool pascal CDStop() = 0;
	virtual bool pascal CDPause() = 0;
	virtual bool pascal CDEject() = 0;
	virtual bool pascal CDClose() = 0;
	
	
	virtual void pascal CDTrackListing() = 0;
};

extern "C" const IID IID_IMUSCD;
extern "C" const CLSID CLSID_VCDMusic;

#endif