#include "Con_main.h"
#include "Standard.h"
#include "Tex_hdr.h"
#include "Hud_main.h"

//======================================================================================
//======================================================================================
#define CONBACK_INDEX			1
#define CON_DIFFERENTIAL		150

extern	CVar *	g_pConAlpha;

/*
=======================================
Constructor
=======================================
*/
CRConsole::CRConsole(): m_seperatorchar('^'),
						m_conSpeed("r_conspeed","500",CVar::CVAR_INT,CVar::CVAR_ARCHIVE)
						
{
	m_statuslen = 0;
	m_statusline = 0;
	m_alpha = 0.0;
	m_curline = 0;
	
	m_status = CON_OPEN;
	m_fullscreen = true;

	//Allocate space for Lines
	for (int i = 0; i < CON_MAX_LINES; i++)
	{
		m_lines[i] = new Conline_t();
		if (m_lines[i] == NULL) 
		{
			FError("Con_Init::Couldnt allocate space for Console line %d\n",i);
			return;
		}
	}

	g_pConsole->RegisterCVar(&m_conSpeed);

	RegisterConObjects();
}

/*
==========================================
Destructor
==========================================
*/
CRConsole::~CRConsole()
{
	m_statusline = 0;
	
	for (int c=0;c<CON_MAX_LINES;c++)
	{
		if(m_lines[c])
			delete m_lines[c];
	}
	DestroyConObjects();
}

/*
======================================
Initialize the console
======================================
*/
bool CRConsole::Init(bool fullscreen, bool down)
{
	m_status = CON_OPENING;
	m_fullscreen = fullscreen;
	UpdateRes();
	m_condown = down;
	return true;
}

/*
======================================
Console Shutdown Func
======================================
*/
bool CRConsole::Shutdown()
{	return true;
}


/*
======================================
Used for updating Resolutions etc
======================================
*/
void CRConsole::UpdateRes()
{
	if(m_fullscreen)
		m_maxlines = g_rInfo.height / 8;
	else
		m_maxlines = g_rInfo.height  / 16;
	m_maxchars =  g_rInfo.width / 8;
}

/*
======================================
Console Draw func, called every frame
======================================
*/
void CRConsole::Draw()
{
	switch(m_status)
	{
	case CON_CLOSED:
		{
			if (m_condown)	// open/opening
				m_status = CON_OPENING;
			else
				return;
			break;
		}
	case CON_CLOSING:
		{
			m_alpha -= (GetFrameTime() * m_conSpeed.value);
			
			if(m_condown)
				m_status = CON_OPENING;

			if (m_alpha <= 0)
			{
				m_condown = false;
				m_status = CON_CLOSED;
				m_alpha = 0;
				return;
			}
			break;
		}
	case CON_OPENING:
		{
			if(!world)
			{
				m_alpha = 255 + CON_DIFFERENTIAL;
				break;
			}

			m_alpha += (GetFrameTime() * m_conSpeed.value);
			
			if (m_alpha >= 255 + CON_DIFFERENTIAL)
			{
				m_status = CON_OPEN;
				m_alpha = 255 + CON_DIFFERENTIAL;
			}

			if(!m_condown)
				m_status = CON_CLOSING;
			break;
		}
	default:
		{
			if(!m_condown)
				m_status = CON_CLOSING;
			break;
		}
	}

	DWORD top = (int) m_alpha;
	if (m_alpha > (int)g_pConAlpha->value)
		top = (int)g_pConAlpha->value;

	DWORD bottom = (int) m_alpha - CON_DIFFERENTIAL;
	if (m_alpha < CON_DIFFERENTIAL)
		bottom = 0;

	if (bottom > (int)g_pConAlpha->value)
		bottom = (int)g_pConAlpha->value;

	float ftop = (float)top/255;
	float fbot = (float)bottom/255;

	// transform
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, g_rInfo.width, 0, g_rInfo.height, -1, 1);
	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, tex->base_names[1]);
	
	glBegin(GL_QUADS);

	glColor4f(ftop, ftop, ftop, ftop);
	glTexCoord2f(0, 0);
	glVertex2i(0, g_rInfo.height);
	glTexCoord2f(1, 0);
	glVertex2i(g_rInfo.width, g_rInfo.height);
	
	glColor4f(fbot, fbot, fbot, fbot);
	glTexCoord2f(1, 1);
	if(m_fullscreen)
		glVertex2i(g_rInfo.width, 0);
	else
		glVertex2i(g_rInfo.width, g_rInfo.height/2);
	glTexCoord2f(0, 1);
	if(m_fullscreen)
		glVertex2i(0, 0);
	else
		glVertex2i(0, g_rInfo.height/2);
	
	glEnd();

	//print all our text over the top
	PrintBuffer(top, bottom);

	//restore stuff
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}


