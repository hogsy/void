#include "Sys_hdr.h"
#include "In_state.h"

namespace
{
	const float KB_REPEATWAIT = 0.3f;
}

using namespace VoidInput;

CInputState::CInputState()
{
	m_pCursorHandler = &m_defCurHandler;
	m_pKeyHandler = &m_defKeyHandler;
	m_bRepeatEvents = false;
	m_fRepeatRate = IN_DEFAULTREPEATRATE;
}
	
CInputState::~CInputState()
{
	m_pCursorHandler = 0;
	m_pKeyHandler = 0;
}

void CInputState::SetKeyListener(I_InKeyListener * plistener,
								 bool bRepeatEvents,
								 float fRepeatRate)
{
	FlushKeys();
	if(plistener)
		m_pKeyHandler = plistener;
	else
		m_pKeyHandler = &m_defKeyHandler;
	m_bRepeatEvents = bRepeatEvents;
	m_fRepeatRate = fRepeatRate;
}

	
void CInputState::SetCursorListener(I_InCursorListener * plistener)
{
	if(plistener)
		m_pCursorHandler = plistener;
	else
		m_pCursorHandler = &m_defCurHandler;
}

void CInputState::UpdateKey(int keyid, EButtonState keyState)
{
	if(keyid >= INKEY_LEFTSHIFT && keyid <= INKEY_RIGHTALT)
	{
		switch(keyid)
		{
		case INKEY_LEFTSHIFT:
		case INKEY_RIGHTSHIFT:
			if(keyState == BUTTONUP)
				m_keyEvent.flags &= ~SHIFTDOWN;
			else
				m_keyEvent.flags |= SHIFTDOWN;
			break;
		case INKEY_LEFTCTRL:
		case INKEY_RIGHTCTRL:
			if(keyState == BUTTONUP)
				m_keyEvent.flags &= ~CTRLDOWN;
			else
				m_keyEvent.flags |= CTRLDOWN;
			break;
		case INKEY_LEFTALT:
		case INKEY_RIGHTALT:
			if(keyState == BUTTONUP)
				m_keyEvent.flags &= ~ALTDOWN;
			else
				m_keyEvent.flags |= ALTDOWN;
			break;
		}
	}

	if((keyState == BUTTONDOWN) &&
	   (m_aHeldKeys[keyid].state == BUTTONUP))
	{
		m_aHeldKeys[keyid].id = keyid;
		m_aHeldKeys[keyid].state = BUTTONDOWN;
		
		m_keyEvent.id = keyid;
		m_keyEvent.time = System::g_fcurTime;
		m_keyEvent.state = BUTTONDOWN;
		
		//Apply flags
		if(m_keyEvent.flags & SHIFTDOWN)
			ShiftCharacter(m_keyEvent.id);
			
		//Dispatch event
		m_pKeyHandler->HandleKeyEvent(m_keyEvent);
	}
	else if((keyState == BUTTONUP) &&
			(m_aHeldKeys[keyid].state != BUTTONUP))
	{
		//Reset old keystate
		m_aHeldKeys[keyid].id = keyid;
		m_aHeldKeys[keyid].time = 0.0f;
		m_aHeldKeys[keyid].state = BUTTONUP;

		//Send new Keystate
		m_keyEvent.id = keyid;
		m_keyEvent.time = System::g_fcurTime;
		m_keyEvent.state = BUTTONUP;

		//Apply flags
		if(m_keyEvent.flags & SHIFTDOWN)
			ShiftCharacter(m_keyEvent.id);
		
		//Dispatch event
		m_pKeyHandler->HandleKeyEvent(m_keyEvent);
	}
}

void CInputState::DispatchKeys()
{
	//Does the listener require repeat events ?
	if(m_bRepeatEvents)
	{
		//Check for any keys that qualify to send out another repeat event
		for(int i=0;i<IN_NUMKEYS;i++)
		{
			//Set all the BUTTONDOWN events to BUTTONHELD
			if(m_aHeldKeys[i].state == BUTTONDOWN)
			{
				m_aHeldKeys[i].state = BUTTONHELD;
				m_aHeldKeys[i].time = System::g_fcurTime + KB_REPEATWAIT;
			}
			//Dispatch HELD mouse events, if time passed since last dispatch
			//is bigger than the repeat rate
			else if((m_aHeldKeys[i].state == BUTTONHELD) &&
					(System::g_fcurTime > (m_aHeldKeys[i].time + m_fRepeatRate)))
			{
				m_keyEvent.id = m_aHeldKeys[i].id; 
				m_keyEvent.time = m_aHeldKeys[i].time = System::g_fcurTime;
				m_keyEvent.state = BUTTONHELD;

				if(m_keyEvent.flags & SHIFTDOWN)
					ShiftCharacter(m_keyEvent.id);

				m_pKeyHandler->HandleKeyEvent(m_keyEvent);
			}
		}
	}
}

void CInputState::FlushKeys()
{
	//Send key up events for all keys currently down, to reset them
	for(int i=0;i<IN_NUMKEYS;i++)
	{
		m_keyEvent.flags = 0;

		if(m_aHeldKeys[i].state != BUTTONUP)
		{
			m_keyEvent.time = System::g_fcurTime;
			m_keyEvent.state = BUTTONUP;
			m_keyEvent.id = m_aHeldKeys[i].id; 

			m_aHeldKeys[i].time = 0.0f;
			m_aHeldKeys[i].state = BUTTONUP;
			m_aHeldKeys[i].id = 0;
			
			m_pKeyHandler->HandleKeyEvent(m_keyEvent);
		}
	}
}


/*
=====================================
changed val to its "shifted" equivalent
=====================================
*/
void CInputState::ShiftCharacter(int &val)
{	
	if(val >= 'a' && val <= 'z')
		val = val - 'a' + 'A';
	else 
	{
		switch(val)
		{
			case '0':
				val = ')';
				break;
			case '1':
				val = '!';
				break;
			case '2':
				val = '@';
				break;
			case '3':
				val = '#';
				break;
			case '4':
				val = '$';
				break;
			case '5':
				val = '%';
				break;
			case '6':
				val = '^';
				break;
			case '7':
				val = '&';
				break;
			case '8':
				val = '*';
				break;
			case '9':
				val = '(';
				break;
			case '-':
				val = '_';
				break;
			case '=':
				val = '+';
				break;
			case '[':
				val = '{';
				break;
			case ']':
				val = '}';
				break;
			case '\\':
				val = '|';
				break;
			case ';':
				val = ':';
				break;
			case '\'':
				val = '"';
				break;
			case ',':
				val = '<';
				break;
			case '.':
				val = '>';
				break;
			case '/':
				val = '?';
				break;
			case '`':
				val = '~';
				break;
		}
	}
}
