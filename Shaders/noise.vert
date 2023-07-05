#version 450

// vulkan NDC:	x: -1(left), 1(right)
//				y: -1(top), 1(bottom)


layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

// vertexAttributes
layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec2 in_uv;

// Instanced attributes
layout (location = 3) in vec3 instancePos;
layout (location = 4) in float instanceScale;
layout (location = 5) in int texIndex;

vec2 positions[3] = vec2[](
	vec2(0.0, -0.5),
	vec2(0.5, 0.5),
	vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0)
);

layout(location = 0) out vec2 uv ;
layout(location = 1) out vec4 color ;


vec4 random(vec4 st)
{
	float dotProduct1 = dot(st, vec4(127.1, 311.7, 23423.1, 98.2));
    float dotProduct2 = dot(st, vec4(269.5, 183.3, 21.2, 65.1));
    float dotProduct3 = dot(st, vec4(129.5, 383.3, 11.2, 5.1));
    float dotProduct4 = dot(st, vec4(26.5, 83.3, 2.2, 15.1));

    vec4 sinResult = sin(vec4(dotProduct1, dotProduct2,dotProduct3,dotProduct4));
    vec4 multiplied = sinResult * 43758.5453;

    return fract(multiplied);
}
float random (vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}
float noise (vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

	// smooth step function lol
    vec2 u = f * f * (3.0 - 2.0 * f);

	//return u.x;
float interpolated;
#define MYVERSION 1
	#ifdef MYVERSION 
	float interpolatedX = mix(a, b, u.x);
	float interpolatedY = mix(c, d, u.x);
	interpolated = mix(interpolatedX , interpolatedY , u.y);
	#else
 	interpolated = mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
	#endif

    return interpolated;
}

#define OCTAVES 37
float fbm(vec2 st)
{
	// Initial values
    float value = 0.0;
    float amplitude = .5;
    float frequency = 0.;

	  // Loop of octaves
    for (int i = 0; i < OCTAVES; i++)
	{
        value += amplitude * noise(st);
        st *=3.;
        amplitude *= .3;
    }
    return value;
}

void main() 
{
	//gl_PointSize = 14.0;
	vec4 worldPos= ubo.model * vec4((in_pos*instanceScale)+instancePos,1.0);
	worldPos.y+=fbm(in_uv*3.0);
	vec4 pos =	ubo.proj *  ubo.view * worldPos;
	
   
	gl_Position = pos;
	uv = in_uv;
	uv = vec2(in_uv.x,in_uv.y);
	color = in_color;
	color.x = fbm(in_uv*3.0);
		
}