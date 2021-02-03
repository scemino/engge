#include <squirrel.h>
#include <glm/gtc/epsilon.hpp>
#include "engge/Engine/Engine.hpp"
#include "engge/Engine/ActorIconSlot.hpp"
#include "engge/Engine/ActorIcons.hpp"
#include "engge/Engine/Camera.hpp"
#include "engge/Engine/Cutscene.hpp"
#include "engge/Engine/Hud.hpp"
#include "engge/Input/InputConstants.hpp"
#include "engge/Dialog/DialogManager.hpp"
#include "engge/Graphics/GGFont.hpp"
#include "engge/Engine/Inventory.hpp"
#include "engge/UI/OptionsDialog.hpp"
#include "engge/UI/StartScreenDialog.hpp"
#include "engge/Engine/Preferences.hpp"
#include "engge/Room/Room.hpp"
#include "engge/Room/RoomScaling.hpp"
#include "engge/Graphics/Screen.hpp"
#include "engge/Scripting/ScriptEngine.hpp"
#include "engge/Scripting/ScriptExecute.hpp"
#include "engge/Engine/Sentence.hpp"
#include "engge/Audio/SoundDefinition.hpp"
#include "engge/Audio/SoundManager.hpp"
#include "engge/Graphics/SpriteSheet.hpp"
#include "engge/Engine/TextDatabase.hpp"
#include "engge/Engine/Thread.hpp"
#include "engge/Engine/Verb.hpp"
#include "engge/Scripting/VerbExecute.hpp"
#include "squirrel.h"
#include "../../extlibs/squirrel/squirrel/sqpcheader.h"
#include "../../extlibs/squirrel/squirrel/sqvm.h"
#include "../../extlibs/squirrel/squirrel/sqstring.h"
#include "../../extlibs/squirrel/squirrel/sqtable.h"
#include "../../extlibs/squirrel/squirrel/sqarray.h"
#include "../../extlibs/squirrel/squirrel/sqfuncproto.h"
#include "../../extlibs/squirrel/squirrel/sqclosure.h"
#include "../../extlibs/squirrel/squirrel/squserdata.h"
#include "../../extlibs/squirrel/squirrel/sqcompiler.h"
#include "../../extlibs/squirrel/squirrel/sqfuncstate.h"
#include "../../extlibs/squirrel/squirrel/sqclass.h"
#include "../Entities/Actor/TalkingState.hpp"
#include "engge/System/Logger.hpp"
#include "engge/Parsers/SavegameManager.hpp"
#include <cmath>
#include <ctime>
#include <cctype>
#include <cwchar>
#include <filesystem>
#include <iomanip>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <ngf/Graphics/Sprite.h>
#include <ngf/Graphics/RenderTexture.h>
#include <ngf/Graphics/RectangleShape.h>
#include <engge/Graphics/Text.hpp>
#include <imgui.h>
#include "engge/Engine/EngineSettings.hpp"
#include "engge/Input/CommandManager.hpp"
#include "engge/Engine/EngineCommands.hpp"
#include "DebugFeatures.hpp"
#include "../Graphics/WalkboxDrawable.hpp"
#include "../Graphics/GraphDrawable.hpp"
namespace fs = std::filesystem;

namespace ng {
namespace {
const char *_vertexShader =
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

const char *_bwFragmentShader =
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

const char *_egaFragmenShader =
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

const char *_fadeFragmentShader =
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

const char *_ghostFragmentShader =
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

const char *_sepiaFragmentShader =
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

const char *_vhsFragmentShader =
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

uint32_t toInteger(const ngf::Color &c) {
  auto r = static_cast<uint32_t>(c.r * 255u);
  auto g = static_cast<uint32_t>(c.g * 255u);
  auto b = static_cast<uint32_t>(c.b * 255u);
  auto a = static_cast<uint32_t>(c.a * 255u);
  return (r << 24) | (g << 16) | (b << 8) | a;
}
}

static const char *const _objectKey = "_objectKey";
static const char *const _roomKey = "_roomKey";
static const char *const _actorKey = "_actorKey";
static const char *const _idKey = "_id";
static const char *const _pseudoObjectsKey = "_pseudoObjects";
static const auto _clickedAtCallback = "clickedAt";

enum class CursorDirection : unsigned int {
  None = 0,
  Left = 1,
  Right = 1u << 1u,
  Up = 1u << 2u,
  Down = 1u << 3u,
  Hotspot = 1u << 4u
};

CursorDirection operator|=(CursorDirection &lhs, CursorDirection rhs) {
  lhs = static_cast<CursorDirection>(static_cast<std::underlying_type<CursorDirection>::type>(lhs) |
      static_cast<std::underlying_type<CursorDirection>::type>(rhs));
  return lhs;
}

bool operator&(CursorDirection lhs, CursorDirection rhs) {
  return static_cast<CursorDirection>(static_cast<std::underlying_type<CursorDirection>::type>(lhs) &
      static_cast<std::underlying_type<CursorDirection>::type>(rhs)) >
      CursorDirection::None;
}

enum class EngineState {
  Game, Paused, Options, StartScreen
};

struct Engine::Impl {
  class SaveGameSystem {
  public:
    explicit SaveGameSystem(Engine::Impl *pImpl) : _pImpl(pImpl) {}

    void saveGame(const std::filesystem::path &path) {
      ScriptEngine::call("preSave");

      auto actorsHash = saveActors();
      auto callbacksHash = saveCallbacks();
      auto dialogHash = saveDialogs();
      auto gameSceneHash = saveGameScene();
      auto globalsHash = saveGlobals();
      auto inventoryHash = saveInventory();
      auto objectsHash = saveObjects();
      auto roomsHash = saveRooms();

      time_t now;
      time(&now);

      SQObjectPtr g;
      _table(ScriptEngine::getVm()->_roottable)->Get(ScriptEngine::toSquirrel("g"), g);
      SQObjectPtr easyMode;
      _table(g)->Get(ScriptEngine::toSquirrel("easy_mode"), easyMode);

      ngf::GGPackValue saveGameHash = {
          {"actors", actorsHash},
          {"callbacks", callbacksHash},
          {"currentRoom", _pImpl->_pRoom->getName()},
          {"dialog", dialogHash},
          {"easy_mode", static_cast<int>(_integer(easyMode))},
          {"gameGUID", std::string()},
          {"gameScene", gameSceneHash},
          {"gameTime", _pImpl->_time.getTotalSeconds()},
          {"globals", globalsHash},
          {"inputState", _pImpl->_pEngine->getInputState()},
          {"inventory", inventoryHash},
          {"objects", objectsHash},
          {"rooms", roomsHash},
          {"savebuild", 958},
          {"savetime", static_cast<int>(now)},
          {"selectedActor", _pImpl->_pEngine->getCurrentActor()->getKey()},
          {"version", 2},
      };

      SavegameManager::saveGame(path, saveGameHash);

      ScriptEngine::call("postSave");
    }

    void loadGame(const std::string &path) {
      auto hash = SavegameManager::loadGame(path);

      std::ofstream os(path + ".json");
      os << hash;
      os.close();

      loadGame(hash);
    }

    static std::filesystem::path getSlotPath(int slot) {
      std::ostringstream filename;
      filename << "Savegame" << slot << ".save";
      auto path = Locator<EngineSettings>::get().getPath();
      path.append(filename.str());
      return path;
    }

    static void getSlot(SavegameSlot &slot) {
      auto hash = SavegameManager::loadGame(slot.path);
      slot.easyMode = hash["easy_mode"].getInt() != 0;
      slot.savetime = (time_t) hash["savetime"].getInt();
      slot.gametime = ngf::TimeSpan::seconds(static_cast<float>(hash["gameTime"].getDouble()));
    }

  private:
    static std::string getValue(const ngf::GGPackValue &property) {
      std::ostringstream s;
      if (property.isInteger()) {
        s << property.getInt();
      } else if (property.isDouble()) {
        s << property.getDouble();
      } else if (property.isString()) {
        s << property.getString();
      }
      return s.str();
    }

    SQObjectPtr toSquirrel(const ngf::GGPackValue &value) {
      if (value.isString()) {
        return ScriptEngine::toSquirrel(value.getString());
      }
      if (value.isInteger()) {
        return static_cast<SQInteger>(value.getInt());
      }
      if (value.isDouble()) {
        return static_cast<SQFloat>(value.getDouble());
      }
      if (value.isArray()) {
        auto array = SQArray::Create(_ss(ScriptEngine::getVm()), value.size());
        SQInteger i = 0;
        for (auto &item : value) {
          array->Set(i++, toSquirrel(item));
        }
        return array;
      }
      if (value.isHash()) {
        auto actor = value[_actorKey];
        if (!actor.isNull()) {
          auto pActor = getActor(actor.getString());
          return pActor->getTable();
        }
        auto object = value[_objectKey];
        auto room = value[_roomKey];
        if (!object.isNull()) {
          Object *pObject;
          if (!room.isNull()) {
            auto pRoom = getRoom(room.getString());
            pObject = getObject(pRoom, object.getString());
            if (!pObject) {
              warn("load: object {} not found", object.getString());
              return SQObjectPtr();
            }
            return pObject->getTable();
          }
          pObject = getObject(object.getString());
          if (!pObject) {
            warn("load: object {} not found", object.getString());
            return SQObjectPtr();
          }
          return pObject->getTable();
        }

        if (!room.isNull()) {
          auto pRoom = getRoom(room.getString());
          return pRoom->getTable();
        }

        auto table = SQTable::Create(_ss(ScriptEngine::getVm()), 0);
        for (const auto&[key, value] : value.items()) {
          table->NewSlot(ScriptEngine::toSquirrel(key), toSquirrel(value));
        }
        return table;
      }
      if (!value.isNull()) {
        warn("trying to convert an unknown value (type={}) to squirrel", static_cast<int >(value.type()));
      }
      return SQObjectPtr();
    }