/*
======================================
Print Current Console Messages - 
assumes view and tex filter are ok
======================================
*/
void CRConsole::PrintBuffer(DWORD top, DWORD bottom)
{
	//Alpha values
	float ftop, fbot;
	float diff;
	float alpha;
	int a;

	diff = float(top - bottom);
	diff /= m_maxlines;

	alpha = (float)bottom;

	a = (int)alpha;
	ftop = (float)a / 255;

	alpha += diff;
	fbot = (float)a / 255;


	glBindTexture(GL_TEXTURE_2D, tex->base_names[0]);

	//Text positioning
	//Find starting position

	int y1, y2;
	int x1, x2;
	float s,t;
	int start=0;
	int end = 0;
	
	//Set starting position for printing
	y2 = (m_fullscreen ? 0 : g_rInfo.height/2);
	y1 = y2 + 8;
	x1 = 0;
	x2 = 8;

	if(m_statuslen)
	{
		//Dont go over more than 512 chars in the input line
		if(m_statuslen > 512)
			m_statuslen = 512;

		//Find number of chars that dont fill up a line
		int numlines=1;
		if(m_statuslen > m_maxchars)
			numlines = (int)(m_statuslen /m_maxchars) + 1;

		glBegin(GL_QUADS);

		for(int lines =0; lines < numlines; lines ++)
		{
			if(m_statuslen == m_maxchars)
			{
				start = 0;
				end = m_statuslen;
			}
			else
			{
				start = m_statuslen - ((m_statuslen % m_maxchars) + (lines*m_maxchars));
				if(lines)
					end   = start + (lines*m_maxchars);
				else
					end = start + (m_statuslen % m_maxchars);
			}

			for(start; start < end; start ++)
			{
				s = (m_statusline[start] % 16) * 0.0625f;
				t = (m_statusline[start] / 16) * 0.0625f;

				glColor4f(ftop, ftop, ftop, ftop);
				glColor4f(1, 1, 1, ftop);
				glTexCoord2f(s, t);
				glVertex2i(x1, y1);

				glColor4f(ftop, ftop, ftop, ftop);
				glColor4f(1, 1, 1, ftop);
				glTexCoord2f(s + 0.0625f, t);
				glVertex2i(x2, y1);

				glColor4f(fbot, fbot, fbot, fbot);
				glColor4f(1, 1, 1, fbot);
				glTexCoord2f(s + 0.0625f, t + 0.0625f);
				glVertex2i(x2, y2);
		
				glColor4f(fbot, fbot, fbot, fbot);
				glColor4f(1, 1, 1, fbot);
				glTexCoord2f(s, t + 0.0625f);
				glVertex2i(x1, y2);

				//move to the right one character
				x1 = x2;
				x2 += 8;
			}

			if(numlines > 1)
			{
				y2 = y1;
				y1 += 8;
			}
			x1 = 0;
			x2 = 8;
		}
		glEnd();
	}


	//Print the divider line thingie
	//size depends on resolution
	if(m_curline > 0)
	{
		y2=y1;
		y1+= 8;

		glBegin(GL_QUADS);

		for(start = 0; start < m_maxchars; start++)
		{
			if(start %3)
			{
				s = (' ' % 16) * 0.0625f;
				t = (' ' / 16) * 0.0625f;
			}
			else
			{
				s = (m_seperatorchar % 16) * 0.0625f;
				t = (m_seperatorchar / 16) * 0.0625f;
			}

			glColor4f(ftop, ftop, ftop, ftop);
			glTexCoord2f(s, t);
			glVertex2i(x1, y1);

			glColor4f(ftop, ftop, ftop, ftop);
			glTexCoord2f(s + 0.0625f, t);
			glVertex2i(x2, y1);

			glColor4f(fbot, fbot, fbot, fbot);
			glTexCoord2f(s + 0.0625f, t + 0.0625f);
			glVertex2i(x2, y2);
	
			glColor4f(fbot, fbot, fbot, fbot);
			glTexCoord2f(s, t + 0.0625f);
			glVertex2i(x1, y2);

			//move to the right one character
			x1 = x2;
			x2 += 8;
		}
		
		glEnd();
	}


	unsigned int endline = m_curline + m_maxlines - 1;
	for(uint l = m_curline; l < endline; l++)
	{
		if (m_lines[l]->line[0] == '\0')
			break;

		// move up one line
		y2 = y1;
		y1 += 8;
		
		x1 = 0;
		x2 = 8;

	    // find our new interpolated alpha values
		fbot = ftop;
		alpha += diff;
		ftop = (float)a/255;

		glBegin(GL_QUADS);

		for (int c = 0; c < m_lines[l]->length; c++)
		{
			s = (m_lines[l]->line[c] % 16) * 0.0625f;
			t = (m_lines[l]->line[c] / 16) * 0.0625f;



			glColor4f(ftop, ftop, ftop, ftop);
			glTexCoord2f(s, t);
			glVertex2i(x1, y1);

			glColor4f(ftop, ftop, ftop, ftop);
			glTexCoord2f(s + 0.0625f, t);
			glVertex2i(x2, y1);

			glColor4f(fbot, fbot, fbot, fbot);
			glTexCoord2f(s + 0.0625f, t + 0.0625f);
			glVertex2i(x2, y2);

			glColor4f(fbot, fbot, fbot, fbot);
			glTexCoord2f(s, t + 0.0625f);
			glVertex2i(x1, y2);

		    //move to the right one character
			x1 = x2;
			x2 += 8;
		}
		glEnd();
	}
}



