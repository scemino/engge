#include "Shaders.hpp"

namespace ng {
const char *Shaders::vertexShader =
    R"(#version 100
precision mediump float;
attribute vec2 a_position;
attribute vec4 a_color;
attribute vec2 a_texCoords;

uniform mat3 u_transform;
varying vec2 v_texCoords;
varying vec4 v_color;

void main(void) {
  v_color = a_color;
  v_texCoords = a_texCoords;

  vec3 worldPosition = vec3(a_position, 1);
  vec3 normalizedPosition = worldPosition * u_transform;
  gl_Position = vec4(normalizedPosition.xy, 0, 1);
})";

const char *Shaders::bwFragmentShader =
    R"(#version 100
precision mediump float;
varying vec2 v_texCoords;
varying vec4 v_color;
uniform sampler2D u_texture;
void main()
{
  vec4 texColor = texture2D(u_texture, v_texCoords);
  vec4 col = v_color * texColor;
  float gray = dot(col.xyz, vec3(0.299, 0.587, 0.114));
  gl_FragColor = vec4(gray, gray, gray, col.a);
})";

const char *Shaders::egaFragmenShader =
    R"(#version 100
#ifdef GL_ES
precision highp float;
#endif

varying vec2 v_texCoords;
varying vec4 v_color;
uniform sampler2D u_texture;

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

float dist_sq(vec3 a, vec3 b)
{
    vec3 delta = a - b;
    return dot(delta, delta);
}

vec3 nearest_rgbi (vec3 orig)
{
    vec3 original = rgb2hsv(orig);
    float min_dst = 4.0;
    vec3 ret = vec3(1,1,1);

    vec3 pal = vec3(0, 0, 0);
    float dst = dist_sq(original, pal);
    if(dst < min_dst) { min_dst = dst; ret = vec3(0.0,     0.0,     0.0); }

    pal = vec3(0.66667, 1, 0.66667);
    dst = dist_sq(original, pal);
    if(dst < min_dst) { min_dst = dst; ret = vec3(0.0,     0.0,     0.66667); }

    pal = vec3(0.33333, 1, 0.66667);
    dst = dist_sq(original, pal);
    if(dst < min_dst) { min_dst = dst; ret = vec3(0.0,     0.66667, 0.0); }

    pal = vec3(0.5, 1, 0.66667);
    dst = dist_sq(original, pal);
    if(dst < min_dst) { min_dst = dst; ret = vec3(0.0,     0.66667, 0.66667); }

    pal = vec3(0, 1, 0.66667);
    dst = dist_sq(original, pal);
    if(dst < min_dst) { min_dst = dst; ret = vec3(0.66667, 0.0,     0.0); }

    pal = vec3(0.83333, 1, 0.66667);
    dst = dist_sq(original, pal);
    if(dst < min_dst) { min_dst = dst; ret = vec3(0.66667, 0.0,     0.66667); }

    pal = vec3(0.083333, 1, 0.66667);
    dst = dist_sq(original, pal);
    if(dst < min_dst) { min_dst = dst; ret = vec3(0.66667, 0.33333, 0.0); }

    pal = vec3(0, 0, 0.666667);
    dst = dist_sq(original, pal);
    if(dst < min_dst) { min_dst = dst; ret = vec3(0.66667, 0.66667, 0.66667); }

    pal = vec3(0, 0, 0.333333);
    dst = dist_sq(original, pal);
    if(dst < min_dst) { min_dst = dst; ret = vec3(0.33333, 0.33333, 0.33333); }

    pal = vec3(0.66667, 0.66667, 1);
    dst = dist_sq(original, pal);
    if(dst < min_dst) { min_dst = dst; ret = vec3(0.33333, 0.33333, 1.0); }

    pal = vec3(0.33333, 0.66667, 1);
    dst = dist_sq(original, pal);
    if(dst < min_dst) { min_dst = dst; ret = vec3(0.33333, 1.0,     0.33333); }

    pal = vec3(0.5, 0.66667, 1);
    dst = dist_sq(original, pal);
    if(dst < min_dst) { min_dst = dst; ret = vec3(0.33333, 1.0,     1.0); }

    pal = vec3(0, 0.66667, 1);
    dst = dist_sq(original, pal);
    if(dst < min_dst) { min_dst = dst; ret = vec3(1.0,     0.33333, 0.33333); }

    pal = vec3(0.83333, 0.66667, 1);
    dst = dist_sq(original, pal);
    if(dst < min_dst) { min_dst = dst; ret = vec3(1.0,     0.33333, 1.0); }

    pal = vec3(0.16666, 0.66667, 1);
    dst = dist_sq(original, pal);
    if(dst < min_dst) { min_dst = dst; ret = vec3(1.0,     1.0,     0.33333); }

    return ret;
}