    void loadGameScene(const ngf::GGPackValue &hash) {
      auto actorsSelectable = hash["actorsSelectable"].getInt();
      auto actorsTempUnselectable = hash["actorsTempUnselectable"].getInt();
      auto mode = actorsSelectable ? ActorSlotSelectableMode::On : ActorSlotSelectableMode::Off;
      if (actorsTempUnselectable) {
        mode |= ActorSlotSelectableMode::TemporaryUnselectable;
      }
      _pImpl->_pEngine->setActorSlotSelectable(mode);
      auto forceTalkieText = hash["forceTalkieText"].getInt() != 0;
      _pImpl->_pEngine->getPreferences().setTempPreference(TempPreferenceNames::ForceTalkieText, forceTalkieText);
      for (const auto &selectableActor : hash["selectableActors"]) {
        auto pActor = getActor(selectableActor[_actorKey].getString());
        auto selectable = selectableActor["selectable"].getInt() != 0;
        _pImpl->_pEngine->actorSlotSelectable(pActor, selectable);
      }
    }

    void loadDialog(const ngf::GGPackValue &hash) {
      auto &states = _pImpl->_dialogManager.getStates();
      states.clear();
      for (auto &property : hash.items()) {
        const auto& dialog = property.key();
        // dialog format: mode dialog number actor
        // example: #ChetAgentStreetDialog14reyes
        // mode:
        // ?: once
        // #: showonce
        // &: onceever
        // $: showonceever
        // ^: temponce
        auto state = parseState(dialog);
        states.push_back(state);
        // TODO: what to do with this dialog value ?
        //auto value = property.second.getInt();
      }
    }

    [[nodiscard]] static DialogConditionState parseState(const std::string &dialog) {
      DialogConditionState state;
      switch (dialog[0]) {
      case '?':state.mode = DialogConditionMode::Once;
        break;
      case '#':state.mode = DialogConditionMode::ShowOnce;
        break;
      case '&':state.mode = DialogConditionMode::OnceEver;
        break;
      case '$':state.mode = DialogConditionMode::ShowOnceEver;
        break;
      case '^':state.mode = DialogConditionMode::TempOnce;
        break;
      }
      std::string dialogName;
      int i;
      for (i = 1; i < static_cast<int>(dialog.length()) && !isdigit(dialog[i]); i++) {
        dialogName.append(1, dialog[i]);
      }
      auto &settings = Locator<EngineSettings>::get();
      while (!settings.hasEntry(dialogName + ".byack")) {
        dialogName.append(1, dialog.at(i++));
      }
      std::string num;
      state.dialog = dialogName;
      for (; i < static_cast<int>(dialog.length()) && isdigit(dialog[i]); i++) {
        num.append(1, dialog[i]);
      }
      state.line = atol(num.data());
      state.actorKey = dialog.substr(i);
      return state;
    }

    void loadCallbacks(const ngf::GGPackValue &hash) {
      _pImpl->_callbacks.clear();
      for (auto &callBackHash : hash["callbacks"]) {
        auto name = callBackHash["function"].getString();
        auto id = callBackHash["guid"].getInt();
        auto time = ngf::TimeSpan::seconds(static_cast<float>(callBackHash["time"].getInt()) / 1000.f);
        auto arg = toSquirrel(callBackHash["param"]);
        auto callback = std::make_unique<Callback>(id, time, name, arg);
        _pImpl->_callbacks.push_back(std::move(callback));
      }
      Locator<EntityManager>::get().setCallbackId(hash["nextGuid"].getInt());
    }

    void loadActors(const ngf::GGPackValue &hash) {
      for (auto &pActor : _pImpl->_actors) {
        if (pActor->getKey().empty())
          continue;

        const auto &actorHash = hash[pActor->getKey()];
        loadActor(pActor.get(), actorHash);
      }
    }

    void loadActor(Actor *pActor, const ngf::GGPackValue &actorHash) {
      ngf::Color color{ngf::Colors::White};
      getValue(actorHash, "_color", color);
      pActor->setColor(color);

      glm::vec2 pos;
      getValue(actorHash, "_pos", pos);
      pActor->setPosition(pos);

      std::string costume;
      getValue(actorHash, "_costume", costume);
      std::string costumesheet;
      getValue(actorHash, "_costumeSheet", costumesheet);
      pActor->getCostume().loadCostume(costume, costumesheet);

      std::string room;
      getValue(actorHash, _roomKey, room);
      auto *pRoom = getRoom(room.empty() ? "Void" : room);
      pActor->setRoom(pRoom);

      int dir = static_cast<int>(Facing::FACE_FRONT);
      getValue(actorHash, "_dir", dir);
      pActor->getCostume().setFacing(static_cast<Facing>(dir));

      int useDirValue = static_cast<int>(UseDirection::Front);
      getValue(actorHash, "_useDir", useDirValue);
      pActor->setUseDirection(static_cast<UseDirection>(dir));

      int lockFacing = 0;
      getValue(actorHash, "_lockFacing", lockFacing);
      if (lockFacing == 0) {
        pActor->getCostume().unlockFacing();
      } else {
        pActor->getCostume().lockFacing(static_cast<Facing>(lockFacing),
                                        static_cast<Facing>(lockFacing),
                                        static_cast<Facing>(lockFacing),
                                        static_cast<Facing>(lockFacing));
      }
      float volume = 0;
      getValue(actorHash, "_volume", volume);
      pActor->setVolume(volume);

      glm::vec2 usePos;
      getValue(actorHash, "_usePos", usePos);
      pActor->setUsePosition(usePos);

      glm::vec2 renderOffset = glm::vec2(0, 45);
      getValue(actorHash, "_renderOffset", renderOffset);
      pActor->setRenderOffset((glm::ivec2) renderOffset);

      glm::vec2 offset;
      getValue(actorHash, "_offset", offset);
      pActor->setOffset(offset);

      for (auto &property : actorHash.items()) {
        if (property.key().empty() || property.key()[0] == '_') {
          if (property.key() == "_animations") {
            std::vector<std::string> anims;
            for (auto &value : property.value()) {
              anims.push_back(value.getString());
            }
            // TODO: _animations
            trace("load: actor {} property '{}' not loaded (type={}) size={}",
                  pActor->getKey(),
                  property.key(),
                  static_cast<int>(property.value().type()), anims.size());
          } else if ((property.key() == "_pos") || (property.key() == "_costume") || (property.key() == "_costumeSheet")
              || (property.key() == _roomKey) ||
              (property.key() == "_color") || (property.key() == "_dir") || (property.key() == "_useDir")
              || (property.key() == "_lockFacing") || (property.key() == "_volume") || (property.key() == "_usePos")
              || (property.key() == "_renderOffset") || (property.key() == "_offset")) {
          } else {
            // TODO: other types
            auto s = getValue(property.value());
            trace("load: actor {} property '{}' not loaded (type={}): {}",
                  pActor->getKey(),
                  property.key(),
                  static_cast<int>(property.value().type()), s);
          }
          continue;
        }

        _table(pActor->getTable())->Set(ScriptEngine::toSquirrel(property.key()), toSquirrel(property.value()));
      }
      if (ScriptEngine::rawExists(pActor, "postLoad")) {
        ScriptEngine::objCall(pActor, "postLoad");
      }
    }

    void loadInventory(const ngf::GGPackValue &hash) {
      for (auto i = 0; i < static_cast<int>(_pImpl->_actorsIconSlots.size()); ++i) {
        auto *pActor = _pImpl->_actorsIconSlots[i].pActor;
        if (!pActor)
          continue;
        auto &slot = hash["slots"].at(i);
        pActor->clearInventory();
        int jiggleCount = 0;
        for (const auto &obj : slot["objects"]) {
          auto pObj = getInventoryObject(obj.getString());
          // TODO: why we don't find the inventory object here ?
          if (!pObj)
            continue;
          const auto jiggle = slot["jiggle"].isArray() && slot["jiggle"][jiggleCount++].getInt() != 0;
          pObj->setJiggle(jiggle);
          pActor->pickupObject(pObj);
        }
        auto scroll = slot["scroll"].getInt();
        pActor->setInventoryOffset(scroll);
      }
    }

