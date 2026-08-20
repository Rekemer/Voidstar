#pragma once
#include "glm.hpp"
#include <unordered_map>
#include "Types.h"
#include "Submission.h"


namespace std {
	template<>
	struct hash<std::pair<char,char>>
	{
		size_t operator()(const std::pair<char, char>& k) const
		{
			auto h1 = k.first;
			auto h2 = k.second;
			return h1 ^ (h2 << 1);
		}
	};
}

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
		// most modern fonts don't support kern table anymore
		// instead they use OpenType GPOS 
		std::unordered_map<std::pair<char, char>, int> Kerning;
	};
}