void main()
{
   vec4 texColor = texture2D( u_texture, v_texCoords );
   vec4 srcCol = v_color * texColor;
   vec3 newCol = nearest_rgbi(srcCol.rgb);
   gl_FragColor = vec4(newCol, srcCol.a);
})";

const char *Shaders::fadeFragmentShader =
    R"(#version 100
#ifdef GL_ES
precision highp float;
#endif

varying vec2 v_texCoords;
varying vec4 v_color;
uniform sampler2D u_texture;
uniform sampler2D u_texture2;

uniform float u_timer;
uniform float u_fade;
uniform int u_fadeToSep;
uniform float u_movement;

void main()
{
   const float RADIUS = 0.75;
   const float SOFTNESS = 0.45;
   const float ScratchValue = 0.3;
   vec2 uv = v_texCoords;
   float pi2 = (3.142*2.0);
   float intervals = 4.0;
   uv.x += sin((u_timer+uv.y)*(pi2*intervals))*u_movement;
   vec4 texColor1 = v_color * texture2D( u_texture, uv);
   vec4 texColor2 = v_color * texture2D( u_texture2, uv);
   if (u_fadeToSep!=0) {
       float gray = dot(texColor2.rgb, vec3(0.299, 0.587, 0.114));
       vec2 dist = vec2(uv.x - 0.5, uv.y - 0.5);
       vec3 sepiaColor = vec3(gray,gray,gray) * vec3(0.9, 0.8, 0.6);
       float len = dot(dist,dist);
       float vignette = smoothstep(RADIUS, RADIUS-SOFTNESS, len);
       vec3 sep = mix(texColor2.rgb, sepiaColor, 0.80) * vignette;
       gl_FragColor.rgb = (texColor1.rgb*(1.0-u_fade)) + (sep*u_fade);
   }
   else {
       gl_FragColor.rgb = (texColor1.rgb*(1.0-u_fade)) + (texColor2.rgb*u_fade);
   }
   gl_FragColor.a = 1.0;
})";

const char *Shaders::ghostFragmentShader =
    R"(#version 100
// Work in progress ghost shader.. Too over the top at the moment, it'll make you sick.

#ifdef GL_ES
precision highp float;
#endif

uniform float iGlobalTime;
uniform float iFade;
uniform float wobbleIntensity;
uniform vec3 shadows;
uniform vec3 midtones;
uniform vec3 highlights;
uniform sampler2D u_texture;

varying vec2 v_texCoords;

const float speed = 0.1;
const float emboss = 0.70;
const float intensity = 0.6;
const int steps = 4;
const float frequency = 9.0;


float colour(vec2 coord) {
    float col = 0.0;

    float timeSpeed = iGlobalTime*speed;
    vec2 adjc = coord;
    adjc.x += timeSpeed;   //adjc0.x += fcos*timeSpeed;
    float sum0 = cos( adjc.x*frequency)*intensity;
    col += sum0;

    adjc = coord;
    float fcos = 0.623489797;
    float fsin = 0.781831503;
    adjc.x += fcos*timeSpeed;
    adjc.y -= fsin*timeSpeed;
    float sum1 = cos( (adjc.x*fcos - adjc.y*fsin)*frequency)*intensity;
    col += sum1;

    adjc = coord;
    fcos = -0.900968909;
    fsin = 0.433883607;
    adjc.x += fcos*timeSpeed;
    adjc.y -= fsin*timeSpeed;
    col += cos( (adjc.x*fcos - adjc.y*fsin)*frequency)*intensity;

    // do same in reverse.
    col += sum1;
    col += sum0;

    return cos(col);
}

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float rand(vec2 Input)
{
    float dt= dot(Input, vec2(12.9898,78.233));
    float sn= mod(dt,3.14);
    return fract(sin(sn)*43758.5453123);
}

