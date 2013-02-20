
#include "stdafx.h"
#include "Drift.h"

#include "slowglobe-cfg.h"
#include "SceneGraph.h"

Drift::Drift() : 
	IGraphNode( NULL, NULL, std::string("Drift") ),
	m_bInitialized	( false ),
	m_nRows			( 0 ),
	m_nCols			( 0 ),
	m_rCellWidth	( 0.0f ),
	m_rCellHeight	( 0.0f ),
	m_rCellDepth	( 0.0f )
{
}
Drift::Drift(  SceneGraph* pGraph, IGraphNode* pParent, const std::string & sId, const std::string & sConfigFile ) :
	IGraphNode		( pGraph, pParent, sId ),
	m_bInitialized	( false ),
	m_sConfigFile	( sConfigFile ),
	m_nRows			( 0 ),
	m_nCols			( 0 ),
	m_rCellWidth	( 0.0f ),
	m_rCellHeight	( 0.0f ),
	m_rCellDepth	( 0.0f )
{
	m_bInitialized = Initialize();
}
Drift::Drift( const Drift & rhs ) :
	m_bInitialized	( false ),
	m_nRows			( 0 ),
	m_nCols			( 0 ),
	m_rCellWidth	( 0.0f ),
	m_rCellHeight	( 0.0f ),
	m_rCellDepth	( 0.0f )
{
	m_sConfigFile = rhs.m_sConfigFile;

	if( rhs.Initialized() )	
		m_bInitialized = Initialize();	
}
Drift & Drift::operator=( const Drift & rhs )
{
	if( this != &rhs )
	{
		m_pGraph	= rhs.m_pGraph;
		m_pParent	= rhs.m_pParent;
		m_sId		= rhs.m_sId;
		
		if( rhs.Initialized() )
			Initialize();
	}

	return *this;
}

Drift::~Drift()
{
	Uninitialize();
}

bool Drift::Initialize()
{
	if( m_bInitialized )
		return true;

	using AntiMatter::AppLog;
	using AntiMatter::Shell::FileExists;
	using namespace std;

	glex::Load();


	// attempt to load data from config file
	if( FileExists( m_sConfigFile ) )
	{
		ifstream in;
		in.open( m_sConfigFile, std::ios::in );

		if( in.good() )
		{
			in >> *this;
			in.close();
		}
		else
		{
			AppLog::Ref().LogMsg("%s config file invalid", __FUNCTION__ );
			Uninitialize();
			return false;
		}
	}
	else
	{
		AppLog::Ref().LogMsg("%s config file not found", __FUNCTION__ );
		Uninitialize();
		return false;
	}

	if( ! LoadAssets() )
	{
		AppLog::Ref().LogMsg("%s load assets failed", __FUNCTION__ );
		Uninitialize();
		return false;
	}

	m_Plane = Plane( m_nCols, m_nRows, m_rCellWidth, m_rCellHeight, m_rCellDepth );

	return true;
}
void Drift::Uninitialize()
{
	m_bInitialized = false;

	if( m_AlphaMap.Initialized() )	
		m_AlphaMap.Uninitialize();

	m_sAlphaMap		= "";
	m_sConfigFile	= "";
	m_nRows			= 0;
	m_nCols			= 0;
	m_rCellWidth	= 0.0f;
	m_rCellHeight	= 0.0f;
	m_rCellDepth	= 0.0f;

}

bool Drift::LoadAssets()
{
	using AntiMatter::AppLog;
	using AntiMatter::Shell::FileExists;
	using std::string;

	string sMap = g_Cfg.AssetsDir() + m_sAlphaMap;

	if( FileExists( sMap ) )
		m_AlphaMap = Texture( sMap );

	return ( m_AlphaMap.Initialized()? true:false );
}

// IGraphNode
HRESULT Drift::Update( const float & rSecsDelta )
{
	if( ! m_bInitialized )
		return S_OK;

	m_Data.MVP()	=	m_pGraph->Proj().P() * 
						m_pGraph->Cam().V() * 
						m_pParent->GetNodeData().W() * 
						m_Data.W();

	// Updates child nodes
	if( IGraphNode::HasChildNodes() )
		IGraphNode::UpdateChildren( rSecsDelta );

	return S_OK;
}
HRESULT Drift::PreRender()
{
	return S_OK;
}
HRESULT Drift::Render()
{
	return S_OK;
}
HRESULT Drift::PostRender()
{
	return S_OK;
}
HRESULT Drift::DrawItem()
{
	// Execute default behaviour, which renders all child nodes first
	IGraphNode::DrawItem();	

	return S_OK;
}

// streaming
std::ostream & operator << ( std::ostream & out, const Drift & r )
{
	using AntiMatter::AppLog;	
	using std::string;

	try
	{
		out << "Drift" << " ";
		out << r.AlphaMapFile() << " ";
		out << r.Rows() << " ";
		out << r.Cols() << " ";
		out << r.CellWidth() << " ";
		out << r.CellHeight() << " ";
		out << r.CellDepth() << std::endl;
	}
	catch(...)
	{
		AppLog::Ref().LogMsg("%s unanticipated exception encountered (probably disk issue)", __FUNCTION__ );
	}

	return out;
}
std::istream & operator >> ( std::istream & in, Drift & r )
{
	using AntiMatter::AppLog;	
	using std::string;

	char sBuffer[256];

	string	sAlphaMapFile;
	int		nRows		= 0;
	int		nCols		= 0;
	float	rCellWidth	= 0.0f;
	float	rCellHeight = 0.0f;
	float	rCellDepth	= 0.0f;

	try
	{
		in >> sBuffer;

		if( strcmp(sBuffer, "Drift") == 0 )
		{
			in >> sBuffer;	
			in >> nRows >> nCols;
			in >> rCellWidth >> rCellHeight >> rCellDepth;
			
			sAlphaMapFile = sBuffer;
			
			r.AlphaMapFile( sAlphaMapFile );
			r.Rows( nRows );
			r.Cols( nCols );
			r.CellWidth( rCellWidth );
			r.CellHeight( rCellHeight );
			r.CellDepth( rCellDepth );
		}
	}
	catch(...)
	{
		AppLog::Ref().LogMsg("%s unanticipated exception encountered (probably disk issue)", __FUNCTION__ );
	}

	return in;
}