    void loadObjects(const ngf::GGPackValue &hash) {
      for (auto &obj :  hash.items()) {
        const auto& objName = obj.key();
        if (objName.empty())
          continue;
        auto pObj = getObject(objName);
        // TODO: if the object does not exist creates it
        if (!pObj) {
          trace("load: object '{}' not loaded because it has not been found", objName);
          continue;
        }
        loadObject(pObj, obj.value());
      }
    }

    static void getValue(const ngf::GGPackValue &hash, const std::string &key, int &value) {
      if (!hash[key].isNull()) {
        value = hash[key].getInt();
      }
    }

    static void getValue(const ngf::GGPackValue &hash, const std::string &key, std::string &value) {
      if (!hash[key].isNull()) {
        value = hash[key].getString();
      }
    }

    static void getValue(const ngf::GGPackValue &hash, const std::string &key, float &value) {
      if (!hash[key].isNull()) {
        value = static_cast<float>(hash[key].getDouble());
      }
    }

    static void getValue(const ngf::GGPackValue &hash, const std::string &key, bool &value) {
      if (!hash[key].isNull()) {
        value = hash[key].getInt() != 0;
      }
    }

    static void getValue(const ngf::GGPackValue &hash, const std::string &key, glm::vec2 &value) {
      if (!hash[key].isNull()) {
        value = _parsePos(hash[key].getString());
      }
    }

    static void getValue(const ngf::GGPackValue &hash, const std::string &key, ngf::Color &value) {
      if (!hash[key].isNull()) {
        value = _toColor(hash[key].getInt());
      }
    }

    void loadObject(Object *pObj, const ngf::GGPackValue &hash) {
      auto state = 0;
      ScriptEngine::rawGet(pObj, "initState", state);
      getValue(hash, "_state", state);
      pObj->setStateAnimIndex(state);
      auto touchable = true;
      ScriptEngine::rawGet(pObj, "initTouchable", touchable);
      getValue(hash, "_touchable", touchable);
      pObj->setTouchable(touchable);
      glm::vec2 offset;
      getValue(hash, "_offset", offset);
      pObj->setOffset(offset);
      bool hidden = false;
      getValue(hash, "_hidden", hidden);
      pObj->setVisible(!hidden);
      float rotation = 0;
      getValue(hash, "_rotation", rotation);
      pObj->setRotation(rotation);
      ngf::Color color{ngf::Colors::White};
      getValue(hash, "_color", color);
      pObj->setColor(color);

      for (auto &property :  hash.items()) {
        if (property.key().empty() || property.key()[0] == '_') {
          if (property.key() == "_state" || property.key() == "_touchable" || property.key() == "_offset"
              || property.key() == "_hidden" || property.key() == "_rotation" || property.key() == "_color")
            continue;

          // TODO: other types
          auto s = getValue(property.value());
          warn("load: object {} property '{}' not loaded (type={}): {}",
               pObj->getKey(),
               property.key(),
               static_cast<int>(property.value().type()), s);
          continue;
        }

        _table(pObj->getTable())->Set(ScriptEngine::toSquirrel(property.key()), toSquirrel(property.value()));
      }
    }

    void loadPseudoObjects(Room *pRoom, const ngf::GGPackValue &hash) {
      for (const auto &entry :  hash.items()) {
        auto pObj = getObject(pRoom, entry.key());
        if (!pObj) {
          trace("load: room '{}' object '{}' not loaded because it has not been found", pRoom->getName(), entry.key());
          continue;
        }
        loadObject(pObj, entry.value());
      }
    }

    void loadRooms(const ngf::GGPackValue &hash) {
      for (auto &roomHash :  hash.items()) {
        const auto& roomName = roomHash.key();
        auto pRoom = getRoom(roomName);
        if (!pRoom) {
          trace("load: room '{}' not loaded because it has not been found", roomName);
          continue;
        }

        for (auto &property : roomHash.value().items()) {
          if (property.key().empty() || property.key()[0] == '_') {
            if (property.key() == _pseudoObjectsKey) {
              loadPseudoObjects(pRoom, property.value());
            } else {
              trace("load: room '{}' property '{}' (type={}) not loaded",
                    roomName,
                    property.key(),
                    static_cast<int>(property.value().type()));
              continue;
            }
          }

          _table(pRoom->getTable())->Set(ScriptEngine::toSquirrel(property.key()), toSquirrel(property.value()));
          if (ScriptEngine::rawExists(pRoom, "postLoad")) {
            ScriptEngine::objCall(pRoom, "postLoad");
          }
        }
      }
    }

    void loadGame(const ngf::GGPackValue &hash) {
      auto version = hash["version"].getInt();
      if (version != 2) {
        warn("Cannot load savegame version {}", version);
        return;
      }

      ScriptEngine::call("preLoad");

      loadGameScene(hash["gameScene"]);
      loadDialog(hash["dialog"]);
      loadCallbacks(hash["callbacks"]);
      loadGlobals(hash["globals"]);
      loadActors(hash["actors"]);
      loadInventory(hash["inventory"]);
      loadRooms(hash["rooms"]);

      _pImpl->_time = ngf::TimeSpan::seconds(static_cast<float>(hash["gameTime"].getDouble()));
      _pImpl->_pEngine->setInputState(hash["inputState"].getInt());

      setActor(hash["selectedActor"].getString());
      setCurrentRoom(hash["currentRoom"].getString());
      loadObjects(hash["objects"]);

      ScriptEngine::set("SAVEBUILD", hash["savebuild"].getInt());

      ScriptEngine::call("postLoad");
    }

    void setActor(const std::string &name) {
      auto *pActor = getActor(name);
      _pImpl->_pEngine->setCurrentActor(pActor, false);
    }

    Actor *getActor(const std::string &name) {
      return dynamic_cast<Actor *>(_pImpl->_pEngine->getEntity(name));
    }

    Room *getRoom(const std::string &name) {
      auto &rooms = _pImpl->_pEngine->getRooms();
      auto it = std::find_if(rooms.begin(), rooms.end(), [&name](auto &pRoom) { return pRoom->getName() == name; });
      if (it != rooms.end())
        return it->get();
      return nullptr;
    }

    static Object *getInventoryObject(const std::string &name) {
      auto v = ScriptEngine::getVm();
      SQObjectPtr obj;
      if (!_table(v->_roottable)->Get(ScriptEngine::toSquirrel(name), obj)) {
        return nullptr;
      }
      SQObjectPtr id;
      if (!_table(obj)->Get(ScriptEngine::toSquirrel(_idKey), id)) {
        return nullptr;
      }
      return EntityManager::getObjectFromId(static_cast<int>(_integer(id)));
    }

    Object *getObject(const std::string &name) {
      for (auto &pRoom : _pImpl->_rooms) {
        for (auto &pObj : pRoom->getObjects()) {
          if (pObj->getKey() == name)
            return pObj.get();
        }
      }
      return nullptr;
    }

    static Object *getObject(Room *pRoom, const std::string &name) {
      for (auto &pObj : pRoom->getObjects()) {
        if (pObj->getKey() == name)
          return pObj.get();
      }
      return nullptr;
    }

    void setCurrentRoom(const std::string &name) {
      _pImpl->_pEngine->setRoom(getRoom(name));
    }

    [[nodiscard]] ngf::GGPackValue saveActors() const {
      ngf::GGPackValue actorsHash;
      for (auto &pActor : _pImpl->_actors) {
        // TODO: find why this entry exists...
        if (pActor->getKey().empty())
          continue;

        auto table = pActor->getTable();
        auto actorHash = ng::toGGPackValue(table);
        auto costume = fs::path(pActor->getCostume().getPath()).filename();
        if (costume.has_extension())
          costume.replace_extension();
        actorHash["_costume"] = costume.u8string();
        actorHash["_dir"] = static_cast<int>(pActor->getCostume().getFacing());
        auto lockFacing = pActor->getCostume().getLockFacing();
        actorHash["_lockFacing"] = lockFacing.has_value() ? static_cast<int>(lockFacing.value()) : 0;
        actorHash["_pos"] = toString(pActor->getPosition());
        if (pActor->getVolume().has_value()) {
          actorHash["_volume"] = pActor->getVolume().value();
        }
        auto useDir = pActor->getUseDirection();
        if (useDir.has_value()) {
          actorHash["_useDir"] = static_cast<int>(useDir.value());
        }
        auto usePos = pActor->getUsePosition();
        if (useDir.has_value()) {
          actorHash["_usePos"] = toString(usePos.value());
        }
        auto renderOffset = pActor->getRenderOffset();
        if (renderOffset != glm::ivec2(0, 45)) {
          actorHash["_renderOffset"] = toString(renderOffset);
        }
        if (pActor->getColor() != ngf::Colors::White) {
          actorHash["_color"] = static_cast<int>(toInteger(pActor->getColor()));
        }
        auto costumeSheet = pActor->getCostume().getSheet();
        if (!costumeSheet.empty()) {
          actorHash["_costumeSheet"] = costumeSheet;
        }
        if (pActor->getRoom()) {
          actorHash[_roomKey] = pActor->getRoom()->getName();
        } else {
          actorHash[_roomKey] = nullptr;
        }

        actorsHash[pActor->getKey()] = actorHash;
      }
      return actorsHash;
    }

