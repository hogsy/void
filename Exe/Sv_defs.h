#ifndef VOID_MAIN_SERVER
#define VOID_MAIN_SERVER

/*
================================================
Only need to exposed this to Sys_main
================================================
*/

namespace VoidServer
{
	bool InitializeNetwork();
	void ShutdownNetwork();

	void Create();
	void Destroy();
	void RunFrame();
}


#endif