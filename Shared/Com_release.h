#ifndef VOID_RELEASE_H
#define VOID_RELEASE_H

//Release specific stuff

#ifdef VOID_TEST
	static const char VOID_REG_KEY [] = "Software\\Devvoid\\VoidTest";
#else
	static const char VOID_REG_KEY [] = "Software\\Devvoid\\Void";
#endif

const char VOID_DEFAULTBINARYNAME[]	= "Void.exe";
const char VOID_VERSION[] = "0.1";

#endif