    [[nodiscard]] static ngf::GGPackValue saveGlobals() {
      auto v = ScriptEngine::getVm();
      auto top = sq_gettop(v);
      sq_pushroottable(v);
      sq_pushstring(v, _SC("g"), -1);
      sq_get(v, -2);
      HSQOBJECT g;
      sq_getstackobj(v, -1, &g);

      auto globalsHash = ng::toGGPackValue(g);
      sq_settop(v, top);
      return globalsHash;
    }

    [[nodiscard]] ngf::GGPackValue saveDialogs() const {
      ngf::GGPackValue hash;
      const auto &states = _pImpl->_dialogManager.getStates();
      for (const auto &state : states) {
        std::ostringstream s;
        switch (state.mode) {
        case DialogConditionMode::TempOnce:continue;
        case DialogConditionMode::OnceEver:s << "&";
          break;
        case DialogConditionMode::ShowOnce:s << "#";
          break;
        case DialogConditionMode::Once:s << "?";
          break;
        case DialogConditionMode::ShowOnceEver:s << "$";
          break;
        }
        s << state.dialog << state.line << state.actorKey;
        // TODO: value should be 1 or another value ?
        hash[s.str()] = state.mode == DialogConditionMode::ShowOnce ? 2 : 1;
      }
      return hash;
    }

    [[nodiscard]] ngf::GGPackValue saveGameScene() const {
      auto actorsSelectable =
          ((_pImpl->_actorIcons.getMode() & ActorSlotSelectableMode::On) == ActorSlotSelectableMode::On);
      auto actorsTempUnselectable = ((_pImpl->_actorIcons.getMode() & ActorSlotSelectableMode::TemporaryUnselectable)
          == ActorSlotSelectableMode::TemporaryUnselectable);

      ngf::GGPackValue selectableActors;
      for (auto &slot : _pImpl->_actorsIconSlots) {
        ngf::GGPackValue selectableActor;
        if (slot.pActor) {
          selectableActor = {
              {_actorKey, slot.pActor->getKey()},
              {"selectable", slot.selectable ? 1 : 0},
          };
        } else {
          selectableActor = {{"selectable", 0}};
        }
        selectableActors.push_back(selectableActor);
      }

      auto forceTalkieText = _pImpl->_pEngine->getPreferences()
          .getTempPreference(TempPreferenceNames::ForceTalkieText,
                             TempPreferenceDefaultValues::ForceTalkieText);
      return {
          {"actorsSelectable", actorsSelectable ? 1 : 0},
          {"actorsTempUnselectable", actorsTempUnselectable ? 1 : 0},
          {"forceTalkieText", forceTalkieText ? 1 : 0},
          {"selectableActors", selectableActors}
      };
    }

    [[nodiscard]] ngf::GGPackValue saveInventory() const {
      ngf::GGPackValue slots;
      for (auto &slot : _pImpl->_actorsIconSlots) {
        ngf::GGPackValue actorSlot;
        if (slot.pActor) {
          std::vector<int> jiggleArray(slot.pActor->getObjects().size());
          ngf::GGPackValue objects;
          int jiggleCount = 0;
          for (auto &obj : slot.pActor->getObjects()) {
            jiggleArray[jiggleCount++] = obj->getJiggle() ? 1 : 0;
            objects.push_back(obj->getKey());
          }
          actorSlot = {
              {"objects", objects},
              {"scroll", slot.pActor->getInventoryOffset()},
          };
          const auto saveJiggle = std::any_of(jiggleArray.cbegin(),
                                              jiggleArray.cend(), [](int value) { return value == 1; });
          if (saveJiggle) {
            ngf::GGPackValue jiggle;
            std::copy(jiggleArray.cbegin(),
                           jiggleArray.cend(),
                           std::back_inserter(jiggle));
            actorSlot["jiggle"] = jiggle;
          }
        } else {
          actorSlot = {{"scroll", 0}};
        }
        slots.push_back(actorSlot);
      }

      return {
          {"slots", slots},
      };
    }

    [[nodiscard]] ngf::GGPackValue saveObjects() const {
      ngf::GGPackValue hash;
      for (auto &room : _pImpl->_rooms) {
        for (auto &object : room->getObjects()) {
          if (object->getType() != ObjectType::Object)
            continue;
          auto pRoom = object->getRoom();
          if (pRoom && pRoom->isPseudoRoom())
            continue;
          hash[object->getKey()] = saveObject(object.get());
        }
      }
      return hash;
    }

    static ngf::GGPackValue savePseudoObjects(const Room *pRoom) {
      ngf::GGPackValue hashObjects;
      for (const auto &pObj : pRoom->getObjects()) {
        hashObjects[pObj->getKey()] = saveObject(pObj.get());
      }
      return hashObjects;
    }

    static ngf::GGPackValue saveObject(const Object *pObject) {
      auto hashObject = ng::toGGPackValue(pObject->getTable());
      if (pObject->getState() != 0) {
        hashObject["_state"] = pObject->getState();
      }
      if (!pObject->isTouchable()) {
        hashObject["_touchable"] = pObject->isTouchable() ? 1 : 0;
      }
      // this is the way to compare 2 vectors... not so simple
      if (glm::any(glm::epsilonNotEqual(pObject->getOffset(), glm::vec2(0, 0), 1e-6f))) {
        hashObject["_offset"] = toString(pObject->getOffset());
      }
      return hashObject;
    }

    [[nodiscard]] ngf::GGPackValue saveRooms() const {
      ngf::GGPackValue hash;
      for (auto &room : _pImpl->_rooms) {
        auto hashRoom = ng::toGGPackValue(room->getTable());
        if (room->isPseudoRoom()) {
          hashRoom[_pseudoObjectsKey] = savePseudoObjects(room.get());
        }
        hash[room->getName()] = hashRoom;
      }
      return hash;
    }

    [[nodiscard]] ngf::GGPackValue saveCallbacks() const {
      ngf::GGPackValue callbacksArray;
      for (auto &callback : _pImpl->_callbacks) {
        ngf::GGPackValue callbackHash{
            {"function", callback->getMethod()},
            {"guid", callback->getId()},
            {"time", callback->getElapsed().getTotalMilliseconds()}
        };
        auto arg = callback->getArgument();
        if (arg._type != OT_NULL) {
          callbackHash["param"] = ng::toGGPackValue(arg);
        }
        callbacksArray.push_back(callbackHash);
      }

      auto &resourceManager = Locator<EntityManager>::get();
      auto id = resourceManager.getCallbackId();
      resourceManager.setCallbackId(id);

      return {
          {"callbacks", callbacksArray},
          {"nextGuid", id},
      };
    }

    void loadGlobals(const ngf::GGPackValue &hash) {
      SQTable *pRootTable = _table(ScriptEngine::getVm()->_roottable);
      SQObjectPtr gObject;
      pRootTable->Get(ScriptEngine::toSquirrel("g"), gObject);
      SQTable *gTable = _table(gObject);
      for (const auto &variable : hash.items()) {
        gTable->Set(ScriptEngine::toSquirrel(variable.key()), toSquirrel(variable.value()));
      }
    }

    static std::string toString(const glm::vec2 &pos) {
      std::ostringstream os;
      os << "{" << static_cast<int>(pos.x) << "," << static_cast<int>(pos.y) << "}";
      return os.str();
    }

    static std::string toString(const glm::ivec2 &pos) {
      std::ostringstream os;
      os << "{" << pos.x << "," << pos.y << "}";
      return os.str();
    }

  private:
    Impl *_pImpl{nullptr};
  };

  Engine *_pEngine{nullptr};
  ResourceManager &_textureManager;
  Room *_pRoom{nullptr};
  int _roomEffect{0};
  ngf::Shader _roomShader;
  ngf::Shader _fadeShader;
  ngf::Texture _blackTexture;
  std::vector<std::unique_ptr<Actor>> _actors;
  std::vector<std::unique_ptr<Room>> _rooms;
  std::vector<std::unique_ptr<Function>> _newFunctions;
  std::vector<std::unique_ptr<Function>> _functions;
  std::vector<std::unique_ptr<Callback>> _callbacks;
  Cutscene *_pCutscene{nullptr};
  ngf::Application *_pApp{nullptr};
  Actor *_pCurrentActor{nullptr};
  bool _inputHUD{false};
  bool _inputActive{false};
  bool _showCursor{true};
  bool _inputVerbsActive{false};
  Actor *_pFollowActor{nullptr};
  Entity *_pUseObject{nullptr};
  int _objId1{0};
  Entity *_pObj2{nullptr};
  glm::vec2 _mousePos{0, 0};
  glm::vec2 _mousePosInRoom{0, 0};
  std::unique_ptr<VerbExecute> _pVerbExecute;
  std::unique_ptr<ScriptExecute> _pScriptExecute;
  std::vector<std::unique_ptr<ThreadBase>> _threads;
  DialogManager _dialogManager;
  Preferences &_preferences;
  SoundManager &_soundManager;
  CursorDirection _cursorDirection{CursorDirection::None};
  std::array<ActorIconSlot, 6> _actorsIconSlots;
  UseFlag _useFlag{UseFlag::None};
  ActorIcons _actorIcons;
  ngf::TimeSpan _time;
  bool _isMouseDown{false};
  ngf::TimeSpan _mouseDownTime;
  bool _isMouseRightDown{false};
  int _frameCounter{0};
  HSQOBJECT _pDefaultObject{};
  Camera _camera;
  std::unique_ptr<Sentence> _pSentence{};
  std::unordered_set<Input, InputHash> _oldKeyDowns;
  std::unordered_set<Input, InputHash> _newKeyDowns;
  EngineState _state{EngineState::StartScreen};
  TalkingState _talkingState;
  int _showDrawWalkboxes{0};
  OptionsDialog _optionsDialog;
  StartScreenDialog _startScreenDialog;
  bool _run{false};
  ngf::TimeSpan _noOverrideElapsed{ngf::TimeSpan::seconds(2)};
  Hud _hud;
  bool _autoSave{true};
  bool _cursorVisible{true};
  FadeEffectParameters _fadeEffect;

