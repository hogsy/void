#include "Standard.h"
#include "Hud_main.h"
#include "Tex_main.h"

CRHud	  * g_prHud;

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

	y1 = rInfo->height - y;
	y2 = y1 - 8;

	x1 = x;
	x2 = x1 + 8;

	glBegin(GL_QUADS);
	glColor4f(1, 1, 1, 1);

	float s, t;
	for (int c = 0; c < len; c++)
	{
		s = (data[c] % 16) * 0.0625f;
		t = (data[c] / 16) * 0.0625f;

		glTexCoord2f(s, t);
		glVertex2i(x1, y1);

		glTexCoord2f(s + 0.0625f, t);
		glVertex2i(x2, y1);

		glTexCoord2f(s + 0.0625f, t + 0.0625f);
		glVertex2i(x2, y2);

		glTexCoord2f(s, t + 0.0625f);
		glVertex2i(x1, y2);

		//move to the right one character
		x1 = x2;
		x2 += 8;

	}

	glEnd();

	//Expire items who have passed the time limit
	if(time >= *g_pCurTime)
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
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, rInfo->width, 0, rInfo->height, -1, 1);

	glBindTexture(GL_TEXTURE_2D, tex->base_names[0]);
	glDisable(GL_DEPTH_TEST);


	for(int i=0;i<MAX_MESSAGES;i++)
	{
		if(m_hmessages[i].inuse)
		{
			m_hmessages[i].Draw();
		}
	}

	//restore everything
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}


/*
==========================================
Parse and add a message to the hud
==========================================
*/

void __stdcall CRHud::HudPrintf(int x, int y, float time, char *msg,...)
{
	char buff[256];

	va_list args;
	va_start(args, msg);
	vsprintf(buff, msg, args);
	va_end(args);

	for(int i=0;i<MAX_MESSAGES;i++)
		if(m_hmessages[i].inuse == false)
			break;

	//no more space
	if(i== MAX_MESSAGES)
		return;

	m_hmessages[i].Set(buff,x,y,time+(*g_pCurTime));
}


/*
==========================================
Just add a message to the hud
==========================================
*/
void __stdcall CRHud::HudPrint(char *msg, int x, int y, float time, int color)
{
	for(int i=0;i<MAX_MESSAGES;i++)
		if(m_hmessages[i].inuse == false)
			break;

	//no more space
	if(i== MAX_MESSAGES)
		return;
	
	m_hmessages[i].Set(msg,x,y,time+(*g_pCurTime));
}


/*
==========================================
Print a message, used for Console type 
that appear at the top of the screen
==========================================
*/
void __stdcall CRHud::PrintMessage(char *msg, int color, float time)
{
}


/*
==========================================

==========================================
*/


