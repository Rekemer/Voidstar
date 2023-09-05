#version 450



layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D u_Noise;

float scale = 100;
vec2 quadNumber= vec2(scale/2,scale/2);

float lerp(float a, float b, float t)
{
   
    return min(a,b) + t*(max(a,b)-a);
}

float catRom(float x, float a,float a1,float a2,float a3)
{
    return pow(x,3) * a3 +pow(x,2) * a2 +pow(x,1) * a1 +a;
}

vec2 points[4] = {quadNumber + vec2(-1,0),quadNumber + vec2(0,1),quadNumber + vec2(1,0),quadNumber + vec2(0,1)};

float getSquare(vec2 point)
{
    float res = 0;
     for (int j =0; j< 4; j++)
    {
        res = length(points[j] - point );
    }
    return res;
}

//https://danceswithcode.net/engineeringnotes/interpolation/interpolation.html
float bicubic(vec2 scaledUv)
{
    vec2 indexUv = floor(scaledUv);
    vec2 f = fract(scaledUv);
    f = vec2(1)-f;
    float offset = 20;
    float splineX[4][4];

    for (int i =0; i< 4; i++)
    {
        for (int j =0; j< 4; j++)
        {
            splineX[i][j] = length(quadNumber - indexUv + vec2(i-1,j-1)) - offset;
           // splineX[i][j] = length(points[j] - indexUv ) - offset;
        }
    }

    float splineY[4];
    for (int i=0; i <4; i++)
    {
        float ya = splineX[0][i];
        float yb = splineX[1][i];
        float yc = splineX[2][i];
        float yd = splineX[3][i];
        float a0 = yb;
        float a1 = 0.5 *(-ya+yc);
        float a2 = 0.5 * (2*ya-5*yb+4*yc -yd);   
        float a3 = 0.5 * (-ya+3*yb-3*yc +yd);   
        splineY[i] = catRom(f.x,a0,a1,a2,a3);
    }

    float ya = splineY[0];
    float yb = splineY[1];
    float yc = splineY[2];
    float yd = splineY[3];
    float a0 = yb;
    float a1 = 0.5 *(-ya+yc);
    float a2 = 0.5 * (2*ya-5*yb+4*yc -yd);   
    float a3 = 0.5 * (-ya+3*yb-3*yc +yd);   
    float res = catRom(f.y,a0,a1,a2,a3);
    return res;

}

float billinear(vec2 scaledUv)
{
    vec2 indexUv = floor(scaledUv);
    //indexUv = vec2(0,0);
    vec2 f = fract(scaledUv);
    // I don't know why inverted is correct one >:-( and why is offset vec2 supposed to be with - 
    //if we dont invert direction of interpolation

    f = vec2(1)-f;
    float offset = 10;
        //float indexLeftLen = length(((quadNumber)-(indexUv))) - offset;
        //float indexRightLen = length(((quadNumber+vec2(1,0))-(indexUv)))-offset;
    float current = length(quadNumber - indexUv + vec2(0.0, 0.0)) - offset;
    float bottomRight = length(quadNumber - indexUv + vec2(1.0, 0.0)) - offset;
    float topLeft = length(quadNumber - indexUv +vec2(0.0, 1.0)) - offset;
    float topRight = length(quadNumber - indexUv + vec2(1.0, 1.0)) - offset;


    
    vec2 u = f * f * (3.0 - 2.0 * f);

    float interpolated;

    //x×(1−a)+y×a
    // x + a(y-x)
   

	float interpolatedX = mix(current, bottomRight,u.x);
	float interpolatedY = mix(topLeft, topRight, u.x);
	interpolated = mix(interpolatedX , interpolatedY , u.y);
    return interpolated ;
}

void main() 
{


    
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
    float bbColor = bicubic(scaledUv);
	outColor = vec4(selectedQuad );
	outColor = vec4(vec2(1)-fractedUv,0,1);
	//outColor = vec4(invertedIndexLen,invertedIndexLen,invertedIndexLen,1);
	//outColor = vec4(len,len,len,1);
	//outColor = vec4(invertedLen,invertedLen,invertedLen,1);
    outColor = vec4(bColor,bColor,bColor,1);
    float inverBbColor = 1 - bbColor;
    float inverBColor = 1 - bColor;

    float res = noise.x*inverBbColor;
     //res = inverBbColor;
    // res = getSquare(indexUv);
    outColor = vec4(res,res,res,res-0.15);

}