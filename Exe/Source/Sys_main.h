#ifndef VOID_SYS_MAIN
#define VOID_SYS_MAIN

#include "Sys_hdr.h"

class CVoid
{
	public:
		
		CVoid(HINSTANCE hInstance, 
			  HINSTANCE hPrevInstance, 
			  LPSTR lpCmdLine, 
			  int nCmdShow);					//Constructor
		~CVoid();								//Destructor
		
		bool Init();							//Init subsystems
		bool Shutdown();						//Shutdown everything

		void RunFrame();						//Game Loop

		bool LoadWorld(char *worldname);
		bool UnloadWorld();

		bool InitServer(char *map);				//Init game
		bool ShutdownServer();

		void Error(char *error, ...);

	private:
	
		HINSTANCE hRenderer;					//Handle to Renderer
		
		//Windows	
		bool	RegisterWindow();				//Register Window
		void	ParseCmdLine(LPSTR lpCmdLine);	//Parse Cmd Line

		//Write configuration file
		void    WriteConfig(char *config);
};

extern CVoid * g_pVoid;

#endif