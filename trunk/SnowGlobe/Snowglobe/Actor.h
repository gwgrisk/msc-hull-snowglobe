
#pragma once

#include <GXBase.h>
#include "IGraphNode.h"

#include "WavefrontObj.h"

#include "VertexClass.h"
#include "Vbo.h"
#include "EffectMgr.h"
#include "Texture.h"
#include "Material.h"

#include <glm\glm.hpp>
#include <string>
#include <vector>
#include <map>

class CustomVertex;

class Actor :	public glex,
				public IGraphNode
{
private:
	bool						m_bInitialized;

	Effect*						m_pEffect;
	Vbo<CustomVertex>*			m_pVbo;
	GLuint						m_nVaoId;

	std::vector<Texture>		m_textures;
	std::vector<Material>		m_materials;

	WavefrontObj				_obj;

private:
	bool InitByWavefrontObj( const std::string & s );
	bool StealWavefrontData();
	bool InitializeVbo();

public:

	Actor();
	Actor( const std::string & sWavefrontObjFile );
	Actor( const Actor & r );
	~Actor();

	// IGraphNode
	HRESULT Update( const float & rSecsDelta )	{ return E_NOTIMPL; }
	HRESULT PreRender()							{ return E_NOTIMPL; }
	HRESULT Render()							{ return E_NOTIMPL; }
	HRESULT PostRender()						{ return E_NOTIMPL; }
	HRESULT DrawItem()							{ return E_NOTIMPL; }
};

/*
An actor can be defined from the scene.xml configuration file.
Here's an example of how the Actor class is specified:

<Actor name="house" type="obj" file="house.obj" effect="textured-phong">
	<parent-object name="terrain" />
	<world-pos x=0 y=0 z=0 />
	<orientation />
	<scale />
</Actor>
*/