#pragma once
#include "glm.hpp"
struct Character {
	// should not be here
	static int lineSpacing;
	glm::vec2 minUv;
	glm::vec2 maxUv;
	glm::ivec2   Size;       // Size of glyph
	glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Offset to advance to next glyph
};
inline int Character::lineSpacing = 0;