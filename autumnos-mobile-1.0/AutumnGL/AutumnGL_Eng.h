
#ifndef OPENGL_H
#define OPENGL_H

#include "AutumnGL_Math.h"
#include <GLES3/gl3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
typedef struct {
	int x;
	int y;
} ivec2;

typedef struct {
	float x;
	float y;
	float z;
} vec3;

typedef struct {
	GLuint TextureID;
	ivec2  Size;
	ivec2  Bearing;
	unsigned int Advance;
} FontChar;

static const char* CIRCLE_FRAG = 
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec2 v_uv;\n"
    "out vec4 fragColor;\n"
    "uniform vec3 u_color;\n"
    "uniform float u_opacity;\n"
    "void main() {\n"
    "    float dist = distance(v_uv, vec2(0.5));\n"
    "    float alpha = 1.0 - smoothstep(0.48, 0.5, dist);\n"
    "    if(alpha < 0.01) discard;\n"
    "    fragColor = vec4(u_color, alpha * u_opacity);\n"
    "}\n";

static const char* TRIANGLE_FRAG = 
    "#version 300 es\n"
    "precision mediump float;\n"
    "uniform vec4 u_color;\n"
    "out vec4 fragColor;\n"
    "void main() { fragColor = u_color; }\n";

static const char* RECT_FRAG =
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec2 v_uv;\n"
    "out vec4 fragColor;\n"
    "uniform vec3 u_topCol;\n"
    "uniform vec3 u_botCol;\n"
    "uniform float u_opacity;\n"
    "void main() {\n"
    "    fragColor = vec4(mix(u_botCol, u_topCol, v_uv.y), u_opacity);\n"
    "}\n";

static const char* ROUND_GRD_RECT_FRAG = 
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec2 v_uv;\n"
    "out vec4 fragColor;\n"
    "uniform vec2 u_size;\n"
    "uniform float u_radius;\n"
    "uniform vec3 u_topCol;\n"
    "uniform vec3 u_botCol;\n"
    "uniform vec3 u_edgeCol;\n"
    "uniform float u_edgeThick;\n"
    "uniform float u_opacity;\n"
    "float sdRoundRect(vec2 p, vec2 b, float r) {\n"
    "    vec2 d = abs(p) - b + r;\n"
    "    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - r;\n"
    "}\n"
    "void main() {\n"
    "    vec2 p = v_uv - 0.5;\n"
    "    float d = sdRoundRect(p, vec2(0.45), u_radius);\n"
    "    float shadow = 1.0 - smoothstep(0.0, 0.1, d);\n"
    "    vec4 shadowCol = vec4(0.0, 0.0, 0.0, shadow * 0.3);\n"
    "    vec3 fillColor = mix(u_botCol, u_topCol, v_uv.y);\n"
    "    float fillMask = 1.0 - smoothstep(-0.005, 0.005, d);\n"
    "    float border = smoothstep(-u_edgeThick - 0.005, -u_edgeThick, d) - smoothstep(-0.005, 0.005, d);\n"
    "    vec4 res = vec4(fillColor, fillMask);\n"
    "    res.rgb = mix(res.rgb, u_edgeCol, border);\n"
    "    fragColor = vec4(mix(shadowCol.rgb, res.rgb, res.a), max(shadowCol.a, res.a) * u_opacity);\n"
    "}\n";

static const char* ROUNDED_FRM_RECT_FRAG = 
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec2 v_uv;\n"
    "out vec4 fragColor;\n"
    "uniform vec2 u_size;\n"
    "uniform float u_radius;\n"
    "uniform vec3 u_topCol;\n"
    "uniform vec3 u_botCol;\n"
    "uniform float u_opacity;\n"
    "float sdRoundRect(vec2 p, vec2 b, float r) {\n"
    "    vec2 d = abs(p) - b + r;\n"
    "    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - r;\n"
    "}\n"
    "void main() {\n"
    "    vec2 p = v_uv - 0.5;\n"
    "    float d = sdRoundRect(p, vec2(0.5) - 0.01, u_radius);\n"
    "    if(d > 0.0) discard;\n"
    "    vec3 finalColor = mix(u_botCol, u_topCol, v_uv.y);\n"
    "    fragColor = vec4(finalColor, u_opacity);\n"
    "}\n";

static const char* ICO_FRAG = 
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec2 v_uv;\n"
    "out vec4 fragColor;\n"
    "uniform sampler2D u_texture;\n"
    "uniform float u_opacity;\n"
    "void main() {\n"
    "    vec4 tex = texture(u_texture, v_uv);\n"
    "    fragColor = vec4(tex.rgb, tex.a * u_opacity);\n"
    "}\n";

static const char* FONT_FRAG = 
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec2 v_uv;\n"
    "out vec4 fragColor;\n"
    "uniform sampler2D u_texture;\n"
    "uniform vec3 u_color;\n"
    "uniform float u_opacity;\n"
    "void main() {\n"
    "    float sampled = texture(u_texture, v_uv).r;\n"
    "    fragColor = vec4(u_color, sampled * u_opacity);\n"
    "}\n";

static const char* SHADOW_FRAG = 
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec2 v_uv;\n"
    "out vec4 fragColor;\n"
    "uniform vec2 u_size;\n"
    "uniform float u_radius;\n"
    "uniform float u_softness;\n"
    "uniform float u_opacity;\n"
    "float sdRoundRect(vec2 p, vec2 b, float r) {\n"
    "    vec2 d = abs(p) - b + r;\n"
    "    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - r;\n"
    "}\n"
    "void main() {\n"
    "    vec2 p = v_uv - 0.5;\n"
    "    float d = sdRoundRect(p, vec2(0.5) - u_softness, u_radius);\n"
    "    float shadow = 1.0 - smoothstep(-u_softness, u_softness, d);\n"
    "    fragColor = vec4(0.0, 0.0, 0.0, shadow * u_opacity);\n"
    "}\n";

static const char* CUBE_FRAG =
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec2 v_uv;\n"
    "out vec4 fragColor;\n"
    "uniform sampler2D u_texture;\n"
    "uniform float u_opacity;\n"
    "void main() {\n"
    "    vec4 tex = texture(u_texture, v_uv);\n"
    "    fragColor = vec4(tex.rgb, tex.a * u_opacity);\n"
    "}\n";

void AutumnGL_Init();
void AutumnGL_DrawRect(float x, float y, float w, float h, float opacity, float tr, float tg, float tb, float br, float bg, float bb);
void AutumnGL_DrawRoundedRect(float x, float y, float w, float h, float r, float thickness, float opacity, float rr, float gg, float bb);
void AutumnGL_DrawFramedRoundedRect(float x, float y, float w, float h, float r, float tr, float tg, float tb, float br, float bg, float bb, float er, float eg, float eb, float thickness, float opacity);
void AutumnGL_DrawText(const char *text, float x, float y, float scale, float rr, float gg, float bb, float opacity); 
void AutumnGL_DrawShadow(float x, float y, float w, float h, float r, float soft, float opacity);
void AutumnGL_DrawCircle(float x, float y, float radius, float r, float g, float b, float opacity);
void AutumnGL_DrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3, float r, float g, float b, float opacity);
void AutumnGL_DrawCube(GLuint texID, float x, float y, float z, float size, float angle);

#endif

