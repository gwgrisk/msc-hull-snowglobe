
#pragma once

#include <GXBase.h>
#include <AntiMatter\Timeline.h>

#include "SceneGraph.h"
#include "Camera.h"
#include "Projection.h"
#include "InputMgr.h"


class Simulation : public gxbase::GLWindowEx
{
private:
	SceneGraph*				m_pScene;
	Camera					m_Camera;
	Projection				m_Projection;
		
	AntiMatter::Timeline	m_Clock;

	int						m_nWidth;
	int						m_nHeight;
	bool					m_bLButton;
	bool					m_bRButton;
	bool					m_bFullScreen;

public:
	Simulation();
	~Simulation();

	void CenterTheSqueaker();

	void OnCreate();
	void OnDisplay();
	void OnResize(int w, int h);
	
	void OnKeyboard(int key, bool down);
	void OnMouseButton(MouseButton button, bool down);
	void OnMouseMove(int x, int y);

	void OnIdle();

};