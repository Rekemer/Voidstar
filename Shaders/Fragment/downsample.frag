#version 450
layout(location = 0) out vec4 color;
layout(set = 1, binding = 0) uniform sampler2D u_Source;


layout(location = 0)  in vec2 texCoords;


// New Helper Function for Karis Average
// This prevents "fireflies" and helps the glow feel smoother
vec3 KarisWeight(vec3 c) {
    float luma = dot(c, vec3(0.2126, 0.7152, 0.0722));
    return c / (1.0 + luma);
}

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


	// Instead of raw colors, we weight them
    vec3 group1 = (etl.rgb + ett.rgb + ell.rgb + ccc.rgb) * 0.25;
    vec3 group2 = (ett.rgb + etr.rgb + ccc.rgb + err.rgb) * 0.25;
    vec3 group3 = (ell.rgb + ccc.rgb + ebl.rgb + ebb.rgb) * 0.25;
    vec3 group4 = (ccc.rgb + err.rgb + ebb.rgb + ebr.rgb) * 0.25;
    vec3 group5 = (itl.rgb + itr.rgb + ibl.rgb + ibr.rgb) * 0.25;

    // Apply Karis Average to each group
    vec3 weightedSum = (KarisWeight(group1) + KarisWeight(group2) + 
                        KarisWeight(group3) + KarisWeight(group4) + 
                        KarisWeight(group5)) * 0.2;

    // Inverse the weighting to bring it back to HDR range
    float finalLuma = dot(weightedSum, vec3(0.2126, 0.7152, 0.0722));
    color.rgb = weightedSum / (1.0 - finalLuma);
    color.a = 1.0;

	//color = texture(u_Tex, texCoords);
	//color = vec4(1,0,0,1);
}