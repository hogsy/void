#ifndef R_CONSOLERENDERER_H
#define R_CONSOLERENDERER_H

#include "I_console.h"
#include "I_renderer.h"
#include "Com_cvar.h"

/*
======================================================================================
This is just the console renderer
Shouldnt need to worry about logging here.
Just display the data it accumulates 
======================================================================================
*/

class CRConsole:public I_ConsoleRenderer,
				public I_CVarHandler,
				public I_CmdHandler
{
public:

	CRConsole();
	~CRConsole();

	bool Init(bool fullscreen, bool down);
	bool Shutdown();

	//Interface functions
	void Toggle(bool down);
	void ToggleFullscreen(bool full);
	void Lineup();
	void Linedown();
	void Statusline(const char  *status_line, const int &len);
	void AddLine(char *line, int color=0, int size=0);

	void UpdateRes();
	void Draw(); 

	bool HandleCVar(const CVarBase * cvar, const CParms &parms);
	void HandleCommand(HCMD cmdId, const CParms &parms);

	enum
	{
		CON_MAX_LINES		   = 200,
		CON_MAX_CHARS_PER_LINE = 200			// 8 pixels each at 1600
	};

private:

	void RegisterConObjects();
	void DestroyConObjects();

	//struct to store a line in the console
	struct Conline_t
	{
		Conline_t() { length = 0; line[0] = '\0'; }
		int		length;
		char 	line[CON_MAX_CHARS_PER_LINE];
	};
	Conline_t * m_lines[CON_MAX_LINES];

	//Console status
	enum EConStatus
	{
		CON_OPEN,
		CON_CLOSED,
		CON_OPENING,
		CON_CLOSING
	};
	EConStatus	m_status;

	//Private methods
	void AddMessage(char *buff, bool first);
	void PrintRecursive(bool first, char *msg);
	void PrintBuffer(DWORD top, DWORD bottom);

	//CVars
	CVar 		m_conSpeed;

	bool		m_condown;		// is the console down
	bool		m_fullscreen;	// fullscreen console?
	int			m_maxchars;		// chars per line
	int			m_maxlines;		// number of lines to display
	
	float		m_alpha;		// how much it has faded in
	
	int			m_curline;		// which is the first line to display - for scrolling up and down

	const char* m_statusline;	// current status line
	int			m_statuslen;	// lenght of current status line

	const char  m_seperatorchar;// constant seperator char using during scrolling
};

#endif
