
#include "ShaderManager.h"

CShaderManager	*g_pShaders=0;



/*
===================================================================================================
CShader
===================================================================================================
*/
CShaderManager::CShaderManager()
{
	mNumShaders = 0;
	mNumLightmaps = 0;
	mLightmapBin = -1;

	CFileBuffer	 fileReader;
	if (!fileReader.Open("Shaders/shaderlist.txt"))
		return;


	char token[1024];
	while (1)
	{
		fileReader.GetToken(token, true);
		if (!token[0])
			break;

		ParseShaders(token);
	}

	fileReader.Close();
}



CShaderManager::~CShaderManager()
{
	for (int s=0; s<mNumShaders; s++)
		delete mShaders[s];
}


/*
===========
ParseShaders
===========
*/
void CShaderManager::ParseShaders(char *shaderfile)
{
	CFileBuffer	 fileReader;
	char path[260];

	sprintf(path, "Shaders/%s.txt", shaderfile);

	if (!fileReader.Open(path))
		return;

	if (mNumShaders == MAX_SHADERS)
		FError("too many shaders - tell js\n");


	char token[1024];

	while (1)
	{
		fileReader.GetToken(token, true);
		if (!token[0])
			break;

		mShaders[mNumShaders] = new CShader(token);
		fileReader.GetToken(token, true);
		mShaders[mNumShaders]->Load(&fileReader);
		mNumShaders++;
	}

	fileReader.Close();
}







