#include "Con_main.h"
#include "Standard.h"
#include "Tex_hdr.h"
#include "Hud_main.h"

//======================================================================================
//======================================================================================
#define CONBACK_INDEX			1
#define CON_DIFFERENTIAL		150

/*
=======================================
Constructor
=======================================
*/
CRConsole::CRConsole(): m_seperatorchar('^'),
						m_conSpeed("r_conspeed","500",CVAR_INT,CVAR_ARCHIVE),
						m_conAlpha("r_conalpha","200", CVAR_INT,CVAR_ARCHIVE)
						
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
	g_pConsole->RegisterCVar(&m_conAlpha);

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
	m_condown = down;
	UpdateRes();
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
			if (!m_condown)	// open/opening
				return;
			m_status = CON_OPENING;
			break;
		}
	case CON_CLOSING:
		{
			m_alpha -= (GetFrameTime() * m_conSpeed.ival);
			
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

			m_alpha += (GetFrameTime() * m_conSpeed.ival);
			
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
	if (m_alpha > m_conAlpha.ival)
		top = m_conAlpha.ival;

	DWORD bottom = (int) m_alpha - CON_DIFFERENTIAL;
	if (m_alpha < CON_DIFFERENTIAL)
		bottom = 0;

	if (bottom > m_conAlpha.ival)
		bottom = m_conAlpha.ival;

	float ftop = (float)top/255;
	float fbot = (float)bottom/255;

	// transform
	g_pRast->ProjectionMode(VRAST_ORTHO);
	
	if(m_fullscreen)
		ftop = fbot = 1;

	g_pRast->BlendFunc(VRAST_SRC_BLEND_SRC_ALPHA, VRAST_DEST_BLEND_ONE_MINUS_SRC_ALPHA);
	g_pRast->DepthFunc(VRAST_DEPTH_NONE);
	g_pRast->TextureSet(tex->bin_base, 1);

	g_pRast->PolyStart(VRAST_QUADS);

	g_pRast->PolyColor4f(ftop, ftop, ftop, ftop);
	g_pRast->PolyTexCoord(0, 0);
	g_pRast->PolyVertexi(0, g_rInfo.height);
	g_pRast->PolyTexCoord(1, 0);
	g_pRast->PolyVertexi(g_rInfo.width, g_rInfo.height);

	g_pRast->PolyColor4f(fbot, fbot, fbot, fbot);
	g_pRast->PolyTexCoord(1, 1);
	if(m_fullscreen)
		g_pRast->PolyVertexi(g_rInfo.width, 0);
	else
		g_pRast->PolyVertexi(g_rInfo.width, g_rInfo.height/2 - 1);
	g_pRast->PolyTexCoord(0, 1);
	if(m_fullscreen)
		g_pRast->PolyVertexi(0, 0);
	else
		g_pRast->PolyVertexi(0, g_rInfo.height/2 - 1);

	g_pRast->PolyEnd();

	//print all our text over the top
	PrintBuffer();
}


