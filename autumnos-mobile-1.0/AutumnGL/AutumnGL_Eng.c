#include "AutumnGL_Eng.h"
#include "AutumnGL_Math.h"
#include <stdio.h>
#include <stdlib.h>

float screenWidth = 800.0f;
float scrollPos = 0.0f;
float targetPos = 0.0f;
float scrollSpeed = 0.12f;
int currentTileIndex = 0;
GLuint rectProg, circleProg, cubeProg, triangleProg, roundFrameProg, iconProg, fontProg, shadowProg, framedgradrectProg;
GLuint triangleVBO, triangleVAO, cubeVAO, fontVAO, fontVBO;
GLint modelLoc;
FontChar Characters[128];

GLuint CompileShader(GLenum type, const char* source) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char log[512];
		glGetShaderInfoLog(shader,512, NULL, log);
		printf("Shader error: Type->%d, Detail->%s", type, log);
	}
	else {
		printf("Shaders is compiled, ID->%u/n", shader);
	}
	return shader;
}

GLuint CreateProgram(const char* vsSource, const char* fsSource) {
	GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSource);
	GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSource);
	GLuint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);
	GLint success;
	glGetProgramiv(prog, GL_LINK_STATUS, &success);
	if (!success) {
		char log[512];
		glGetProgramInfoLog(prog, 512, NULL, log);
		printf("Shader Program link error: Detail->%s\n", log);
	}
	else {
		printf("Shader Program is compiled, ID->%u/n", prog);
	}
	return prog;
}

static const char* COMMON_VERTEX =
	"#version 300 es\n"
	"precision mediump float;\n"
	"layout(location = 0) in vec2 a_position;\n"
	"layout(location = 1) in vec2 a_uv;\n"
	"out vec2 v_uv;\n"
	"void main() {\n"
	"	gl_Position = a_position;\n"
	"	v_uv = a_uv;\n"
	"}";

