#include	<iostream>
#include    <cmath>
using namespace std;

//#define FREEGLUT_STATIC
//#define GLEW_STATIC
#include <gl/glew.h>
#include <gl/freeglut.h>
#include "gTransform.h"

struct VecColor4f { // 색을 표현하기 위한 구조체
    float r, g, b, a;
    VecColor4f() {}
    VecColor4f(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
};

struct Vertex {
    Vec3f       mCoord;
    VecColor4f  mColor;
};

enum { CUBE, N_VBOs };
GLuint VBO[N_VBOs];         // 꼭짓점 버퍼 객체
GLuint uGTransLocation;     // 기하변환 행렬 유니폼의 위치

static const char* pVS =    // 꼭짓점 쉐이더 소스
"#version 330                                                      \n"
"layout (location = 0) in vec3 Position;                           \n"
"layout (location = 1) in vec4 VColor;                             \n"
"out vec4 Color;                                                   \n"
"uniform    mat4    gTransform;                                    \n"
"                                                                  \n"
"void main()                                                       \n"
"{                                                                 \n"
"    gl_Position = gTransform * vec4(Position, 1.0);               \n"
"    Color = VColor;                                               \n"
"}";

static const char* pFS =    // 프래그먼트 쉐이더 소스
"#version 330                                                      \n"
"in vec4 Color;                                                    \n"
"out vec4 FragColor;                                               \n"
"                                                                  \n"
"void main()                                                       \n"
"{                                                                 \n"
"    FragColor = Color;                                            \n"
"}";

static void AddShader(GLuint shaderProg, const char* pShaderSrc, GLint ShaderType)
{   // 쉐이더 생성
    GLuint shader = glCreateShader(ShaderType);
    if (!shader) {
        cerr << "오류 - Shader 생성(" << ShaderType << ")" << endl;
        exit(0);
    }
    // 쉐이더 컴파일
    const GLchar* src[1] = { pShaderSrc };
    const GLint len[1] = { strlen(pShaderSrc) };
    glShaderSource(shader, 1, src, len);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {		// 컴파일 오류 발생
        GLchar infoLog[256];
        glGetShaderInfoLog(shader, 256, NULL, infoLog);
        cerr << "오류 - Shader 컴파일(" << ShaderType << "): " << infoLog << endl;
        exit(1);
    }
    // 쉐이더 프로그램에 컴파일된 쉐이더를 추가
    glAttachShader(shaderProg, shader);
}

static void SetUpShaders()
{   // 쉐이더 프로그램 객체 생성
    GLuint shaderProg = glCreateProgram();
    if (!shaderProg) {
        cerr << "오류 - Shader 프로그램 생성" << endl;
        exit(1);
    }

    // 꼭짓점 쉐이더 및 프래그먼트 쉐이더 적재
    AddShader(shaderProg, pVS, GL_VERTEX_SHADER);
    AddShader(shaderProg, pFS, GL_FRAGMENT_SHADER);

    GLint success = 0;
    GLchar errLog[256];

    glLinkProgram(shaderProg);      // 쉐이더 프로그램 링크
    glGetProgramiv(shaderProg, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProg, sizeof(errLog), NULL, errLog);
        cerr << "오류 - Shader 프로그램 링크: " << errLog << endl;
        exit(1);
    }

    glValidateProgram(shaderProg);  // 프로그램 객체가 유효한지 검사
    glGetProgramiv(shaderProg, GL_VALIDATE_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProg, sizeof(errLog), NULL, errLog);
        cerr << "Invalid shader program: " << errLog << endl;
        exit(1);
    }
    glUseProgram(shaderProg);       // 현재 쉐이더 프로그램 객체로 지정
    uGTransLocation = glGetUniformLocation(shaderProg, "gTransform");
    if (uGTransLocation == 0xFFFFFFFF) {
        cerr << "Uniform location not found!!" << endl;
        exit(1);
    }
}

static void RenderCB()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);   // 백색으로 화면 지움

    TransMat4f  viewMat;
    viewMat.viewingTrans(Vec3f(150.0f, 120.0f, 100.0f), Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 1.0f, 0.0f));
    TransMat4f  perspectiveMat;
    perspectiveMat.perspectivePrj(-50.0f, 50.0f, -50.0f, 50.0f, 70.0f, 350.0f);
    TransMat4f  tMat(perspectiveMat*viewMat);
    glUniformMatrix4fv(uGTransLocation, 1, GL_TRUE, tMat.getPMat());

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[CUBE]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)sizeof(Vec3f));
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glDrawArrays(GL_LINE_LOOP, 4, 4);
    glDrawArrays(GL_LINE_LOOP, 8, 4);
    glDrawArrays(GL_LINE_LOOP, 12, 4);
    glDrawArrays(GL_LINE_LOOP, 16, 4);
    glDrawArrays(GL_LINE_LOOP, 20, 4);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glutSwapBuffers();
}

static void InitVBOs()
{
    Vertex Vertices[24] = {     // 육면체형의 꼭짓점 좌표 및 색
        { Vec3f(-50.0f, -50.0f, -50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },
        { Vec3f(-50.0f, 50.0f, -50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },
        { Vec3f(50.0f, 50.0f, -50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },
        { Vec3f(50.0f, -50.0f, -50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },

        { Vec3f(-50.0f, -50.0f, 50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },
        { Vec3f(50.0f, -50.0f, 50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },
        { Vec3f(50.0f, 50.0f, 50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },
        { Vec3f(-50.0f, 50.0f, 50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },

        { Vec3f(-50.0f, -50.0f, -50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },
        { Vec3f(-50.0f, -50.0f, 50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },
        { Vec3f(-50.0f, 50.0f, 50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },
        { Vec3f(-50.0f, 50.0f, -50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },

        { Vec3f(50.0f, -50.0f, -50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },
        { Vec3f(50.0f, 50.0f, -50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },
        { Vec3f(50.0f, 50.0f, 50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },
        { Vec3f(50.0f, -50.0f, 50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },

        { Vec3f(-50.0f, -50.0f, -50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },
        { Vec3f(50.0f, -50.0f, -50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },
        { Vec3f(50.0f, -50.0f, 50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },
        { Vec3f(-50.0f, -50.0f, 50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },

        { Vec3f(-50.0f, 50.0f, -50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },
        { Vec3f(-50.0f, 50.0f, 50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },
        { Vec3f(50.0f, 50.0f, 50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) },
        { Vec3f(50.0f, 50.0f, -50.0f), VecColor4f(0.0f, 0.0f, 0.0f, 1.0f) } };

    // 꼭짓점 버퍼를 생성하여 삼각형의 꼭짓점 정보 전달
    glGenBuffers(N_VBOs, VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[CUBE]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(50, 100);
    glutInitWindowSize(400, 400);
    glutCreateWindow("원근투영");

    GLenum s = glewInit();
    if (s != GLEW_OK) {
        cerr << "오류 - " << glewGetErrorString(s) << endl;
        return 1;
    }

    SetUpShaders();
    InitVBOs();

    glutDisplayFunc(RenderCB);
    glutIdleFunc(RenderCB);
    glutMainLoop();

    return 0;
}
