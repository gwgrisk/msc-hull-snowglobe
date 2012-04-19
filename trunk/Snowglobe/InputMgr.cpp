
#include "stdafx.h"
#include "InputMgr.h"

KeyData::KeyData() :
	m_Up		(KeyUp),
	m_Down		(KeyUp),
	m_Left		(KeyUp),
	m_Right		(KeyUp),
	m_Use		(KeyUp),
	m_Space		(KeyUp),
	m_Shift		(KeyUp),
	m_Ctrl		(KeyUp),
	m_Alt		(KeyUp),
	m_Escape	(KeyUp)
{	
}

MouseData::MouseData() :
	m_nXPosition	( 0 ),
	m_nYPosition	( 0 ),
	m_rMouseSpeed	( 3.0f ),
	m_LButton		( MBtnUp ),
	m_RButton		( MBtnUp ),
	m_rWheelDelta	( 0.0f )
{
}
MouseData::MouseData( int x, int y, REAL rSpeed ) :
	m_nXPosition	(x),
	m_nYPosition	(y),
	m_rMouseSpeed	(rSpeed)
{
}