  Impl();

  void drawHud(ngf::RenderTarget &target) const;
  void drawCursor(ngf::RenderTarget &target) const;
  void drawCursorText(ngf::RenderTarget &target) const;
  void drawNoOverride(ngf::RenderTarget &target) const;
  int getCurrentActorIndex() const;
  ngf::irect getCursorRect() const;
  void appendUseFlag(std::wstring &sentence) const;
  bool clickedAt(const glm::vec2 &pos) const;
  void updateCutscene(const ngf::TimeSpan &elapsed);
  void updateFunctions(const ngf::TimeSpan &elapsed);
  void updateActorIcons(const ngf::TimeSpan &elapsed);
  void updateSentence(const ngf::TimeSpan &elapsed) const;
  void updateMouseCursor();
  void updateHoveredEntity(bool isRightClick);
  SQInteger enterRoom(Room *pRoom, Object *pObject) const;
  SQInteger exitRoom(Object *pObject);
  void updateRoomScalings() const;
  void setCurrentRoom(Room *pRoom);
  uint32_t getFlags(int id) const;
  uint32_t getFlags(Entity *pEntity) const;
  Entity *getHoveredEntity(const glm::vec2 &mousPos);
  void actorEnter() const;
  void actorExit() const;
  static void onLanguageChange(const std::string &lang);
  void onVerbClick(const Verb *pVerb);
  void updateKeyboard();
  bool isKeyPressed(const Input &key);
  void updateKeys();
  static InputConstants toKey(const std::string &keyText);
  void drawPause(ngf::RenderTarget &target) const;
  void stopThreads();
  void drawWalkboxes(ngf::RenderTarget &target) const;
  const Verb *getHoveredVerb() const;
  static std::wstring getDisplayName(const std::wstring &name);
  void run(bool state);
  void stopTalking() const;
  void stopTalkingExcept(Entity *pEntity) const;
  Entity *getEntity(Entity *pEntity) const;
  const Verb *overrideVerb(const Verb *pVerb) const;
  void captureScreen(const std::string &path) const;
  void skipText() const;
  void skipCutscene();
  void pauseGame();
  void selectActor(int index);
  void selectPreviousActor();
  void selectNextActor();
  bool hasFlag(int id, uint32_t flagToTest) const;
};

Engine::Impl::Impl()
    : _textureManager(Locator<ResourceManager>::get()),
      _preferences(Locator<Preferences>::get()),
      _soundManager(Locator<SoundManager>::get()),
      _actorIcons(_actorsIconSlots, _hud, _pCurrentActor) {
  _hud.setTextureManager(&_textureManager);
  sq_resetobject(&_pDefaultObject);

  Locator<CommandManager>::get().registerCommands(
      {
          {EngineCommands::SkipText, [this]() { skipText(); }},
          {EngineCommands::SkipCutscene, [this] { skipCutscene(); }},
          {EngineCommands::PauseGame, [this] { pauseGame(); }},
          {EngineCommands::SelectActor1, [this] { selectActor(1); }},
          {EngineCommands::SelectActor2, [this] { selectActor(2); }},
          {EngineCommands::SelectActor3, [this] { selectActor(3); }},
          {EngineCommands::SelectActor4, [this] { selectActor(4); }},
          {EngineCommands::SelectActor5, [this] { selectActor(5); }},
          {EngineCommands::SelectActor6, [this] { selectActor(6); }},
          {EngineCommands::SelectPreviousActor, [this] { selectPreviousActor(); }},
          {EngineCommands::SelectNextActor, [this] { selectNextActor(); }},
          {EngineCommands::SelectChoice1, [this] { _dialogManager.choose(1); }},
          {EngineCommands::SelectChoice2, [this] { _dialogManager.choose(2); }},
          {EngineCommands::SelectChoice3, [this] { _dialogManager.choose(3); }},
          {EngineCommands::SelectChoice4, [this] { _dialogManager.choose(4); }},
          {EngineCommands::SelectChoice5, [this] { _dialogManager.choose(5); }},
          {EngineCommands::SelectChoice6, [this] { _dialogManager.choose(6); }},
          {EngineCommands::ShowOptions, [this] { _pEngine->showOptions(true); }},
          {EngineCommands::ToggleHud, [this] {
            _hud.setVisible(!_cursorVisible);
            _actorIcons.setVisible(!_cursorVisible);
            _cursorVisible = !_cursorVisible;
          }}
      });
  Locator<CommandManager>::get().registerPressedCommand(EngineCommands::ShowHotspots, [this](bool down) {
    _preferences.setTempPreference(TempPreferenceNames::ShowHotspot, down);
  });

  _fadeShader.load(_vertexShader, _fadeFragmentShader);
  uint32_t pixels[4]{0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF};
  _blackTexture.loadFromMemory({2, 2}, pixels);
}

void Engine::Impl::pauseGame() {
  _state = _state == EngineState::Game ? EngineState::Paused : EngineState::Game;
  if (_state == EngineState::Paused) {
    _soundManager.pauseAllSounds();
  } else {
    _soundManager.resumeAllSounds();
  }
}

void Engine::Impl::selectActor(int index) {
  if (index <= 0 || index > static_cast<int>(_actorsIconSlots.size()))
    return;
  const auto &slot = _actorsIconSlots[index - 1];
  if (!slot.selectable)
    return;
  _pEngine->setCurrentActor(slot.pActor, true);
}

void Engine::Impl::selectPreviousActor() {
  auto currentActorIndex = getCurrentActorIndex();
  if (currentActorIndex == -1)
    return;
  auto size = static_cast<int>(_actorsIconSlots.size());
  for (auto i = 0; i < size; i++) {
    auto index = currentActorIndex - i - 1;
    if (index < 0)
      index += size - 1;
    if (index == currentActorIndex)
      return;
    const auto &slot = _actorsIconSlots[index];
    if (slot.selectable) {
      _pEngine->setCurrentActor(slot.pActor, true);
      return;
    }
  }
}

void Engine::Impl::selectNextActor() {
  auto currentActorIndex = getCurrentActorIndex();
  if (currentActorIndex == -1)
    return;
  auto size = static_cast<int>(_actorsIconSlots.size());
  for (auto i = 0; i < size; i++) {
    auto index = (currentActorIndex + i + 1) % size;
    if (index == currentActorIndex)
      return;
    const auto &slot = _actorsIconSlots[index];
    if (slot.selectable) {
      _pEngine->setCurrentActor(slot.pActor, true);
      return;
    }
  }
}

void Engine::Impl::skipCutscene() {
  if (_pEngine->inCutscene()) {
    if (_pCutscene && _pCutscene->hasCutsceneOverride()) {
      _pEngine->cutsceneOverride();
    } else {
      _noOverrideElapsed = ngf::TimeSpan::seconds(0);
    }
  }
}

void Engine::Impl::skipText() const {
  if (_dialogManager.getState() == DialogManagerState::Active) {
    stopTalking();
  }
}

void Engine::Impl::onLanguageChange(const std::string &lang) {
  std::stringstream ss;
  ss << "ThimbleweedText_" << lang << ".tsv";
  Locator<TextDatabase>::get().load(ss.str());

  ScriptEngine::call("onLanguageChange");
}

SQInteger Engine::Impl::exitRoom(Object *pObject) {
  _pEngine->setDefaultVerb();
  _talkingState.stop();

  if (!_pRoom)
    return 0;

  auto pOldRoom = _pRoom;

  actorExit();

  // call exit room function
  auto nparams = ScriptEngine::getParameterCount(pOldRoom, "exit");
  trace("call exit room function of {} ({} params)", pOldRoom->getName(), nparams);

  if (nparams == 2) {
    auto pRoom = pObject ? pObject->getRoom() : nullptr;
    ScriptEngine::rawCall(pOldRoom, "exit", pRoom);
  } else {
    ScriptEngine::rawCall(pOldRoom, "exit");
  }

  pOldRoom->exit();

  ScriptEngine::rawCall("exitedRoom", pOldRoom);

  // stop all local threads
  std::for_each(_threads.begin(), _threads.end(), [](auto &pThread) {
    if (!pThread->isGlobal())
      pThread->stop();
  });

  return 0;
}

void Engine::Impl::actorEnter() const {
  if (!_pCurrentActor)
    return;

  _pCurrentActor->stopWalking();
  ScriptEngine::rawCall("actorEnter", _pCurrentActor);

  if (!_pRoom)
    return;

  if (ScriptEngine::rawExists(_pRoom, "actorEnter")) {
    ScriptEngine::rawCall(_pRoom, "actorEnter", _pCurrentActor);
  }
}

void Engine::Impl::actorExit() const {
  if (!_pCurrentActor || !_pRoom)
    return;

  if (ScriptEngine::rawExists(_pRoom, "actorExit")) {
    ScriptEngine::rawCall(_pRoom, "actorExit", _pCurrentActor);
  }
}

SQInteger Engine::Impl::enterRoom(Room *pRoom, Object *pObject) const {
  // call enter room function
  trace("call enter room function of {}", pRoom->getName());
  auto nparams = ScriptEngine::getParameterCount(pRoom, "enter");
  if (nparams == 2) {
    ScriptEngine::rawCall(pRoom, "enter", pObject);
  } else {
    ScriptEngine::rawCall(pRoom, "enter");
  }

  actorEnter();

  auto lang = Locator<Preferences>::get().getUserPreference<std::string>(PreferenceNames::Language,
                                                                         PreferenceDefaultValues::Language);
  const auto &spriteSheet = pRoom->getSpriteSheet();
  auto &objects = pRoom->getObjects();
  for (auto &obj : objects) {
    for (auto &anim : obj->getAnims()) {
      for (size_t i = 0; i < anim.frames.size(); ++i) {
        auto &frame = anim.frames.at(i);
        auto name = frame.name;
        if (!endsWith(name, "_en"))
          continue;

        checkLanguage(name);
        anim.frames[i] = spriteSheet.getItem(name);
      }
    }
    if (obj->getId() == 0 || obj->isTemporary())
      continue;

    if (ScriptEngine::rawExists(obj.get(), "enter")) {
      ScriptEngine::rawCall(obj.get(), "enter");
    }
  }

  ScriptEngine::rawCall("enteredRoom", pRoom);

  return 0;
}

void Engine::Impl::run(bool state) {
  if (_run != state) {
    _run = state;
    if (_pCurrentActor) {
      ScriptEngine::objCall(_pCurrentActor, "run", state);
    }
  }
}

void Engine::Impl::setCurrentRoom(Room *pRoom) {
  if (pRoom) {
    ScriptEngine::set("currentRoom", pRoom);
  }
  _camera.resetBounds();
  _pRoom = pRoom;
  _camera.at(glm::vec2(0, 0));
}

void Engine::Impl::updateCutscene(const ngf::TimeSpan &elapsed) {
  if (_pCutscene) {
    (*_pCutscene)(elapsed);
    if (_pCutscene->isElapsed()) {
      _pCutscene = nullptr;
    }
  }
}

void Engine::Impl::updateSentence(const ngf::TimeSpan &elapsed) const {
  if (!_pSentence)
    return;
  (*_pSentence)(elapsed);
  if (!_pSentence->isElapsed())
    return;
  _pEngine->stopSentence();
}

void Engine::Impl::updateFunctions(const ngf::TimeSpan &elapsed) {
  for (auto &function : _newFunctions) {
    _functions.push_back(std::move(function));
  }
  _newFunctions.clear();
  for (auto &function : _functions) {
    (*function)(elapsed);
  }
  _functions.erase(std::remove_if(_functions.begin(), _functions.end(),
                                  [](std::unique_ptr<Function> &f) { return f->isElapsed(); }),
                   _functions.end());

  std::vector<std::unique_ptr<Callback>> callbacks;
  std::move(_callbacks.begin(), _callbacks.end(), std::back_inserter(callbacks));
  _callbacks.clear();
  for (auto &callback : callbacks) {
    (*callback)(elapsed);
  }
  callbacks.erase(std::remove_if(callbacks.begin(),
                                 callbacks.end(),
                                 [](auto &f) { return f->isElapsed(); }),
                  callbacks.end());
  std::move(callbacks.begin(), callbacks.end(), std::back_inserter(_callbacks));
}

void Engine::Impl::updateActorIcons(const ngf::TimeSpan &elapsed) {
  auto screenSize = _pRoom->getScreenSize();
  auto screenMouse = toDefaultView((glm::ivec2) _mousePos, screenSize);
  _actorIcons.setMousePosition(screenMouse);
  _actorIcons.update(elapsed);
}

void Engine::Impl::updateMouseCursor() {
  auto flags = getFlags(_objId1);
  auto screen = _pApp->getRenderTarget()->getView().getSize();
  _cursorDirection = CursorDirection::None;
  if ((_mousePos.x < 20) || (flags & ObjectFlagConstants::DOOR_LEFT) == ObjectFlagConstants::DOOR_LEFT)
    _cursorDirection |= CursorDirection::Left;
  else if ((_mousePos.x > screen.x - 20) ||
      (flags & ObjectFlagConstants::DOOR_RIGHT) == ObjectFlagConstants::DOOR_RIGHT)
    _cursorDirection |= CursorDirection::Right;
  if ((flags & ObjectFlagConstants::DOOR_FRONT) == ObjectFlagConstants::DOOR_FRONT)
    _cursorDirection |= CursorDirection::Down;
  else if ((flags & ObjectFlagConstants::DOOR_BACK) == ObjectFlagConstants::DOOR_BACK)
    _cursorDirection |= CursorDirection::Up;
  if ((_cursorDirection == CursorDirection::None) && _objId1)
    _cursorDirection |= CursorDirection::Hotspot;
}

Entity *Engine::Impl::getHoveredEntity(const glm::vec2 &mousPos) {
  Entity *pCurrentObject = nullptr;

  // mouse on actor ?
  for (auto &&actor : _actors) {
    if (actor.get() == _pCurrentActor)
      continue;
    if (actor->getRoom() != _pRoom)
      continue;

    if (actor->contains(mousPos)) {
      if (!pCurrentObject || actor->getZOrder() < pCurrentObject->getZOrder()) {
        pCurrentObject = actor.get();
      }
    }
  }

  // mouse on object ?
  const auto &objects = _pRoom->getObjects();
  std::for_each(objects.cbegin(), objects.cend(), [mousPos, &pCurrentObject](const auto &pObj) {
    if (!pObj->isTouchable())
      return;
    auto rect = pObj->getRealHotspot();
    if (!rect.contains((glm::ivec2) mousPos))
      return;
    if (!pCurrentObject || pObj->getZOrder() <= pCurrentObject->getZOrder())
      pCurrentObject = pObj.get();
  });

  if (!pCurrentObject && _pRoom && _pRoom->getFullscreen() != 1) {
    // mouse on inventory object ?
    pCurrentObject = _hud.getInventory().getCurrentInventoryObject();
  }

  return pCurrentObject;
}

void Engine::Impl::updateHoveredEntity(bool isRightClick) {
  _hud.setVerbOverride(nullptr);
  if (!_hud.getCurrentVerb()) {
    _hud.setCurrentVerb(_hud.getVerb(VerbConstants::VERB_WALKTO));
  }

  if (_pUseObject) {
    _objId1 = _pUseObject ? _pUseObject->getId() : 0;
    _pObj2 = _hud.getHoveredEntity();
  } else {
    _objId1 = _hud.getHoveredEntity() ? _hud.getHoveredEntity()->getId() : 0;
    _pObj2 = nullptr;
  }

  // abort some invalid actions
  if (!_objId1 || !_hud.getCurrentVerb()) {
    return;
  }

  if (_pObj2 && _pObj2->getId() == _objId1) {
    _pObj2 = nullptr;
  }

  if (_objId1 && isRightClick) {
    _hud.setVerbOverride(_hud.getVerb(EntityManager::getScriptObjectFromId<Entity>(_objId1)->getDefaultVerb(
        VerbConstants::VERB_LOOKAT)));
  }

  auto verbId = _hud.getCurrentVerb()->id;
  switch (verbId) {
  case VerbConstants::VERB_WALKTO: {
    auto pObj1 = EntityManager::getScriptObjectFromId<Entity>(_objId1);
    if (pObj1 && pObj1->isInventoryObject()) {
      _hud.setVerbOverride(_hud.getVerb(EntityManager::getScriptObjectFromId<Entity>(_objId1)->getDefaultVerb(
          VerbConstants::VERB_LOOKAT)));
    }
    break;
  }
  case VerbConstants::VERB_TALKTO:
    // select actor/object only if talkable flag is set
    if (!hasFlag(_objId1, ObjectFlagConstants::TALKABLE)) {
      _objId1 = 0;
    }
    break;
  case VerbConstants::VERB_GIVE: {
    auto pObj1 = EntityManager::getScriptObjectFromId<Entity>(_objId1);
    if (!pObj1->isInventoryObject())
      _objId1 = 0;

    // select actor/object only if giveable flag is set
    if (_pObj2 && !hasFlag(_pObj2->getId(), ObjectFlagConstants::GIVEABLE))
      _pObj2 = nullptr;
    break;
  }
  default: {
    auto pActor = EntityManager::getScriptObjectFromId<Actor>(_objId1);
    if (pActor) {
      _objId1 = 0;
    }
    break;
  }
  }
}

Entity *Engine::Impl::getEntity(Entity *pEntity) const {
  if (!pEntity)
    return nullptr;

  // if an actor has the same name then get its flags
  auto itActor = std::find_if(_actors.begin(), _actors.end(), [pEntity](const auto &pActor) -> bool {
    return pActor->getName() == pEntity->getName();
  });
  if (itActor != _actors.end()) {
    return itActor->get();
  }
  return pEntity;
}

bool Engine::Impl::hasFlag(int id, uint32_t flagToTest) const {
  auto pObj = EntityManager::getScriptObjectFromId<Entity>(id);
  auto flags = getFlags(pObj);
  if (flags & flagToTest)
    return true;
  auto pActor = getEntity(pObj);
  flags = getFlags(pActor);
  return flags & flagToTest;
}

uint32_t Engine::Impl::getFlags(int id) const {
  auto pEntity = EntityManager::getScriptObjectFromId<Entity>(id);
  return getFlags(pEntity);
}

uint32_t Engine::Impl::getFlags(Entity *pEntity) const {
  if (pEntity)
    return pEntity->getFlags();
  return 0;
}

void Engine::Impl::updateRoomScalings() const {
  auto actor = _pCurrentActor;
  if (!actor)
    return;

  auto &scalings = _pRoom->getScalings();
  auto &objects = _pRoom->getObjects();
  for (auto &&object : objects) {
    if (object->getType() != ObjectType::Trigger)
      continue;
    if (object->getRealHotspot().contains((glm::ivec2) actor->getPosition())) {
      auto it = std::find_if(scalings.begin(), scalings.end(), [&object](const auto &s) -> bool {
        return s.getName() == object->getName();
      });
      if (it != scalings.end()) {
        _pRoom->setRoomScaling(*it);
        return;
      }
    }
  }
  if (!scalings.empty()) {
    _pRoom->setRoomScaling(scalings[0]);
  }
}

const Verb *Engine::Impl::getHoveredVerb() const {
  if (!_hud.getActive())
    return nullptr;
  if (_pRoom && _pRoom->getFullscreen() == 1)
    return nullptr;

  return _hud.getHoveredVerb();
}

void Engine::Impl::stopTalking() const {
  for (auto &&a : _pEngine->getActors()) {
    a->stopTalking();
  }
  for (auto &&a : _pEngine->getRoom()->getObjects()) {
    a->stopTalking();
  }
}

void Engine::Impl::stopTalkingExcept(Entity *pEntity) const {
  for (auto &&a : _pEngine->getActors()) {
    if (a.get() == pEntity)
      continue;
    a->stopTalking();
  }

  for (auto &&a : _pEngine->getRoom()->getObjects()) {
    if (a.get() == pEntity)
      continue;
    a->stopTalking();
  }
}

void Engine::Impl::updateKeys() {
  ImGuiIO &io = ImGui::GetIO();
  if (io.WantTextInput)
    return;

  const auto &cmdMgr = Locator<CommandManager>::get();
  for (auto &key : _oldKeyDowns) {
    if (isKeyPressed(key)) {
      cmdMgr.execute(key);
      cmdMgr.execute(key, false);
    }
  }

  for (auto &key : _newKeyDowns) {
    if (_oldKeyDowns.find(key) != _oldKeyDowns.end()) {
      cmdMgr.execute(key, true);
    }
  }

  _oldKeyDowns.clear();
  for (auto key : _newKeyDowns) {
    _oldKeyDowns.insert(key);
  }
}

bool Engine::Impl::isKeyPressed(const Input &key) {
  auto wasDown = _oldKeyDowns.find(key) != _oldKeyDowns.end();
  auto isDown = _newKeyDowns.find(key) != _newKeyDowns.end();
  return wasDown && !isDown;
}

InputConstants Engine::Impl::toKey(const std::string &keyText) {
  if (keyText.length() == 1) {
    return static_cast<InputConstants>(keyText[0]);
  }
  return InputConstants::NONE;
}

void Engine::Impl::updateKeyboard() {
  if (_oldKeyDowns.empty())
    return;

  if (_pRoom) {
    for (auto key : _oldKeyDowns) {
      if (isKeyPressed(key) && ScriptEngine::rawExists(_pRoom, "pressedKey")) {
        ScriptEngine::rawCall(_pRoom, "pressedKey", static_cast<int>(key.input));
      }
    }
  }

  int currentActorIndex = getCurrentActorIndex();
  if (currentActorIndex == -1)
    return;

  const auto &verbSlot = _hud.getVerbSlot(currentActorIndex);
  for (auto i = 0; i < 10; i++) {
    const auto &verb = verbSlot.getVerb(i);
    if (verb.key.length() == 0)
      continue;
    auto id = std::strtol(verb.key.substr(1, verb.key.length() - 1).c_str(), nullptr, 10);
    auto key = toKey(tostring(ng::Engine::getText(id)));
    if (isKeyPressed(key)) {
      onVerbClick(&verb);
    }
  }
}

void Engine::Impl::onVerbClick(const Verb *pVerb) {
  _hud.setCurrentVerb(pVerb);
  _useFlag = UseFlag::None;
  _pUseObject = nullptr;
  _objId1 = 0;
  _pObj2 = nullptr;

  ScriptEngine::rawCall("onVerbClick");
}

bool Engine::Impl::clickedAt(const glm::vec2 &pos) const {
  if (!_pRoom)
    return false;

  bool handled = false;
  if (ScriptEngine::rawExists(_pRoom, _clickedAtCallback)) {
    ScriptEngine::rawCallFunc(handled, _pRoom, _clickedAtCallback, pos.x, pos.y);
    if (handled)
      return true;
  }

  if (!_pCurrentActor)
    return false;

  if (!ScriptEngine::rawExists(_pCurrentActor, _clickedAtCallback))
    return false;

  ScriptEngine::rawCallFunc(handled, _pCurrentActor, _clickedAtCallback, pos.x, pos.y);
  return handled;
}

void Engine::Impl::drawWalkboxes(ngf::RenderTarget &target) const {
  if (!_pRoom || _showDrawWalkboxes == 0)
    return;

  auto at = _camera.getRect().getTopLeft();
  ngf::Transform t;
  t.setPosition(-at);
  ngf::RenderStates states;
  states.transform = t.getTransform();

  if (_showDrawWalkboxes & 4) {
    for (const auto &walkbox : _pRoom->getWalkboxes()) {
      WalkboxDrawable wd(walkbox, _pRoom->getScreenSize().y);
      wd.draw(target, states);
    }
  }

  if (_showDrawWalkboxes & 1) {
    for (const auto &walkbox : _pRoom->getGraphWalkboxes()) {
      WalkboxDrawable wd(walkbox, _pRoom->getScreenSize().y);
      wd.draw(target, states);
    }
  }

  if (_showDrawWalkboxes & 2) {
    const auto *pGraph = _pRoom->getGraph();
    if (pGraph) {
      auto height = _pRoom->getRoomSize().y;
      ng::GraphDrawable d(*pGraph, height);
      d.draw(target, states);
    }
  }
}

void Engine::Impl::drawPause(ngf::RenderTarget &target) const {
  if (_state != EngineState::Paused)
    return;

  const auto view = target.getView();
  auto viewRect = ngf::frect::fromPositionSize({0, 0}, {320, 176});
  target.setView(ngf::View(viewRect));

  auto &saveLoadSheet = Locator<ResourceManager>::get().getSpriteSheet("SaveLoadSheet");
  auto viewCenter = glm::vec2(viewRect.getWidth() / 2, viewRect.getHeight() / 2);
  auto rect = saveLoadSheet.getRect("pause_dialog");

  ngf::Sprite sprite;
  sprite.getTransform().setPosition(viewCenter);
  sprite.setTexture(*saveLoadSheet.getTexture());
  sprite.getTransform().setOrigin({rect.getWidth() / 2.f, rect.getHeight() / 2.f});
  sprite.setTextureRect(rect);
  sprite.draw(target, {});

  viewRect = ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height});
  viewCenter = glm::vec2(viewRect.getWidth() / 2, viewRect.getHeight() / 2);
  target.setView(ngf::View(viewRect));

  auto retroFonts =
      _pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts, PreferenceDefaultValues::RetroFonts);
  auto &font = _pEngine->getResourceManager().getFont(retroFonts ? "FontRetroSheet" : "FontModernSheet");

  Text text;
  auto screen = target.getView().getSize();
  auto scale = screen.y / 512.f;
  text.getTransform().setScale({scale, scale});
  text.getTransform().setPosition(viewCenter);
  text.setFont(font);
  text.setColor(ngf::Colors::White);
  text.setWideString(Engine::getText(99951));
  auto bounds = getGlobalBounds(text);
  text.getTransform().move({-bounds.getWidth() / 2.f, -scale * bounds.getHeight() / 2.f});
  text.draw(target, {});

  target.setView(view);
}

