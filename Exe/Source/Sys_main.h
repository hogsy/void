#ifndef VOID_SYS_MAIN
#define VOID_SYS_MAIN

#include "Sys_hdr.h"
#include "Sys_time.h"

//========================================================================================
#define MAINWINDOWCLASS "Void"
#define MAINWINDOWTITLE	"Void"


class CVoid
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
	
		CTime		* g_pTime;
		CFileSystem * g_pFileSystem;
		
		//Windows	
		bool	RegisterWindow();				//Register Window
		void	ParseCmdLine(LPSTR lpCmdLine);	//Parse Cmd Line

		//Write configuration file
		void    WriteConfig(char *config);
};

extern CVoid * g_pVoid;

#endif