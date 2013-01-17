
#include "stdafx.h"
#include "Simulation.h"

#include <glm\glm.hpp>
#include <string>

#include <AntiMatter\AppLog.h>
#include <AntiMatter\AppException.h>
#include "Camera.h"
#include "SeasonalTimeline.h"
#include "InputMgr.h"
#include "EffectMgr.h"

Simulation::Simulation() : 
	m_pScene			( NULL ),
	m_nWidth			( 320 ),
	m_nHeight			( 240 ),
	m_bLButton			( false ),
	m_bRButton			( false ),
	m_bFullScreen		( false )
{
	using AntiMatter::AppLog;

	// gxbase calls
	SetSize(m_nWidth, m_nHeight);
	SetDepthBits(32);
	GLWindowEx::SetStencilBits(1);

	int attr[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 1,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
        0, 0
    };

	SetContextAttribs(attr);	

	AppLog::Ref().LogMsg( "Simulation start" );
}
Simulation::~Simulation()
{
	try
	{
		using namespace AntiMatter;
		AppLog::Ref().LogMsg("Simulation complete");
	}
	catch(...)
	{
	}
}

void Simulation::CenterTheSqueaker()
{	
	RECT rcClient;	
	GetWindowRect( this->GetSafeHwnd(), &rcClient );

	int nXOffset = rcClient.left;
	int nYOffset = rcClient.top;
	int nWidth	 = rcClient.right - rcClient.left;
	int nHeight	 = rcClient.bottom - rcClient.top;
	
	// Center the squeaker :D
	SetCursorPos((nWidth/2) + nXOffset, (nHeight/2) + nYOffset );

	// update the projection
	m_Projection.AspectRatio( (float)nWidth/nHeight );
	m_Camera.SetScreenSize( nWidth, nHeight );
	m_Camera.SetFOV( m_Projection.AspectRatio() );
	
}

void Simulation::OnCreate()
{
	using namespace AntiMatter;

	GLWindowEx::OnCreate();	// this loads OpenGL extensions
	
	// Initialize the EffectMgr singleton (have to do it here, since gxbase must be initialized before 
	// we can load any shaders
	EffectMgr::CreateInstance();	
	if( ! EffectMgr::Incarnated() )
	{
		AppLog::Ref().LogMsg( "%s failed to create the EffectMgr, halting execution", __FUNCTION__ );
		return;
	}

	SetFullscreen(true);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	SwapBuffers();

	// Initialize Projection
	float rAr = (float)m_nWidth / (float)m_nHeight;
	m_Projection.SetProjection( 45.0f, rAr, 0.1f, 3000.0f );
		
	// Initialize View
	m_Camera.SetScreenSize( m_nWidth, m_nHeight );
	m_Camera.SetFOV( m_Projection.FOV() );	
	m_Camera.SetPos( glm::vec3(0, 0, 800) );

	// Initialize SceneGraph
	m_pScene = new (std::nothrow) SceneGraph(&m_Camera, &m_Projection);
	if( ! m_pScene )
	{
		AppLog::Ref().LogMsg("Failed to allocate a SceneGraph, simulation cannot continue");
		Enforce<AppException>( false, "Failed to allocate a SceneGraph, simulation cannot continue" );
		// I know that this'll probably be uncaught.  Unknown what gxbase will do with exceptions
	}
	
	// Initialize OpenGL state	
	glEnable( GL_DEPTH_TEST );
	glClearColor( 0.1f, 0.1f, 0.2f, 0.0f );		// set the background clear colour to dark blue
	glShadeModel( GL_FLAT );					// set the shade model to smooth (Gouraud shading)	

	glEnable(GL_TEXTURE_2D);					// enable textures
	glEnable( GL_CULL_FACE );					// enable culling	
    glCullFace( GL_BACK );						// cull backfaces
	glFrontFace( GL_CCW );						// set triangle winding order

	// see 
	// http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=304207
	glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);
	glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);

	// Start the clock
	m_Clock.Start();
}