void AutumnGL_Init() {
    rectProg = CreateProgram(COMMON_VERTEX, RECT_FRAG);
    triangleProg = CreateProgram(COMMON_VERTEX, TRIANGLE_FRAG);
    circleProg = CreateProgram(COMMON_VERTEX, CIRCLE_FRAG);
    roundFrameProg = CreateProgram(COMMON_VERTEX, ROUND_GRD_RECT_FRAG);
    iconProg = CreateProgram(COMMON_VERTEX, ICO_FRAG);
    fontProg = CreateProgram(COMMON_VERTEX, FONT_FRAG);
    shadowProg = CreateProgram(COMMON_VERTEX, SHADOW_FRAG);
    framedgradrectProg = CreateProgram(COMMON_VERTEX, ROUNDED_FRM_RECT_FRAG);
    cubeProg = CreateProgram(COMMON_VERTEX, CUBE_FRAG);
    modelLoc = glGetUniformLocation(cubeProg, "u_model");
    printf("AutumnGL: shaders successfully installed. RectProg ID: %u/n", rectProg);
    
    glGenVertexArrays(1, &triangleVAO);
    glGenBuffers(1, &triangleVBO);

    glBindVertexArray(triangleVAO);

    glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

void _Autumn_RenderFT(const char* text, float x, float y, float scale, vec3 color) {
	glUseProgram(fontProg); glUniform1i(glGetUniformLocation(fontProg, "u_texture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(fontVAO);
	for (const char* c = text; *c; c++) {
		FontChar ch = Characters[(unsigned char)*c];
		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;
		float vertices[6][4] = {
            		{ xpos,     ypos + h,   0.0f, 0.0f },            
            		{ xpos,     ypos,       0.0f, 1.0f },
            		{ xpos + w, ypos,       1.0f, 1.0f },

            		{ xpos,     ypos + h,   0.0f, 0.0f },
            		{ xpos + w, ypos,       1.0f, 1.0f },
            		{ xpos + w, ypos + h,   1.0f, 0.0f }           
        	};

		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		glBindBuffer(GL_ARRAY_BUFFER, fontVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		x += (ch.Advance >> 6) * scale;
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void _Autumn_Pipe(float x, float y, float w, float h) {
    float x1 = (x / 480.0f) * 2.0f - 1.0f;
    float y1 = (y / 800.0f) * 2.0f - 1.0f;
    float x2 = ((x + w) / 480.0f) * 2.0f - 1.0f;
    float y2 = ((y + h) / 800.0f) * 2.0f - 1.0f;

    static GLfloat vertices[8];
    vertices[0] = x1;	vertices[1] = y2;
    vertices[2] = x1;	vertices[3] = y1;
    vertices[4] = x2;	vertices[5] = y2;
    vertices[6] = x2;	vertices[7] = y1;
    static GLfloat uvs[] = { 0, 1, 0, 0, 1, 1, 1, 0 };

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, uvs);
}

void AutumnGL_DrawRect(float x, float y, float w, float h, float opacity, float tr, float tg, float tb, float br, float bg, float bb) {
	glUseProgram(rectProg);
	glUniform3f(glGetUniformLocation(rectProg, "u_topCol"), tr, tg, tb);
	glUniform3f(glGetUniformLocation(rectProg, "u_botCol"), br, bg, bb);
	glUniform1f(glGetUniformLocation(rectProg, "u_opacity"), opacity);
	_Autumn_Pipe(x, y, w, h);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void AutumnGL_DrawCircle(float x, float y, float radius, float r, float g, float b, float opacity) {
	glUseProgram(circleProg);
	glUniform3f(glGetUniformLocation(circleProg, "u_color"), r, g, b);
	glUniform1f(glGetUniformLocation(circleProg, "u_opacity"), opacity);
	float diameter = radius * 2.0f;
	_Autumn_Pipe(x - radius, y - radius, diameter, diameter);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void AutumnGL_DrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3, float r, float g, float b, float opacity) {
    glUseProgram(triangleProg);
    glUniform4f(glGetUniformLocation(triangleProg, "u_color"), r, g, b, opacity);
    
    float vertices[] = {
        x1, y1,
        x2, y2,
        x3, y3
    };

    glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindVertexArray(triangleVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

GLuint LoadTexture3D(int width, int height, int depth, unsigned char* data) {
	GLuint texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_3D, texID);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, width, height, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	return texID;
}


GLuint LoadTexture2D(unsigned char* data, int width, int height, GLenum format) {
	GLuint texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    return texID;
}


void AutumnGL_DrawRoundedRect(float x, float y, float w, float h, float r, float thickness, float opacity, float rr, float gg, float bb) {
	glUseProgram(roundFrameProg);
    glUniform2f(glGetUniformLocation(roundFrameProg, "u_size"), w, h);
	glUniform1f(glGetUniformLocation(roundFrameProg, "u_radius"), r / (w > h ? w : h));
    glUniform1f(glGetUniformLocation(roundFrameProg, "u_thickness"), thickness);
    glUniform3f(glGetUniformLocation(roundFrameProg, "u_color"), rr, gg, bb);
    glUniform1f(glGetUniformLocation(roundFrameProg, "u_opacity"), opacity);

    _Autumn_Pipe(x, y, w, h);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void AutumnGL_DrawFramedRoundedRect(float x, float y, float w, float h, float r, float tr, float tg, float tb,  float br, float bg, float bb, float er, float eg, float eb, float thickness, float opacity) {
	glUseProgram(framedgradrectProg);
    
    glUniform2f(glGetUniformLocation(framedgradrectProg, "u_size"), w, h);
    glUniform1f(glGetUniformLocation(framedgradrectProg, "u_radius"), r / (w > h ? w : h));

    glUniform3f(glGetUniformLocation(framedgradrectProg, "u_topCol"), tr, tg, tb);
    glUniform3f(glGetUniformLocation(framedgradrectProg, "u_botCol"), br, bg, bb);
    glUniform3f(glGetUniformLocation(framedgradrectProg, "u_edgeCol"), er, eg, eb);
    
    glUniform1f(glGetUniformLocation(framedgradrectProg, "u_edgeThick"), thickness);
    glUniform1f(glGetUniformLocation(framedgradrectProg, "u_opacity"), opacity);

    _Autumn_Pipe(x, y, w, h);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void AutumnGL_DrawIcon(GLuint texID, float x, float y, float w, float h, float opacity) {
	glUseProgram(iconProg);
	glBindTexture(GL_TEXTURE_2D, texID);
	glUniform1i(glGetUniformLocation(iconProg, "u_texture"), 0);
	glUniform1f(glGetUniformLocation(iconProg, "u_opacity"), opacity);
	_Autumn_Pipe(x, y, w, h);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void AutumnGL_DrawText(const char *text, float x, float y, float scale,float rr, float gg, float bb, float opacity) {
	glUseProgram(fontProg);
    glUniform1i(glGetUniformLocation(fontProg, "u_texture"), 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture(GL_TEXTURE0);
	glUniform3f(glGetUniformLocation(fontProg, "u_color"), rr, gg, bb);
	glUniform1f(glGetUniformLocation(fontProg, "u_opacity"), opacity);
	const char* p;
	for (p = text; *p; p++) {
		FontChar ch = Characters[(unsigned char)*p];
		float xpos = x + ch.Bearing.x * scale;
        	float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        	float w = ch.Size.x * scale;
        	float h = ch.Size.y * scale;

        	glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        	_Autumn_Pipe(xpos, ypos, w, h);
        
        	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        	x += (ch.Advance >> 6) * scale;
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

void AutumnGL_DrawCube(GLuint texID, float x, float y, float z, float size, float angle) {
	glUseProgram(cubeProg);
	glEnable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);
	glBindVertexArray(cubeVAO);
	mat4 model = CalculateModelMatrix(x, y, z, angle, size);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model.m);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void AutumnGL_DrawShadow(float x, float y, float w, float h, float r, float soft, float opacity) {
    glUseProgram(shadowProg);
    glUniform2f(glGetUniformLocation(shadowProg, "u_size"), w, h);
    glUniform1f(glGetUniformLocation(shadowProg, "u_radius"), r / (w > h ? w : h));
    glUniform1f(glGetUniformLocation(shadowProg, "u_softness"), soft / 100.0f);
    glUniform1f(glGetUniformLocation(shadowProg, "u_opacity"), opacity);

    _Autumn_Pipe(x, y, w, h);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}



