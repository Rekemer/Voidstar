#pragma once
#include "glm.hpp"
#include <unordered_map>
#include "Types.h"
#include "Submission.h"
namespace Voidstar
{
	class Image;
	struct VOIDSTAR_API Character {
		glm::vec2 minUv;
		glm::vec2 maxUv;
		glm::ivec2   Size;       // Size of glyph
		glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
		unsigned int Advance;    // Offset to advance to next glyph
	};

	struct Font
	{
		TextureHandle Atlas;
		int LineSpacing;
		std::unordered_map<unsigned char, Character> Characters;
	};
}