void Engine::Impl::stopThreads() {
  _threads.erase(std::remove_if(_threads.begin(), _threads.end(), [](const auto &t) -> bool {
    return !t || t->isStopped();
  }), _threads.end());
}

void Engine::Impl::drawCursor(ngf::RenderTarget &target) const {
  if (!_cursorVisible)
    return;
  if (!_showCursor && _dialogManager.getState() != DialogManagerState::WaitingForChoice)
    return;

  auto cursorSize = glm::vec2(68.f, 68.f);
  const auto &gameSheet = Locator<ResourceManager>::get().getSpriteSheet("GameSheet");

  const auto view = target.getView();
  target.setView(ngf::View(ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height})));

  auto screenSize = _pRoom->getScreenSize();
  auto pos = toDefaultView((glm::ivec2) _mousePos, screenSize);

  ngf::RectangleShape shape;
  shape.getTransform().setPosition(pos);
  shape.getTransform().setOrigin(cursorSize / 2.f);
  shape.setSize(cursorSize);
  shape.setTexture(*gameSheet.getTexture(), false);
  shape.setTextureRect(getCursorRect());
  shape.draw(target, {});

  target.setView(view);
}

ngf::irect Engine::Impl::getCursorRect() const {
  auto &gameSheet = Locator<ResourceManager>::get().getSpriteSheet("GameSheet");
  if (_state == EngineState::Paused)
    return gameSheet.getRect("cursor_pause");

  if (_state == EngineState::Options)
    return gameSheet.getRect("cursor");

  if (_dialogManager.getState() != DialogManagerState::None)
    return gameSheet.getRect("cursor");

  if (_cursorDirection & CursorDirection::Left) {
    return _cursorDirection & CursorDirection::Hotspot ? gameSheet.getRect("hotspot_cursor_left")
                                                       : gameSheet.getRect("cursor_left");
  }
  if (_cursorDirection & CursorDirection::Right) {
    return _cursorDirection & CursorDirection::Hotspot ? gameSheet.getRect("hotspot_cursor_right")
                                                       : gameSheet.getRect("cursor_right");
  }
  if (_cursorDirection & CursorDirection::Up) {
    return _cursorDirection & CursorDirection::Hotspot ? gameSheet.getRect("hotspot_cursor_back")
                                                       : gameSheet.getRect("cursor_back");
  }
  if (_cursorDirection & CursorDirection::Down) {
    return (_cursorDirection & CursorDirection::Hotspot) ? gameSheet.getRect("hotspot_cursor_front")
                                                         : gameSheet.getRect("cursor_front");
  }
  return (_cursorDirection & CursorDirection::Hotspot) ? gameSheet.getRect("hotspot_cursor")
                                                       : gameSheet.getRect("cursor");
}

