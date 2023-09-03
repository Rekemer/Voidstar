#version 450



layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D u_Noise;

vec2 quadNumber= vec2(50,20);
float billinear(vec2 scaledUv)
{
    vec2 indexUv = floor(scaledUv);
    vec2 f = fract(scaledUv);
    float offset = 10;
    float indexLeftLen = length(((quadNumber)-(indexUv))) - offset;
    float indexRightLen = length(((quadNumber+vec2(1,0))-(indexUv)))-offset;
    float indexBottomLen = length(((quadNumber+vec2(0,1))-(indexUv)))-offset;
    float indexUpLen = length(((quadNumber+vec2(1,1))-(indexUv)))-offset;
      // Four corners in 2D of a tile
   // float a = random(i);
    //float b = random(i + vec2(1.0, 0.0));
    //float c = random(i + vec2(0.0, 1.0));
    //float d = random(i + vec2(1.0, 1.0));

	// smooth step function lol
    vec2 u = f * f * (3.0 - 2.0 * f);

    float interpolated;


	float interpolatedX = mix(indexLeftLen, indexRightLen, u.x);
	float interpolatedY = mix(indexBottomLen, indexUpLen, u.x);
	interpolated = mix(interpolatedX , interpolatedY , u.y);
    return interpolated;
}

void main() 
{


    float scale = 100;
    vec2 scaledUv = uv*scale;
    vec2 indexUv = floor(scaledUv);
    vec2 fractedUv = fract(scaledUv );


    float ratio = 1.f/2.f;
    

    vec4 noise = texture(u_Noise,uv);  
    float maxLen = length(vec2(100,100));
    float len = length(((quadNumber)-(indexUv)))/(maxLen*1.);
    float indexLen = length(((quadNumber)-(indexUv)));
    indexLen -=10;
    float invertedIndexLen = 1-indexLen;
    float invertedLen = 1-len;
    //len+=noise.x;        
    vec4 selectedQuad = smoothstep(vec4(0,0,0,1),vec4(1,1,1,1),vec4(len,len,len,1));


    

    float bColor = billinear(scaledUv);
	outColor = vec4(selectedQuad );
//	outColor = vec4(uv,0,1);
	outColor = vec4(invertedIndexLen,invertedIndexLen,invertedIndexLen,1);
	//outColor = vec4(len,len,len,1);
	//outColor = vec4(invertedLen,invertedLen,invertedLen,1);
    outColor = vec4(bColor,bColor,bColor,1);

}