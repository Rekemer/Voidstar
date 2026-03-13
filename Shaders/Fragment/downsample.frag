#version 450
layout(location = 0) out vec4 color;
layout(set = 1, binding = 0) uniform sampler2D u_Source;


layout(location = 0)  in vec2 texCoords;
void main() 
{
	// Determine texel size.
 // When rendering from one mip level to another mip of the same texture,
 // calling code should use GL_TEXTURE_BASE_LEVEL and GL_TEXTURE_MAX_LEVEL to
 // limit the textures that can be sampled from. When doing this,
 // textureSize(..., 0) will correctly use the BASE mip level.
	vec2 srcTexelSize = 1.0 / vec2(textureSize(u_Source, 0));
	float offsetX = srcTexelSize.x;
	float offsetY = srcTexelSize.y;

	// Take 13 samples around the current texel (ccc):
	//
	// etl --- ett --- etr
	// --- itl --- itr ---
	// ell --- ccc --- err
	// --- ibl --- ibr ---
	// ebl --- ebb --- ebr
	//
	// Convention:
	// - ccc: current
	// - itl: interior top left (etc)
	// - ebr: exterior bottom right (etc)

	// clang-format off
	vec4 etl = texture(u_Source, vec2(texCoords.x - 2 * offsetX, texCoords.y + 2 * offsetY));
	vec4 ett = texture(u_Source, vec2(texCoords.x, texCoords.y + 2 * offsetY));
	vec4 etr = texture(u_Source, vec2(texCoords.x + 2 * offsetX, texCoords.y + 2 * offsetY));

	vec4 ell = texture(u_Source, vec2(texCoords.x - 2 * offsetX, texCoords.y));
	vec4 ccc = texture(u_Source, vec2(texCoords.x, texCoords.y));
	vec4 err = texture(u_Source, vec2(texCoords.x + 2 * offsetX, texCoords.y));

	vec4 ebl = texture(u_Source, vec2(texCoords.x - 2 * offsetX, texCoords.y - 2 * offsetY));
	vec4 ebb = texture(u_Source, vec2(texCoords.x, texCoords.y - 2 * offsetY));
	vec4 ebr = texture(u_Source, vec2(texCoords.x + 2 * offsetX, texCoords.y - 2 * offsetY));

	vec4 itl = texture(u_Source, vec2(texCoords.x - offsetX, texCoords.y + offsetY));
	vec4 itr = texture(u_Source, vec2(texCoords.x + offsetX, texCoords.y + offsetY));
	vec4 ibl = texture(u_Source, vec2(texCoords.x - offsetX, texCoords.y - offsetY));
	vec4 ibr = texture(u_Source, vec2(texCoords.x + offsetX, texCoords.y - offsetY));
	// clang-format on

	// We want to weigh the sample amongst 5 square regions:
	// - 50% weight for 1 center region comprised of itl,itr,ibl,ibr (including
	//   the current texel).
	// - 12.5% weight each for 4 regions around the corners (for example, the
	//   top-left region of etl,ett,ell,ccc)
	//
	// However, if we just add up the samples naively, we'll double-count since
	// the regions overlap. To preserve energy, since each region is comprised of
	// exactly 5 samples, we redistribute the weights between all samples such
	// that:
	//   sample_weight =
	//       sum((region_weight / 5) for each region that sample is in)
	//
	// This yields the following distribution. The weights all add up to 1.
	color = ccc * 0.2;
	color += (itl + itr + ibl + ibr) * 0.125;
	color += (ett + ell + err + ebb) * 0.05;
	color += (etl + etr + ebl + ebr) * 0.025;

	//color = texture(u_Tex, texCoords);
	//color = vec4(1,0,0,1);
}