void Simulation::OnDisplay()
{
	using namespace AntiMatter;

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );			

	gluLookAt(	m_Camera.Pos().x,		m_Camera.Pos().y,		m_Camera.Pos().z,			// eye pos
				m_Camera.LookAt().x,	m_Camera.LookAt().y,	m_Camera.LookAt().z, 		// look at
				m_Camera.Up().x,		m_Camera.Up().y,		m_Camera.Up().z );			// up vector
					
	// Scene graph rendering
	m_pScene->Render();

	SwapBuffers();
}
void Simulation::OnResize( int w, int h )
{	
	// screen dimensions have changed, so update the aspect ratio for the projection matrix and
	// also for the camera
	m_nWidth	= w;
	m_nHeight	= h;

	m_Projection.AspectRatio( (float)w/h );
	m_Camera.SetFOV( m_Projection.FOV() );

	m_Camera.SetScreenSize(w, h);

	/*	
	*/
	// only required for fixed function stuff
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();	
	gluPerspective( m_Projection.FOV(), m_Projection.AspectRatio(), m_Projection.NearClipDist(), m_Projection.FarClipDist() );	
	glViewport(0,0,w,h);
	
	Redraw();
}
void Simulation::OnKeyboard(int key, bool down)
{
	using glm::mat4;
	using glm::vec3;

	switch( key )
	{
	case 37:// arrow left
		InputMgr::Ref().Keybd().Left( down ? KeyData::KeyDown : KeyData::KeyUp );
		break;

	case 39:// arrow right
		InputMgr::Ref().Keybd().Right( down ? KeyData::KeyDown : KeyData::KeyUp );
		break;

	case 107:// Keypad +
		SeasonalTimeline::Ref().IncSpeed();
		break;

	case 109:// Keypad -
		SeasonalTimeline::Ref().DecSpeed();
		break;

	case 0x21:// pgup
		InputMgr::Ref().Keybd().Up( down ? KeyData::KeyDown : KeyData::KeyUp );
		break;

	case 0x22:// pgdn
		InputMgr::Ref().Keybd().Down( down ? KeyData::KeyDown : KeyData::KeyUp );
		break;

	case 112:// F1
		SeasonalTimeline::Ref().Speed(0.0f);
		break;

	case 113:// F2
		SeasonalTimeline::Ref().Speed(1.0f);
		break;

	case 114:// F3
		SeasonalTimeline::Ref().Speed(2.0f);
		break;

	case 115:// F4
		SeasonalTimeline::Ref().Speed(3.0f);
		break;

	case 116:// F5
		SeasonalTimeline::Ref().Speed(4.0f);
		break;

	case 117:// F6
		SeasonalTimeline::Ref().Speed(5.0f);
		break;

	case 119:// F8
		if( ! down )
		{
			SceneLights::LightsState ls =  m_pScene->Lights().GetLightsState();

			if( ls == SceneLights::LightsState::sun )
				m_pScene->Lights().SetLightsState( SceneLights::LightsState::spots );
			else
				m_pScene->Lights().SetLightsState( SceneLights::LightsState::sun );
		}
		
		break;

	default:
		{
			switch( tolower(key) )
			{
				case 'm': // Cycle tree shader
				break;

				case 'p':	// Pause
				{
					if( SeasonalTimeline::Ref().Speed() == 0 )
						SeasonalTimeline::Ref().Resume();
					else
						SeasonalTimeline::Ref().Pause();
				}
				break;
		
				case 'w':	// Forward
					InputMgr::Ref().Keybd().Forward( down ? KeyData::KeyDown : KeyData::KeyUp );
					break;

				case 's':	// Back
					InputMgr::Ref().Keybd().Back( down ? KeyData::KeyDown : KeyData::KeyUp );
					break;

				case 'a':
					InputMgr::Ref().Keybd().Left( down ? KeyData::KeyDown : KeyData::KeyUp );
					break;

				case 'd':
					InputMgr::Ref().Keybd().Right( down ? KeyData::KeyDown : KeyData::KeyUp );
					break;

				case 'f':
					if( ! down )
					{
						m_bFullScreen = !m_bFullScreen;
						SetFullscreen(m_bFullScreen);
					}					
					break;

				case 'q':	// Quit
					Close();
					break;		

				default:
					break;
			}
		}
		break;
	}		
}

void Simulation::OnMouseButton( MouseButton button, bool down )
{
	switch(button)
	{
	case MBLeft: 
		m_bLButton = down ? true:false;
		InputMgr::Ref().Mouse().LBtn( down ? MouseData::MBtnDown : MouseData::MBtnUp );
		break;

	case MBRight:
		m_bRButton = down ? true:false;
		InputMgr::Ref().Mouse().RBtn( down ? MouseData::MBtnDown : MouseData::MBtnUp );
		break;

	default:
		break;
	}
}
void Simulation::OnMouseMove(int x, int y)
{		
	// Update the mouse position reading
	InputMgr::Ref().Mouse().x(x);
	InputMgr::Ref().Mouse().y(y);		
}
	
void Simulation::OnIdle()
{
	m_Clock.Tick();

	float rSecsDelta = (float)m_Clock.DeltaTime();	

	SeasonalTimeline::Ref().Update( rSecsDelta );
	m_Camera.Update( rSecsDelta );
	m_pScene->Update( rSecsDelta );
	
	// a gxbase call to redraw the screen
	Redraw();
}