/*
======================================
Add a Message to the Console Buffer
======================================
*/
void CRConsole::AddMessage(char *buff, bool first)
{
	//move everything back one spot.  end goes to the beginning
	Conline_t *last = m_lines[CON_MAX_LINES - 1];

	for (int l = CON_MAX_LINES-1; l > 0; l--)
		m_lines[l] = m_lines[l-1];

	m_lines[0] = last;

	//set up the new entry
	last->length = strlen(buff) + 2;
	if (last->length > m_maxchars)	// will never be more than we can fit
		last->length = m_maxchars;

	int c = first ? '>' : ' ';
	last->line[0] = last->line[1] = c;
	memcpy(&last->line[2], buff, last->length-2);

	//we're scrolled up, stay in the same spot
	if(m_curline > 0)
		m_curline++;
}


/*
======================================
Print Func
======================================
*/
void CRConsole::PrintRecursive(bool first, char *msg)
{
	//split up the new line chars
	int c, len = strlen(msg);
	char tmp;

	//take trailing \n off if there is one
	if (msg[len-1] == '\n')
	{
		msg[len-1] = 0;
		len--;
	}

	//find the first \n char
	for (c = 0; c < len; c++)
	{
		if (msg[c] == '\n')
			break;
	}

	if ((c < len) && c>0)	// was split by a \n
	{
		tmp = msg[c];
		msg[c] = 0;

		PrintRecursive(first, msg);

		msg[c] = tmp;
		PrintRecursive(false, &msg[c]);

		return;
	}

	//break it up to fit on single lines
	if (len > m_maxchars-2)
	{
		//find the last space that would be on this line
		int c = (len > m_maxchars-2) ? m_maxchars-2 : len;
		for (; c >= 0; c--)
		{
			if (msg[c] == ' ')
				break;
		}

		if (c < 0)	// no spaces at all - split at the lenght of the line
		{
			tmp = msg[m_maxchars-2];
			msg[m_maxchars-2] = 0;

			PrintRecursive(first, msg);
			
			msg[m_maxchars-2] = tmp;
			PrintRecursive(false, &msg[m_maxchars-2]);

			return;
		}

		else	// split at the last space on this line
		{
			msg[c] = 0;
			PrintRecursive(first, msg);

			msg[c] = ' ';
			PrintRecursive(false, &msg[c+1]);

			return;
		}
	}
	AddMessage(msg, first);
	
	if(m_status == CON_CLOSED)
		g_prHud->PrintMessage(msg);
}

/*
====================================================================================
I_RConsole Implementation
====================================================================================
*/

/*
======================================
Console Toggle
======================================
*/
void  CRConsole::Toggle(bool down)
{	m_condown = down;
}

/*
======================================
set full or half screen console
======================================
*/

void  CRConsole::ToggleFullscreen(bool full)
{
	m_fullscreen = full;
	UpdateRes();
}

/*
======================================
scroll up a line
======================================
*/
void  CRConsole::Lineup()
{	
	if(m_curline < (CON_MAX_LINES-1))
		m_curline++;
}

/*
======================================
scroll down a line
======================================
*/
void  CRConsole::Linedown()
{	if(m_curline >0)
		m_curline--;
}

/*
======================================
set the bottom line that says what is currently being typed usually
======================================
*/
void  CRConsole::Statusline(const char  *status_line, const int &len)
{	m_statusline = status_line;
	m_statuslen = len;
}

/*
==========================================
Add a line to the console buffer
==========================================
*/
void  CRConsole::AddLine(char *line, int color, int size)
{	PrintRecursive(true, line);
}