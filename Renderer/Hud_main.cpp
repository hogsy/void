#include "Standard.h"
#include "Hud_hdr.h"
#include "Hud_main.h"
#include "ShaderManager.h"


/*
====================================================================================
Base Hud item class
====================================================================================
*/

CHudItem::CHudItem(): x(0),y(0),time(0.0f)
{	inuse = false;
}

CHudItem::CHudItem(int ix, int iy, float itime): x(ix),y(iy),time(itime)
{	inuse = true;
}

CHudItem::~CHudItem()
{	inuse = false;
	x = y = 0;
	time = 0;
}

void CHudItem::Reset()
{	x = y =0;
	inuse = false;
	time = 0.0f;
}

/*
====================================================================================
Message Class
====================================================================================
*/

//Constructor
CHudMessage::CHudMessage()
{
	len = 0;
	//memset(data,0,len);
	data = new char [HUD_MSGBUFFER];
}


CHudMessage::CHudMessage(const char *msg, int ix, int iy, float itime): CHudItem(ix,iy,itime)
{
	int len = strlen(msg);
	if(len > 256) len =256;
	strncpy(data,msg,len);
}

void CHudMessage::Set(const char *msg, int ix, int iy, float itime)
{
	x = ix;
	y = iy;
	time = itime;
	inuse = true;

	len = strlen(msg);
	if(len > HUD_MSGBUFFER) 
		len =HUD_MSGBUFFER;
	strncpy(data,msg,len);
}

//Destructor
CHudMessage::~CHudMessage()
{
	delete [] data;
	len = 0;
}

void CHudMessage::Reset()
{
	CHudItem::Reset();
	len =0;
	memset(data,0,HUD_MSGBUFFER);
}

/*
==========================================
Draw the message
==========================================
*/
void CHudMessage::Draw()
{
	int y1, y2;
	int x1, x2;

	y1 = g_rInfo.height - y;
	y2 = y1 - 8;

	x1 = x;
	x2 = x1 + 8;

	g_pRast->PolyStart(VRAST_QUADS);
	g_pRast->PolyColor(1, 1, 1, 1);

	float s, t;
	for (int c = 0; c < len; c++)
	{
		s = (data[c] % 16) * 0.0625f;
		t = (data[c] / 16) * 0.0625f;

		g_pRast->PolyTexCoord(s, t);
		g_pRast->PolyVertexi(x1, y1);

		g_pRast->PolyTexCoord(s + 0.0625f, t);
		g_pRast->PolyVertexi(x2, y1);

		g_pRast->PolyTexCoord(s + 0.0625f, t + 0.0625f);
		g_pRast->PolyVertexi(x2, y2);

		g_pRast->PolyTexCoord(s, t + 0.0625f);
		g_pRast->PolyVertexi(x1, y2);

		//move to the right one character
		x1 = x2;
		x2 += 8;
	}

	g_pRast->PolyEnd();

	//Expire items who have passed the time limit
	if(time >= GetCurTime())
	{	Reset();
	}
}




/*
====================================================================================
The HUD class
====================================================================================
*/

/*
==========================================
Constructor
==========================================
*/
CRHud::CRHud()
{	
	m_hmessages = new CHudMessage[MAX_MESSAGES];
	m_conmessages = new CHudMessage[MAX_TEXTLINES];
}

/*
==========================================
Destructor
==========================================
*/
CRHud::~CRHud()
{	
	delete [] m_hmessages;
	m_hmessages = 0;
	delete [] m_conmessages;
	m_conmessages = 0;
}


/*
==========================================
Draw Hud
draw active items
==========================================
*/

void CRHud::DrawHud()
{
	//Print Messages 

	//transform
	g_pRast->MatrixReset();
	g_pRast->ProjectionMode(VRAST_ORTHO);
	g_pRast->TextureLightDef(NULL);
	g_pRast->TextureTexDef(NULL);


	g_pRast->ShaderSet(g_pShaders->GetShader(g_pShaders->mBaseBin, 0));

	for(int i=0;i<MAX_MESSAGES;i++)
	{
		if(m_hmessages[i].inuse)
		{
			m_hmessages[i].Draw();
			g_pRast->ConAlpha(255, 255);
		}
	}

}


/*
==========================================
Parse and add a message to the hud
==========================================

void CRHud::HudPrintf(int x, int y, float time, char *msg)
{
	for(int i=0;i<MAX_MESSAGES;i++)
		if(m_hmessages[i].inuse == false)
			break;

	//no more space
	if(i== MAX_MESSAGES)
		return;

	m_hmessages[i].Set(msg,x,y,time+(GetCurTime()));
}
*/

/*
==========================================
Just add a message to the hud
==========================================
*/
void CRHud::Printf(int x, int y, float time, const char *msg, ...)
{
	va_list args;
	va_start(args, msg);
	vsprintf(m_hudBuffer, msg, args);
	va_end(args);

	for(int i=0;i<MAX_MESSAGES;i++)
		if(m_hmessages[i].inuse == false)
			break;

	//no more space
	if(i== MAX_MESSAGES)
		return;
	
	m_hmessages[i].Set(m_hudBuffer,x,y,time+(GetCurTime()));
}
	
/*void CRHud::HudPrint(char *msg, int x, int y, float time, int color)
{
	for(int i=0;i<MAX_MESSAGES;i++)
		if(m_hmessages[i].inuse == false)
			break;

	//no more space
	if(i== MAX_MESSAGES)
		return;
	
	m_hmessages[i].Set(msg,x,y,time+(GetCurTime()));
}
*/

void CRHud::AddConMessage(const char *msg, int color, float time)
{
}

/*
==========================================
Print a message, used for Console type 
that appear at the top of the screen
==========================================

void CRHud::PrintMessage(char *msg, int color, float time)
{
}
*/

/*
==========================================

==========================================
*/


