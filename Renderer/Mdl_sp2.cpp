#include "Standard.h"
#include "Mdl_sp2.h"
#include "I_file.h"
#include "ShaderManager.h"

typedef struct
{
	int			ident;
	int			version;
	int			numframes;
} sp2_header_t;



/*
=======================================
Constructor /Destructor
=======================================
*/
CModelSp2::CModelSp2()
{	
	frames = NULL;
}

CModelSp2::~CModelSp2()
{
	if (frames)
		delete [] frames;
}

/*
=======================================
LoadModel 
=======================================
*/
void CModelSp2::LoadModel(I_FileReader * pFile, const char * szFileName)
{
	ComPrintf("SP2: Loading %s\n", szFileName);

	sp2_header_t header;
	pFile->Read(&header, sizeof(sp2_header_t), 1);

	// make sure it's a valid sp2
	if ((header.ident != (('2'<<24)+('S'<<16)+('D'<<8)+'I')) || (header.version != 2))
	{
		ComPrintf("%s - invalid sp2 file", szFileName);
		pFile->Close();
		LoadFail();
		return;
	}

	// save file name
	modelfile = new char[strlen(szFileName)+1];
	strcpy(modelfile, szFileName);

	num_skins = header.numframes;
	num_frames= header.numframes;


	frames = new sp2_frame_t[num_frames];

	pFile->Read(frames, sizeof(sp2_frame_t), num_frames);
	pFile->Close();

	// skin names
	skin_names = new char*[num_skins];

	for (int s=0; s<num_frames; s++)
	{
		// md2 skin name list is 64 char strings
		skin_names[s] = new char[64];

		// strip path and extension
		for (int c=strlen(frames[s].skin_name); c>=0; c--)
		{
			if (frames[s].skin_name[c] == '.')
				frames[s].skin_name[c] = '\0';

			else if ((frames[s].skin_name[c] == '/') || (frames[s].skin_name[c] == '/'))
			{
				skin_names[s][0] = '_';
				strcpy(&skin_names[s][1], &frames[s].skin_name[c+1]);
				break;
			}
		}
	}
	LoadSkins();
}


/*
=======================================
LoadFail
=======================================
*/
void CModelSp2::LoadFail()
{
	modelfile = new char[strlen(MODEL_DEFAULT_NAME)+1];
	strcpy(modelfile, MODEL_DEFAULT_NAME);

	// hard code a pyramid
	num_skins = 1;
	num_frames= 1;

	frames = new sp2_frame_t[num_frames];

	frames[0].height = 32;
	frames[0].width  = 32;
	frames[0].origin_x = 16;
	frames[0].origin_y = 16;

	// skin names
	skin_names = new char*[1];

	// sp2 skin name list is 64 char strings
	skin_names[0] = new char[64];

	strcpy(skin_names[0], "none");
	LoadSkins();
}


/*
=======================================
Draw
=======================================
*/
extern const CCamera * camera;

void CModelSp2::Draw(int skin, int fframe, int cframe, float frac)
{
	float frame = fmodf(GetCurTime()*10, num_frames-1);
	fframe = (int)floorf(frame);
	cframe = fframe+1;
	frac = frame - fframe;

//	g_pRast->BlendFunc(VRAST_SRC_BLEND_SRC_ALPHA, VRAST_DST_BLEND_INV_SRC_ALPHA);

	// revers eye transform
	g_pRast->MatrixPush();
	g_pRast->MatrixRotateY(-camera->angles.YAW   * 180/PI);
	g_pRast->MatrixRotateX( camera->angles.PITCH * 180/PI);
	g_pRast->MatrixRotateZ(-camera->angles.ROLL  * 180/PI);


//	g_pRast->PolyColor4f(1, 1, 1, 1-frac);
//	g_pRast->TextureSet(skin_bin, fframe);
	g_pRast->ShaderSet(g_pShaders->GetShader(mShaderBin, fframe));

	g_pRast->PolyStart(VRAST_TRIANGLE_FAN);
	g_pRast->PolyTexCoord(0, 0);
	g_pRast->PolyVertexi(-frames[fframe].origin_x, -frames[fframe].origin_y);
	g_pRast->PolyTexCoord(1, 0);
	g_pRast->PolyVertexi(frames[fframe].width-frames[fframe].origin_x, -frames[fframe].origin_y);
	g_pRast->PolyTexCoord(1, 1);
	g_pRast->PolyVertexi(frames[fframe].width-frames[fframe].origin_x, frames[fframe].height-frames[fframe].origin_y);
	g_pRast->PolyTexCoord(0, 1);
	g_pRast->PolyVertexi(-frames[fframe].origin_x, frames[fframe].height-frames[fframe].origin_y);
	g_pRast->PolyEnd();
/*
	g_pRast->PolyColor4f(1, 1, 1, frac);
	g_pRast->TextureSet(skin_bin, cframe);
	g_pRast->PolyStart(VRAST_TRIANGLE_FAN);
	g_pRast->PolyTexCoord(0, 0);
	g_pRast->PolyVertexi(-frames[cframe].origin_x, -frames[cframe].origin_y);
	g_pRast->PolyTexCoord(1, 0);
	g_pRast->PolyVertexi(frames[cframe].width-frames[cframe].origin_x, -frames[cframe].origin_y);
	g_pRast->PolyTexCoord(1, 1);
	g_pRast->PolyVertexi(frames[cframe].width-frames[cframe].origin_x, frames[cframe].height-frames[cframe].origin_y);
	g_pRast->PolyTexCoord(0, 1);
	g_pRast->PolyVertexi(-frames[cframe].origin_x, frames[cframe].height-frames[cframe].origin_y);
	g_pRast->PolyEnd();
*/
	g_pRast->MatrixPop();
}