#include "Sys_hdr.h"
#include "In_state.h"

namespace
{
	const float KB_REPEATWAIT = 0.5f;
}
using namespace VoidInput;

//======================================================================================
//======================================================================================

/*
==========================================
Constructor and destructor
==========================================
*/
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

/*
==========================================
Set the current Key listener,
sets to self if 0
==========================================
*/
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

/*
==========================================
Set current cursor listener,
default to self if 0
==========================================
*/
void CInputState::SetCursorListener(I_InCursorListener * plistener)
{
	if(plistener)
		m_pCursorHandler = plistener;
	else
		m_pCursorHandler = &m_defCurHandler;
}


/*
==========================================
A device has updated the state of a a key
this will update input state, and see if
an event needs to be dispatched
==========================================
*/
void CInputState::UpdateKey(int keyid, EButtonState keyState)
{
	if(keyid >= INKEY_LEFTSHIFT && keyid <= INKEY_RIGHTALT)
	{
		switch(keyid)
		{
		case INKEY_LEFTSHIFT:
		case INKEY_RIGHTSHIFT:
			if(keyState == BUTTONDOWN)
				m_keyEvent.flags |= SHIFTDOWN;
			else if(m_aHeldKeys[keyid].state != BUTTONUP)
				m_keyEvent.flags &= ~SHIFTDOWN;
			break;
		case INKEY_LEFTCTRL:
		case INKEY_RIGHTCTRL:
			if(keyState == BUTTONDOWN)
				m_keyEvent.flags |= CTRLDOWN;
			else if(m_aHeldKeys[keyid].state != BUTTONUP)
				m_keyEvent.flags &= ~CTRLDOWN;
			break;
		case INKEY_LEFTALT:
		case INKEY_RIGHTALT:
			if(keyState == BUTTONUP)
				m_keyEvent.flags |= ALTDOWN;
			else if(m_aHeldKeys[keyid].state != BUTTONUP)
				m_keyEvent.flags &= ~ALTDOWN;
			break;
		}
	}

	if((keyState == BUTTONDOWN) &&
	   (m_aHeldKeys[keyid].state == BUTTONUP))
	{
		m_keyEvent.id = keyid;
		m_keyEvent.time = System::GetCurTime();
		m_keyEvent.state = BUTTONDOWN;

		m_aHeldKeys[keyid].id = keyid;
		m_aHeldKeys[keyid].state = BUTTONHELD;
		m_aHeldKeys[keyid].time = System::GetCurTime() + (m_fRepeatRate + 0.4);
		
		//Apply flags
		if(m_keyEvent.flags & SHIFTDOWN)
			ShiftCharacter(m_keyEvent.id);
			
		//Dispatch event
		m_pKeyHandler->HandleKeyEvent(m_keyEvent);
	}
	else if((keyState == BUTTONUP) &&
			(m_aHeldKeys[keyid].state != BUTTONUP))
	{
		//Send new Keystate
		m_keyEvent.id = keyid;
		m_keyEvent.time = System::GetCurTime();
		m_keyEvent.state = BUTTONUP;

		//Reset old keystate
		m_aHeldKeys[keyid].id = keyid;
		m_aHeldKeys[keyid].time = 0.0f;
		m_aHeldKeys[keyid].state = BUTTONUP;

		//Apply flags
		if(m_keyEvent.flags & SHIFTDOWN)
			ShiftCharacter(m_keyEvent.id);
		
		//Dispatch event
		m_pKeyHandler->HandleKeyEvent(m_keyEvent);
	}
}

/*
==========================================
Runs everyframe if Keylistener wants to be
notified of held keys. dispatches any keys
which have passed the repeate rate
==========================================
*/
void CInputState::DispatchKeys()
{
	//Does the listener require repeat events ?
	if(m_bRepeatEvents)
	{
		//Check for any keys that qualify to send out another repeat event
		for(int i=0;i<IN_NUMKEYS;i++)
		{
			if((m_aHeldKeys[i].state == BUTTONHELD) &&
			   (System::GetCurTime() > m_aHeldKeys[i].time))
			{
				m_keyEvent.id   = m_aHeldKeys[i].id; 
				m_keyEvent.time = System::GetCurTime();
				m_keyEvent.state= BUTTONHELD;

				//Update time
				m_aHeldKeys[i].time = System::GetCurTime() + m_fRepeatRate;

				if(m_keyEvent.flags & SHIFTDOWN)
					ShiftCharacter(m_keyEvent.id);

				m_pKeyHandler->HandleKeyEvent(m_keyEvent);
			}
		}
	}
}

/*
==========================================
A device messed up. release all the keys
==========================================
*/
void CInputState::FlushKeys()
{
	//Send key up events for all keys currently down, to reset them
//	ComPrintf("CInputState::Flushing keys\n");

	for(int i=0;i<IN_NUMKEYS;i++)
	{
//		m_keyEvent.flags = 0;

		if(m_aHeldKeys[i].state != BUTTONUP)
		{
			m_keyEvent.time = System::GetCurTime();
			m_keyEvent.state = BUTTONUP;
			m_keyEvent.id = m_aHeldKeys[i].id; 

			m_pKeyHandler->HandleKeyEvent(m_keyEvent);

			m_aHeldKeys[i].time = 0.0f;
			m_aHeldKeys[i].state = BUTTONUP;
			m_aHeldKeys[i].id = 0;
		}
	}
	m_keyEvent.flags = 0;
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