float color_balance( float col, float l, vec3 change )
{
    // NOTE: change = (shadow, midtones, highlights)

    float sup = 83.0;    // shadow upper bounds
    float mup = 166.0;    // midtones upper bounds

    float value = col*255.0;
    l = l * 100.0;

    if (l < sup)
    {
        // shadow
        float f = (sup - l + 1.0)/(sup + 1.0);
        value += change.x * f;
    }
    else if (l < mup)
    {
        // midtone
        float mrange = (mup - sup)/2.0;
        float mid = mrange + sup;
        float diff = mid - l;
        diff = -diff;
        if (diff < 0.0)
        {
            float f = 1.0 - (diff + 1.0) / (mrange + 1.0);
            value += change.y * f;
        }
    }
    else
    {
        // highlight
        float f = (l - mup + 1.0)/(255.0 - mup + 1.0);
        value += change.z * f;
    }
    value = min(255.0,max(0.0,value));
    return value/255.0;
}

vec2 rgb2cv(vec3 RGB)
{
    vec4 P = (RGB.g < RGB.b) ? vec4(RGB.bg, -1.0, 2.0/3.0) : vec4(RGB.gb, 0.0, -1.0/3.0);
    vec4 Q = (RGB.r < P.x) ? vec4(P.xyw, RGB.r) : vec4(RGB.r, P.yzx);
    float C = Q.x - min(Q.w, Q.y);
    return vec2(C, Q.x);
}

float rgbToLuminance(vec3 RGB)
{
    float cMax = max( max(RGB.x, RGB.y), RGB.z);
    float cMin = min( min(RGB.x, RGB.y), RGB.z);

    return (cMax+cMin) * 0.5;
}


void main(void)
{
    vec2 c1 = v_texCoords;
   float cc1 = colour(c1);
    vec2 offset;

    c1.x += (0.001 *wobbleIntensity);      // appx 12 pixels horizontal
    offset.x = emboss*(cc1-colour(c1));

    c1.x = v_texCoords.x;
    c1.y += (0.002*wobbleIntensity);      // appx 12 pixels verticle
    offset.y = emboss*(cc1-colour(c1));

    // TODO: The effect should be centered around Franklyns position in the room, not the center
    //if ( emitFromCenter == 1)
    {
        vec2 center = vec2(0.5, 0.5);
        float distToCenter = distance(center, v_texCoords);
        offset *= distToCenter * 2.0;
    }

    c1 = v_texCoords;
    c1 += ( offset * iFade );

    vec3 col = vec3(0,0,0);
    if ( c1.x >= 0.0 && c1.x < (1.0-0.003125) )
    {
        col = texture2D(u_texture,c1).rgb;
        float intensity = rgbToLuminance(col);  //(col.r + col.g + col.b) * 0.333333333;

        // Exponential Shadows
        float shadowsBleed = 1.0 - intensity;
        shadowsBleed *= shadowsBleed;
        shadowsBleed *= shadowsBleed;

        // Exponential midtones
        float midtonesBleed = 1.0 - abs(-1.0 + intensity * 2.0);
        midtonesBleed *= midtonesBleed;
        midtonesBleed *= midtonesBleed;

        // Exponential Hilights
        float hilightsBleed = intensity;
        hilightsBleed *= hilightsBleed;
        hilightsBleed *= hilightsBleed;

        vec3 colorization = col.rgb + shadows * shadowsBleed +
        midtones * midtonesBleed +
        highlights * hilightsBleed;

        colorization = mix(col, colorization,iFade);

        //col = lerp(col, colorization, _Amount);
        col =  min(vec3(1.0),max(vec3(0.0),colorization));
    }
    gl_FragColor = vec4(col,1);
})";

const char *Shaders::sepiaFragmentShader =
    R"(#version 100
#ifdef GL_ES
precision highp float;
#endif

varying vec4 v_color;
uniform sampler2D u_texture;
uniform float sepiaFlicker;
uniform float RandomValue[5];
uniform float TimeLapse;
varying vec2 v_texCoords;

vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289(((x*34.0)+1.0)*x); }
float snoise (vec2 v)
{
    const vec4 C = vec4(0.211324865405187,   // (3.0-sqrt(3.0))/6.0
                        0.366025403784439,   // 0.5*(sqrt(3.0)-1.0)
                        -0.577350269189626,   // -1.0 + 2.0 * C.x
                        0.024390243902439);   // 1.0 / 41.0

    // First corner
    vec2 i  = floor(v + dot(v, C.yy) );
    vec2 x0 = v -   i + dot(i, C.xx);

    // Other corners
    vec2 i1;
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;

    // Permutations
    i = mod289(i); // Avoid truncation effects in permutation
    vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
                     + i.x + vec3(0.0, i1.x, 1.0 ));

    vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
    m = m*m ;
    m = m*m ;

    // Gradients: 41 points uniformly over a line, mapped onto a diamond.
    // The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;

    // Normalise gradients implicitly by scaling m
    // Approximation of: m *= inversesqrt( a0*a0 + h*h );
    m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

    // Compute final noise value at P
    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}


void main(void)
{
    const float RADIUS = 0.75;
    const float SOFTNESS = 0.45;
    const float ScratchValue = 0.3;

    vec4 texColor = texture2D( u_texture, v_texCoords);
    vec4 col = v_color * texColor;
    float gray = dot(col.rgb, vec3(0.299, 0.587, 0.114));
    vec2 dist = vec2(v_texCoords.x - 0.5, v_texCoords.y - 0.5);
    vec3 sepiaColor = vec3(gray) * vec3(0.9, 0.8, 0.6);   //vec3(1.2, 1.0, 0.8);
    float len = dot(dist,dist);
    float vignette = smoothstep(RADIUS, RADIUS-SOFTNESS, len);
    //   float vignette = (1.0 - len);
    col.rgb = mix(col.rgb, sepiaColor, 0.80) * vignette * sepiaFlicker;  // Want to keep SOME of the original color, so only use 80% sepia
    //   col.rgb = vec3( vignette ) * sepiaFlicker;

    for ( int i = 0; i < 1; i ++)
    {
        if ( RandomValue[i] < ScratchValue )
        {
            // Pick a random spot to show scratches
            float dist = 1.0 / ScratchValue;
            float d = distance(v_texCoords, vec2(RandomValue[i] * dist, RandomValue[i] * dist));
            if ( d < 0.4 )
            {
                // Generate the scratch
                float xPeriod = 8.0;
                float yPeriod = 1.0;
                float pi = 3.141592;
                float phase = TimeLapse;
                float turbulence = snoise(v_texCoords * 2.5);
                float vScratch = 0.5 + (sin(((v_texCoords.x * xPeriod + v_texCoords.y * yPeriod + turbulence)) * pi + phase) * 0.5);
                vScratch = clamp((vScratch * 10000.0) + 0.35, 0.0, 1.0);

                col.rgb *= vScratch;
            }
        }
    }
    gl_FragColor = col;
}
)";

const char *Shaders::vhsFragmentShader =
    R"(#version 100
#ifdef GL_ES
precision highp float;
#endif

uniform float iGlobalTime;
uniform float iNoiseThreshold;
uniform sampler2D u_texture;

varying vec2 v_texCoords;

float hash( float Input )
{
    return fract(sin(Input)*43758.5453123);
}

float rand(vec2 Input)
{
    float dt= dot(Input, vec2(12.9898,78.233));
    float sn= mod(dt,3.14);
    return hash(sn);
}

// 1d noise function
float noise1d( float x )
{
    float p = floor(x);
    float f = fract(x);
    f = f*f*(3.0-2.0*f);
    float n = p + 57.0 + 113.0;

    return mix( hash(n), hash(n+1.0),f);
}

// 2d noise function
float noise2d( vec2 x )
{
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*57.0 + 113.0;

    float a = mix( hash(n),      hash(n+ 1.0),f.x);
    float b = mix( hash(n+57.0), hash(n+58.0),f.x);

    return mix(a,b,f.y);
}