std::wstring Engine::Impl::getDisplayName(const std::wstring &name) {
  std::wstring displayName(name);
  auto len = displayName.length();
  if (len > 1 && displayName[0] == '^') {
    displayName = name.substr(1, len - 1);
  }
  if (len > 2 && displayName[len - 2] == '#') {
    displayName = name.substr(0, len - 2);
  }
  return displayName;
}

const Verb *Engine::Impl::overrideVerb(const Verb *pVerb) const {
  if (!pVerb || pVerb->id != VerbConstants::VERB_WALKTO)
    return pVerb;

  auto pObj1 = EntityManager::getScriptObjectFromId<Entity>(_objId1);
  if (!pObj1)
    return pVerb;
  return _hud.getVerb(pObj1->getDefaultVerb(VerbConstants::VERB_WALKTO));
}

void Engine::Impl::drawCursorText(ngf::RenderTarget &target) const {
  if (!_cursorVisible)
    return;
  if (!_showCursor || _state != EngineState::Game)
    return;

  if (_dialogManager.getState() != DialogManagerState::None)
    return;

  auto pVerb = _hud.getVerbOverride();
  if (!pVerb)
    pVerb = _hud.getCurrentVerb();
  if (!pVerb)
    return;

  pVerb = overrideVerb(pVerb);

  auto currentActorIndex = getCurrentActorIndex();
  if (currentActorIndex == -1)
    return;

  auto classicSentence = _pEngine->getPreferences().getUserPreference(PreferenceNames::ClassicSentence,
                                                                      PreferenceDefaultValues::ClassicSentence);

  const auto view = target.getView();
  target.setView(ngf::View(ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height})));

  auto retroFonts =
      _pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts, PreferenceDefaultValues::RetroFonts);
  auto &font = _pEngine->getResourceManager().getFont(retroFonts ? "FontRetroSheet" : "FontModernSheet");

  std::wstring s;
  if (pVerb->id != VerbConstants::VERB_WALKTO || _hud.getHoveredEntity()) {
    auto id = std::strtol(pVerb->text.substr(1).data(), nullptr, 10);
    s.append(ng::Engine::getText(id));
  }
  auto pObj1 = EntityManager::getScriptObjectFromId<Entity>(_objId1);
  if (pObj1) {
    s.append(L" ").append(getDisplayName(ng::Engine::getText(pObj1->getName())));
    if (DebugFeatures::showHoveredObject) {
      if (pObj1) {
        s.append(L"(").append(towstring(pObj1->getKey())).append(L")");
      }
    }
  }
  appendUseFlag(s);
  if (_pObj2) {
    s.append(L" ").append(getDisplayName(ng::Engine::getText(_pObj2->getName())));
  }

  Text text;
  text.setFont(font);
  text.setColor(_hud.getVerbUiColors(currentActorIndex).sentence);
  text.setWideString(s);

  // do display cursor position:
  if (DebugFeatures::showCursorPosition) {
    std::wstringstream ss;
    std::wstring txt = text.getWideString();
    ss << txt << L" (" << std::fixed << std::setprecision(0) << _mousePosInRoom.x << L"," << _mousePosInRoom.y
       << L")";
    text.setWideString(ss.str());
  }

  auto screenSize = _pRoom->getScreenSize();
  auto pos = toDefaultView((glm::ivec2) _mousePos, screenSize);

  auto bounds = getGlobalBounds(text);
  if (classicSentence) {
    auto y = Screen::Height - 210.f;
    auto x = Screen::HalfWidth - bounds.getWidth() / 2.f;
    text.getTransform().setPosition({x, y});
  } else {
    auto y = pos.y - 20 < 40 ? pos.y + 80 : pos.y - 40;
    auto x = std::clamp<float>(pos.x - bounds.getWidth() / 2.f, 20.f, Screen::Width - 20.f - bounds.getWidth());
    text.getTransform().setPosition({x, y - bounds.getHeight()});
  }
  text.draw(target, {});
  target.setView(view);
}