/*
======================================
Print Current Console Messages - 
assumes view and tex filter are ok
======================================
*/
void CRConsole::PrintBuffer()
{
	//Alpha values
	float ftop, fbot;
	float diff;
	float alpha;
	int a;


	DWORD top = (int) m_alpha;
	if (m_alpha > 255)
		top = 255;

	DWORD bottom = (int) m_alpha - CON_DIFFERENTIAL;
	if (m_alpha < CON_DIFFERENTIAL)
		bottom = 0;

	if (bottom > 255)
		bottom = 255;

	ftop = (float)top/255;
	fbot = (float)bottom/255;



	diff = float(top - bottom);
	diff /= m_maxlines;

	alpha = (float)bottom;

	a = (int)alpha;
	ftop = (float)a / 255;

	alpha += diff;
	fbot = (float)a / 255;


	g_pRast->TextureSet(tex->bin_base, 0);
	g_pRast->BlendFunc(VRAST_SRC_BLEND_SRC_ALPHA, VRAST_DEST_BLEND_ONE_MINUS_SRC_ALPHA);
	g_pRast->PolyStart(VRAST_QUADS);


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

		for(int lines =0; lines < numlines; lines ++)
		{
			if(lines)
			{
				y2 = y1;
				y1 += 8;
			}
			x1 = 0;
			x2 = 8;

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

				g_pRast->PolyColor4f(1, 1, 1, ftop);
				g_pRast->PolyTexCoord(s, t);
				g_pRast->PolyVertexi(x1, y1);

				g_pRast->PolyColor4f(1, 1, 1, ftop);
				g_pRast->PolyTexCoord(s + 0.0625f, t);
				g_pRast->PolyVertexi(x2, y1);

				g_pRast->PolyColor4f(1, 1, 1, fbot);
				g_pRast->PolyTexCoord(s + 0.0625f, t + 0.0625f);
				g_pRast->PolyVertexi(x2, y2);
		
				g_pRast->PolyColor4f(1, 1, 1, fbot);
				g_pRast->PolyTexCoord(s, t + 0.0625f);
				g_pRast->PolyVertexi(x1, y2);

				//move to the right one character
				x1 = x2;
				x2 += 8;
			}
		}
	}

	// Draw the blinking cursor
	if(((int)(GetCurTime()*5)) % 2)
	{
		//Print the cursor
		s = ('_' % 16) * 0.0625f;
		t = ('_' / 16) * 0.0625f;

		x1 = (m_statuslen % m_maxchars) * 8 - 8;
		if (x1<0) x1 = 0;
		x2 = x1+8;

		int cy2 = (m_fullscreen ? 0 : g_rInfo.height/2);
		int cy1 = cy2 + 8;

		g_pRast->PolyColor4f(1, 1, 1, ftop);
		g_pRast->PolyTexCoord(s, t);
		g_pRast->PolyVertexi(x1, cy1);

		g_pRast->PolyColor4f(1, 1, 1, ftop);
		g_pRast->PolyTexCoord(s + 0.0625f, t);
		g_pRast->PolyVertexi(x2, cy1);

		g_pRast->PolyColor4f(1, 1, 1, fbot);
		g_pRast->PolyTexCoord(s + 0.0625f, t + 0.0625f);
		g_pRast->PolyVertexi(x2, cy2);

		g_pRast->PolyColor4f(1, 1, 1, fbot);
		g_pRast->PolyTexCoord(s, t + 0.0625f);
		g_pRast->PolyVertexi(x1, cy2);
	}


	//Print the divider line thingie
	//size depends on resolution
	if(m_curline > 0)
	{
		y2=y1;
		y1+= 8;
		x1 = 0;
		x2 = 8;

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

			g_pRast->PolyColor4f(1, 1, 1, ftop);
			g_pRast->PolyTexCoord(s, t);
			g_pRast->PolyVertexi(x1, y1);

			g_pRast->PolyColor4f(1, 1, 1, ftop);
			g_pRast->PolyTexCoord(s + 0.0625f, t);
			g_pRast->PolyVertexi(x2, y1);

			g_pRast->PolyColor4f(1, 1, 1, fbot);
			g_pRast->PolyTexCoord(s + 0.0625f, t + 0.0625f);
			g_pRast->PolyVertexi(x2, y2);
	
			g_pRast->PolyColor4f(1, 1, 1, fbot);
			g_pRast->PolyTexCoord(s, t + 0.0625f);
			g_pRast->PolyVertexi(x1, y2);

			//move to the right one character
			x1 = x2;
			x2 += 8;
		}
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

		for (int c = 0; c < m_lines[l]->length; c++)
		{
			s = (m_lines[l]->line[c] % 16) * 0.0625f;
			t = (m_lines[l]->line[c] / 16) * 0.0625f;


			g_pRast->PolyColor4f(1, 1, 1, ftop);
			g_pRast->PolyTexCoord(s, t);
			g_pRast->PolyVertexi(x1, y1);

			g_pRast->PolyColor4f(1, 1, 1, ftop);
			g_pRast->PolyTexCoord(s + 0.0625f, t);
			g_pRast->PolyVertexi(x2, y1);

			g_pRast->PolyColor4f(1, 1, 1, fbot);
			g_pRast->PolyTexCoord(s + 0.0625f, t + 0.0625f);
			g_pRast->PolyVertexi(x2, y2);

			g_pRast->PolyColor4f(1, 1, 1, fbot);
			g_pRast->PolyTexCoord(s, t + 0.0625f);
			g_pRast->PolyVertexi(x1, y2);

		    //move to the right one character
			x1 = x2;
			x2 += 8;
		}
	}
	g_pRast->PolyEnd();
}