//tape noise
float tapenoise()
{
    vec2 iResolution = vec2(1280,720);
    float linesN = 250.0; //fields per seconds
    float one_y = iResolution.y / linesN; //field line
    vec2 p = floor(v_texCoords*iResolution.xy/one_y)*one_y;

    float s = iGlobalTime;
    vec2 fP = vec2(p.x+s, p.y);

    float v = 0.3 + ( noise1d( p.y*.01 +s ) * noise1d( p.y*.011+1000.0+s ) * noise1d( p.y*.51+421.0+s ) * noise2d( fP * 100.0 ) );

    // if ( v < iNoiseThreshold ) v = 0.0;

    // Same as above, without if
    v *= step(iNoiseThreshold, v);

    return v;
}


void main(void)
{
    // Apply a vhs-style distortion.
    const float magnitude = 0.000003;
    vec2 colRuv = vec2(v_texCoords.x + (rand(vec2(iGlobalTime*0.03,v_texCoords.y*0.42)) * 0.001 + sin(rand(vec2(iGlobalTime*0.2, v_texCoords.y)))*magnitude), v_texCoords.y);
    vec2 colGuv = vec2(v_texCoords.x + (rand(vec2(iGlobalTime*0.004,v_texCoords.y*0.002)) * 0.004 + sin(iGlobalTime*9.0)*magnitude), v_texCoords.y);
    vec2 colBuv = v_texCoords;

    // Now make colours distort around edge
    const float RADIUS = 0.85;
    const float SOFTNESS = 0.65;

    vec2 position = v_texCoords / vec2(0.6,1.0) - vec2(0.8,0.5) ;
    float len = length(position);
    float vignette = 1.0-smoothstep(RADIUS, RADIUS-SOFTNESS, len);

    float angle = dot(position, v_texCoords) / (length(position)*length(v_texCoords));
    vec2 screenPos = vec2(1.0)-( v_texCoords );
    vec3 video;

    video.r = texture2D(u_texture, colRuv-(vignette*(position*(len*0.015)))).r;
    video.g = texture2D(u_texture, colGuv).g;
    video.b = texture2D(u_texture, colBuv+(vignette*(position*(len*0.015)))).b;

    // Now add the tape noise
    video += vec3( tapenoise() );

    gl_FragColor = vec4(video,1.0);
}
)";

const char *Shaders::verbVertexShaderCode =
    R"(#version 100
precision mediump float;
attribute vec2 a_position;
attribute vec4 a_color;
attribute vec2 a_texCoords;

uniform vec4 u_shadowColor;
uniform vec4 u_normalColor;
uniform vec4 u_highlightColor;
uniform vec2 u_ranges;
uniform mat3 u_transform;

varying vec4 v_color;
varying vec2 v_texCoords;
varying vec4 v_shadowColor;
varying vec4 v_normalColor;
varying vec4 v_highlightColor;
varying vec2 v_ranges;

void main(void) {
  v_color = a_color;
  v_texCoords = a_texCoords;
  v_shadowColor = u_shadowColor;
  v_normalColor = u_normalColor;
  v_highlightColor = u_highlightColor;
  v_ranges = u_ranges;

  vec3 worldPosition = vec3(a_position, 1);
  vec3 normalizedPosition = worldPosition * u_transform;
  gl_Position = vec4(normalizedPosition.xy, 0, 1);
})";

const char *Shaders::verbFragmentShaderCode =
    R"(#version 100
#ifdef GL_ES
precision highp float;
#endif

varying vec4 v_color;
varying vec2 v_texCoords;
varying vec4 v_shadowColor;
varying vec4 v_normalColor;
varying vec4 v_highlightColor;
varying vec2 v_ranges;
uniform sampler2D u_texture;

void main(void)
{
    float shadows = v_ranges.x;
    float highlights = v_ranges.y;

    vec4 texColor = texture2D(u_texture, v_texCoords);

    if ( texColor.g <= shadows)
    {
        texColor*=v_shadowColor;
    }
    else if (texColor.g >= highlights)
    {
        texColor*=v_highlightColor;
    }
    else
    {
        texColor*=v_normalColor;
    }
    texColor *= v_color;
    gl_FragColor = texColor;
}
)";

}