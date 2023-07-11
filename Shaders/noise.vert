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
layout(location = 2) out float scale ;
layout(location = 3) out vec4 worldSpacePos;
layout(location = 4) out float depth;

float norm(float val, float max,float min)
 {
     return (val- min) / (max - min);
}

vec2 GetUvs(vec2 worldSpacePos, float dimension, vec2 uv, float depth)
{
    
    float scaling = depth/dimension;
    vec2 uv_ = uv * scaling;

    vec2 wp = worldSpacePos.xy;
    float xn = norm(wp.x,dimension/2,-dimension/2);
    float yn = norm(wp.y,dimension/2,-dimension/2);
    
    vec2 newUv = vec2(xn,yn);
	float n = scaling/2;
    return newUv;
}

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
float noise ( vec2 cell, vec2 uv, float nextVertexOffset) {
    vec2 i = cell;
    vec2 f = uv;
   // i = vec2(gl_InstanceIndex,0);
    float a = random(i);
    float b = random(i + vec2(1, 0.0));
    float c = random(i + vec2(0.0, 1));
    float d = random(i + vec2(1,1));

	// smooth step function lol
    vec2 u = f * f * (3.0 - 2.0 * f);

	//return u.x;
float interpolated;
#define MYVERSION 0
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
float fbm(vec2 cell,vec2 newuv, float nextVertexOffset)
{
	// Initial values
    float value = 0.0;
    float amplitude = .5;
    float frequency = 1;

	  // Loop of octaves
    for (int i = 0; i < OCTAVES; i++)
	{
        value += amplitude * noise(cell  *frequency,newuv, nextVertexOffset);
        //st *=3.;
        amplitude *= .3;
    }
    return value;
}
float noiseInstance (vec2 uv)
{
    vec2 startQuad = floor(uv);
    vec2 endQuad = startQuad + vec2(1.,1.);

    float a = random(startQuad);
    float b = random(vec2(endQuad.x,startQuad.y));
    float c = random(endQuad);
    float d = random(vec2(startQuad.x,endQuad.y));

    vec2 quadP = uv - startQuad;
    float xLerpTop = mix(c,b,quadP.x);
    float xLerpBottom = mix(a,b,quadP.x);
    return mix(xLerpBottom,xLerpTop ,quadP.y);
}



const int permutationTableSize = 256;

// Permutation table
const int[] permutationTable = int[](151,160,137,91,90,15,                 // Pre-defined permutation table
                                        131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142, // Repeat the table twice to avoid indexing issues
                                        8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,
                                        197,62,94,252,219,203,117,35,11,32,57,177,33,88,237,149,
                                        56,87,174,20,125,136,171,168,68,175,74,165,71,134,139,48,
                                        27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,
                                        92,41,55,46,245,40,244,102,143,54,65,25,63,161,1,216,
                                        80,73,209,76,132,187,208,89,18,169,200,196,135,130,116,188,
                                        159,86,164,100,109,198,173,186,3,64,52,217,226,250,124,123,
                                        5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,
                                        58,17,182,189,28,42,223,183,170,213,119,248,152,2,44,154,
                                        163,70,221,153,101,155,167,43,172,9,129,22,39,253,19,98,
                                        108,110,79,113,224,232,178,185,112,104,218,246,97,228,251,34,
                                        242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,
                                        239,107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,
                                        50,45,127,4,150,254,138,236,205,93,222,114,67,29,24,72,
                                        243,141,128,195,78,66,215,61,156,180);  // Total size = 256

// Function to generate pseudo-random gradient vectors
vec2 randomGradient(vec2 cell)
{
    int index = permutationTable[int(cell.x) % permutationTableSize];
    index = permutationTable[(index + int(cell.y)) % permutationTableSize];
    
    float angle = float(index) * 2.0 * 3.14159265 / float(permutationTableSize);
    return vec2(cos(angle), sin(angle));
}
// Function to calculate the dot product between gradient and position vectors
float dotGridGradient(vec2 cell, vec2 vertex)
{
    vec2 gradient = randomGradient(cell);
    vec2 offset = vertex - cell;
    return dot(offset, gradient);
}

// Function to interpolate between grid points
float interpolate(float a, float b, float t)
{
    float f = t * 3.1415927;
    // smooth
    float smoot = (1.0 - cos(f)) * 0.5;
    return mix(a, b, smoot);
}


void main() 
{
	//gl_PointSize = 14.0;
    
    
// Constants
    const int gridSize = 100;
    const float cellSize = 1.0 / float(gridSize);
	//vec4 worldPos= ubo.model * vec4((in_pos+instancePos)*instanceScale,1.0);
    float dimension = 100;
    float tilesAmount  = 2.;
	vec4 worldPos= vec4((in_pos*instanceScale)+instancePos,1.0);
    vec2 newUv  = GetUvs(worldPos.xz,dimension,in_uv,texIndex);
   // float brown = fbm(in_uv*tilesAmount,newUv);
    //worldPos.y+=
	//vec4 pos =	ubo.proj * worldPos;
	worldSpacePos = worldPos;
    depth = texIndex;
	
    scale=instanceScale;
	uv = in_uv;
	color = in_color;
    vec2 f = fract(newUv*tilesAmount);
    float xn = norm(worldPos.x,dimension,-dimension);
    float yn = norm(worldPos.z,dimension,-dimension);
    vec2 uvBegin = vec2(dimension/2.,-dimension/2);
    

   // worldPos.xz += uvBegin;

// Calculate the grid cell position
    vec2 cell = floor(worldPos.xz * gridSize);
    
    // Calculate the relative position within the cell
    vec2 vertex = fract(worldPos.xz * gridSize);
    
    // Calculate the Perlin noise value
    float noiseValue = 0.0;
    
    vec2 cellOffset;




    float width = 5. / texIndex;
  
    newUv = vec2(xn,yn);

    for (float x = -width*2 ; x <= width*2; x+=width) {
        for (float y = -width*2 ; y <= width*2 ; y+=width) {
            cellOffset = vec2(float(x), float(y));
            vec2 gridVertex = cell + cellOffset;
            float dotProduct = dotGridGradient(gridVertex, worldPos.xz);
            noiseValue += dotProduct * interpolate(1.0 - length(cellOffset), 1.0, newUv.x * newUv.y);
            noiseValue = dotProduct;
        }
    }

   // noiseValue *= 0.4;


    //newUv = worldPos.xz - uvBegin;
    //float noiseValue = noiseInstance(newUv);
    vec2 grid =newUv*1.;
    
    noiseValue = fbm(cell,newUv,width);
    worldPos.y += abs(noiseValue)*13.;
	color.xyz =vec3(noiseValue,noiseValue,noiseValue) ;
    //color.xyz = vec3(newUv,0.);
    //color.xyz= vec3( randomGradient(cell),0.);
	//vec4 pos =	ubo.proj *  ubo.view * worldPos;
	vec4 pos =	 worldPos;
	gl_Position = pos;
}