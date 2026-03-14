#version 450
layout(location = 0) out vec4 color;
layout(set = 1, binding = 0) uniform sampler2D u_SourceLow;
layout(set = 1, binding = 1) uniform sampler2D u_SourceHigh;


layout(location = 0)  in vec2 texCoords;
void main() 
{
	// Unlike when downsampling, we use a fixed radius in UV space rather than
 // basing it off the texel size of the target mip.
	vec2 texelSize = 1.0 / vec2(textureSize(u_SourceLow, 0));
	float radius = 1.6;
    float offsetX = texelSize.x * radius; 
    float offsetY = texelSize.y * radius;

	// Take 9 samples around the current texel (cc):
	//
	// tl tt tr
	// ll cc rr
	// bl bb br
	//
	// Convention:
	// - cc: current
	// - tl: top left (etc)
	// - br: bottom right (etc)

	// clang-format off
	vec4 tl = texture(u_SourceLow, vec2(texCoords.x - offsetX, texCoords.y + offsetY));
	vec4 tt = texture(u_SourceLow, vec2(texCoords.x, texCoords.y + offsetY));
	vec4 tr = texture(u_SourceLow, vec2(texCoords.x + offsetX, texCoords.y + offsetY));

	vec4 ll = texture(u_SourceLow, vec2(texCoords.x - offsetX, texCoords.y));
	vec4 cc = texture(u_SourceLow, vec2(texCoords.x, texCoords.y));
	vec4 rr = texture(u_SourceLow, vec2(texCoords.x + offsetX, texCoords.y));

	vec4 bl = texture(u_SourceLow, vec2(texCoords.x - offsetX, texCoords.y - offsetY));
	vec4 bb = texture(u_SourceLow, vec2(texCoords.x, texCoords.y - offsetY));
	vec4 br = texture(u_SourceLow, vec2(texCoords.x + offsetX, texCoords.y - offsetY));
	// clang-format on

	// Apply weighted distribution by using a 3x3 tent filter:
	// | 1 2 1 |
	// | 2 4 2 | * 1/16
	// | 1 2 1 |
	vec4 upsampleBlur = cc* 4.0;
    upsampleBlur += (tt + ll + rr + bb) * 2.0;
    upsampleBlur += (tl + tr + bl + br) * 1.0;
    upsampleBlur *= (1.0 / 16.0);

	float scatter = 0.7;
	upsampleBlur *= scatter;
	vec3 detail = texture(u_SourceHigh, texCoords).rgb; 
	color.xyz = upsampleBlur.xyz + detail.xyz;
	
}