void Engine::Impl::drawNoOverride(ngf::RenderTarget &target) const {
  if (_noOverrideElapsed > ngf::TimeSpan::seconds(2))
    return;

  auto &gameSheet = Locator<ResourceManager>::get().getSpriteSheet("GameSheet");
  const auto view = target.getView();
  target.setView(ngf::View(ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height})));

  ngf::Color c(ngf::Colors::White);
  c.a = static_cast<sf::Uint8>((2.f - _noOverrideElapsed.getTotalSeconds() / 2.f) * 255);
  ngf::Sprite spriteNo;
  spriteNo.setColor(c);
  spriteNo.getTransform().setPosition({8.f, 8.f});
  spriteNo.getTransform().setScale({2.f, 2.f});
  spriteNo.setTexture(*gameSheet.getTexture());
  spriteNo.setTextureRect(gameSheet.getRect("icon_no"));
  spriteNo.draw(target, {});

  target.setView(view);
}

void Engine::Impl::appendUseFlag(std::wstring &sentence) const {
  switch (_useFlag) {
  case UseFlag::UseWith:sentence.append(L" ").append(ng::Engine::getText(10000));
    break;
  case UseFlag::UseOn:sentence.append(L" ").append(ng::Engine::getText(10001));
    break;
  case UseFlag::UseIn:sentence.append(L" ").append(ng::Engine::getText(10002));
    break;
  case UseFlag::GiveTo:sentence.append(L" ").append(ng::Engine::getText(10003));
    break;
  case UseFlag::None:break;
  }
}

int Engine::Impl::getCurrentActorIndex() const {
  for (int i = 0; i < static_cast<int>(_actorsIconSlots.size()); i++) {
    const auto &selectableActor = _actorsIconSlots.at(i);
    if (selectableActor.pActor == _pCurrentActor) {
      return i;
    }
  }
  return -1;
}

void Engine::Impl::drawHud(ngf::RenderTarget &target) const {
  if (_state != EngineState::Game)
    return;

  _hud.draw(target, {});
}

void Engine::Impl::captureScreen(const std::string &path) const {
  ngf::RenderTexture target({320, 180});
  _pEngine->draw(target, true);
  target.display();

  auto screenshot = target.capture();
  screenshot.saveToFile(path);
}

}