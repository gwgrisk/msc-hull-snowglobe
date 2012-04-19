
#pragma once

#include <AntiMatter\precision.h>
#include <AntiMatter\Singleton.h>

class KeyData
{
public:
	enum KeyState { KeyUp, KeyDown };

private: 
	KeyState	m_Up;
	KeyState	m_Down;

	KeyState	m_Forward;
	KeyState	m_Back;
	KeyState	m_Left;
	KeyState	m_Right;

	KeyState	m_Use;

	KeyState	m_Space;
	KeyState	m_Shift;
	KeyState	m_Ctrl;
	KeyState	m_Alt;
	KeyState	m_Escape;

public:
	KeyData();

	void Reset()
	{
		m_Up		= KeyUp;
		m_Down		= KeyUp;
		m_Forward	= KeyUp;
		m_Back		= KeyUp;
		m_Left		= KeyUp;
		m_Right		= KeyUp;
		m_Use		= KeyUp;
		m_Space		= KeyUp;
		m_Shift		= KeyUp;
		m_Ctrl		= KeyUp;
		m_Alt		= KeyUp;
		m_Escape	= KeyUp;
	}

	// gets
	const KeyState Up() const		{ return m_Up; }
	const KeyState Down() const		{ return m_Down; }
	const KeyState Forward() const	{ return m_Forward; }
	const KeyState Back() const		{ return m_Back; }
	const KeyState Left() const		{ return m_Left; }
	const KeyState Right() const	{ return m_Right; }
	const KeyState Use() const		{ return m_Use; }
	const KeyState Space() const	{ return m_Space; }
	const KeyState Shift() const	{ return m_Shift; }
	const KeyState Ctrl() const		{ return m_Ctrl; }
	const KeyState Alt() const		{ return m_Alt; }
	const KeyState Escape() const	{ return m_Escape; }

	// sets
	void Up( const KeyState n )		{ m_Up		= n; }
	void Down( const KeyState n )	{ m_Down	= n; }
	void Forward( const KeyState n ){ m_Forward	= n; }
	void Back( const KeyState n )	{ m_Back	= n; }
	void Left( const KeyState n )	{ m_Left	= n; }
	void Right( const KeyState n )	{ m_Right	= n; }
	void Use( const KeyState n )	{ m_Use		= n; }
	void Space( const KeyState n )	{ m_Space	= n; }
	void Shift( const KeyState n )	{ m_Shift	= n; }
	void Ctrl( const KeyState n )	{ m_Up		= n; }
	void Alt( const KeyState n )	{ m_Up		= n; }
	void Escape( const KeyState n )	{ m_Up		= n; }
};
class MouseData
{
public:
	enum MouseBtn { MBtnUp, MBtnDown };

private:
	int			m_nXPosition;
	int			m_nYPosition;	
	REAL		m_rMouseSpeed;

	MouseBtn	m_LButton;
	MouseBtn	m_RButton;
	REAL		m_rWheelDelta;

public:
	MouseData();
	MouseData( int x, int y, REAL rSpeed );

	// gets
	const int x() const			{ return m_nXPosition; }
	const int y() const			{ return m_nYPosition; }
	const REAL speed() const	{ return m_rMouseSpeed; }

	const MouseBtn LBtn() const	{ return m_LButton; }
	const MouseBtn RBtn() const	{ return m_RButton; }

	// sets
	void x( const int & x )			{ m_nXPosition = x; }
	void y( const int & y )			{ m_nYPosition = y; }
	void speed( const REAL & r )	{ m_rMouseSpeed = r; }
	void LBtn( const MouseBtn & n )	{ m_LButton = n; }
	void RBtn( const MouseBtn & n )	{ m_RButton = n; }
};

class InputMgr : public Singleton<InputMgr>
{
private:
	KeyData		m_keyboard;
	MouseData	m_mouse;

public:
	KeyData & Keybd()	{ return m_keyboard; }
	MouseData & Mouse()	{ return m_mouse; }
};