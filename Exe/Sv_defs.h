#ifndef VOID_SERVER_INTERFACE
#define VOID_SERVER_INTERFACE

/*
================================================
Only need to expose this to Sys_main
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
