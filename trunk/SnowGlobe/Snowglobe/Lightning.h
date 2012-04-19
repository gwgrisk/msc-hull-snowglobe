#pragma once

#include <GXBase.h>
#include "IGraphNode.h"

#include "Quad.h"
#include "Tri.h"
#include "Texture.h"
#include "Material.h"
#include <glm\glm.hpp>

#include <vector>
#include <string>
#include <fstream>

class ShaderProgram;
class SceneGraph;

class Lightning :	public IGraphNode,
					public glex
{
private:

public:
	Lightning()
	{
	}

	~Lightning()
	{
	}
};