#ifndef VOID_SYS_MAIN
#define VOID_SYS_MAIN

#include "Sys_hdr.h"
#include "Sys_time.h"
#include "I_renderer.h"

//========================================================================================
#define VOID_MAINWINDOWCLASS "Void"
#define VOID_MAINWINDOWTITLE "Void"

#define VOID_DEFAULTGAMEDIR	"Game"

class CVoid : public I_CmdHandler
{
	public:
		
		CVoid(HINSTANCE hInstance, 
			  HINSTANCE hPrevInstance, 
			  LPSTR lpCmdLine);					//Constructor
		~CVoid();								//Destructor
		
		bool Init();							//Init subsystems
		bool Shutdown();						//Shutdown everything

		void RunFrame();						//Game Loop

		bool LoadWorld(char *worldname);
		bool UnloadWorld();

		bool InitServer(char *map);				//Init game
		bool ShutdownServer();

		void Error(char *error, ...);

		//Application Events
		void Move(int x, int y);
		void Resize( bool focus, int x, int y, int w, int h);
		void Activate(bool focus);
		void OnFocus();
		void LostFocus();

	private:
	
		CTime		* g_pTime;			//Timer Class
		CFileSystem * g_pFileSystem;	//FileSystem

		RenderInfo_t * g_pRinfo;		//Current Renderering info
		VoidExport_t * g_pExport;		//Exported struct
		
		//Windows	
		bool RegisterWindow();				//Register Window
		void ParseCmdLine(LPSTR lpCmdLine);	//Parse Cmd Line

		//Write configuration file
		void WriteConfig(char *config);

		void CFuncQuit(int argc, char** argv);		//quit game
		void CFuncMap(int argc, char** argv);		//start local server with map + connect to it

		void HandleCommand(HCMD cmdId, int numArgs, char ** szArgs);
};

extern CVoid * g_pVoid;

#endif