/*
======================================
Add a Message to the Console Buffer
======================================
*/
void CRConsole::AddMessage(const char *buff, bool first, LineColor color)
{
	// the first message includes that last message in the buffer
	// only scroll up if we aren't redoing this message
	if (!first)
	{
		//move everything back one spot.  end goes to the beginning
		Conline_t *last = m_lines[CON_MAX_LINES - 1];

		for (int l = CON_MAX_LINES-1; l > 0; l--)
			m_lines[l] = m_lines[l-1];

		m_lines[0] = last;
	}

	//set up the new entry
	m_lines[0]->length = strlen(buff); // + 2;
	if (m_lines[0]->length > m_maxchars)	// will never be more than we can fit
		m_lines[0]->length = m_maxchars;
	memcpy(&m_lines[0]->line, buff, m_lines[0]->length);
	m_lines[0]->line[m_lines[0]->length] = '\0';
	m_lines[0]->color = color;

	//we're scrolled up, stay in the same spot
	if(m_curline > 0)
		m_curline++;
}


/*
======================================
Print Func
======================================
*/
void CRConsole::PrintRecursive(bool first, char *msg, LineColor color)
{
	//split up the new line chars
	int c, len = strlen(msg);
	char tmp;

	if (msg[0] == '\0')
		return;

	// see  if it starts with a newline
	if (msg[0] == '\n')
	{
		AddMessage("\n", first, color);
		PrintRecursive(false, &msg[1],color);
		return;
	}

	//find the first \n char
	for (c = 0; c < len; c++)
	{
		if (msg[c] == '\n')
			break;
	}

	// was split by a \n - dont care about \n at end of line if there is one
	if ((c < len-1) && c>0)	
	{
		tmp = msg[c+1];
		msg[c+1] = '\0';

		PrintRecursive(first, msg,color);

		msg[c+1] = tmp;
		PrintRecursive(false, &msg[c+1],color);

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

			PrintRecursive(first, msg, color);

			msg[m_maxchars-2] = tmp;
			PrintRecursive(false, &msg[m_maxchars-2],color);
			return;
		}

		else	// split at the last space on this line
		{
			msg[c] = 0;
			PrintRecursive(first, msg, color);

			// skip the space
			msg[c] = ' ';
			PrintRecursive(false, &msg[c+1],color);
			return;
		}
	}
	AddMessage(msg, first,color);
	
//	if(m_status == CON_CLOSED)
//		g_prHud->PrintMessage(msg);
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
scroll up/down, top/bottom
======================================
*/
void CRConsole::MoveCurrentLine(LineOffset offset)
{
	switch(offset)
	{
	case LINE_UP:
		if((m_curline + 1 < CON_MAX_LINES) && (m_lines[m_curline]->length))
			m_curline++;
		break;
	case LINE_DOWN:
		if(m_curline >0)
			m_curline--;
		break;
	case PAGE_UP:
		break;
	case PAGE_DOWN:
		break;
	case TOP:
		while((m_curline + 1 < CON_MAX_LINES) && (m_lines[m_curline]->length))
			m_curline++;
		break;
	case BOTTOM:
		m_curline = 0;
		break;
	}
}

/*
======================================
set the bottom line that says what is 
currently being typed usually
======================================
*/
void  CRConsole::SetStatusline(const char  *status_line, const int &len)
{	
	m_statusline = status_line;
	m_statuslen = len;
}

/*
==========================================
Add a line to the console buffer
==========================================
*/
void  CRConsole::AddLine(const char *line, LineColor color, int size)
{
	char message[1024 + CON_MAX_CHARS_PER_LINE];	// max ComPrintf + max previous line
	strcpy(message, m_lines[0]->line);
	strcat(message, line);

	PrintRecursive(true, message, color);
}