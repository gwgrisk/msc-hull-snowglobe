
#pragma once
#include <glm\glm.hpp>

namespace Colours
{
	const glm::vec4 _Red	= glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	const glm::vec4 _Yellow	= glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	const glm::vec4 _Green	= glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	const glm::vec4 _Blue	= glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	const glm::vec4 _White	= glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	enum SelectColour { Red, Yellow, Green, Blue, White };

	const glm::vec4 g_ColourList[] = { 
		_Red, 
		_Yellow, 
		_Green, 
		_Blue, 
		_White 
	};
};
