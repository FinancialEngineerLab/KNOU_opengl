#include	<iostream>
using namespace std;

//#define FREEGLUT_STATIC
//#define GLEW_STATIC
#include <gl/glew.h>
#include <gl/freeglut.h>
#include "gTransform.h"

struct VecColor4f {	// 색을 표현하기 위한 구조체
    float r, g, b, a;
    VecColor4f() {}
    VecColor4f(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
};

GLuint uGTransLocation;     // 기하변환 행렬 유니폼의 위치
GLuint uGColorLocation;     // 색 속성 유니폼의 위치

static const char* pVS =    // 꼭짓점 쉐이더 소스
"#version 330                                                      \n"
"layout (location = 0) in vec3 Position;                           \n"
"uniform    mat4    gTransform;                                    \n"
"                                                                  \n"
"void main()                                                       \n"
"{                                                                 \n"
"    gl_Position = gTransform * vec4(Position, 1.0);               \n"
"}";

static const char* pFS =    // 프래그먼트 쉐이더 소스
"#version 330                                                      \n"
"out vec4 FragColor;                                               \n"
"uniform    vec4    gColor;                                        \n"
"                                                                  \n"
"void main()                                                       \n"
"{                                                                 \n"
"    FragColor = gColor;                                           \n"
"}";

void AddShader(GLuint shaderProg, const char* pShaderSrc, GLint ShaderType)
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

void SetUpShaders()
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
    uGColorLocation = glGetUniformLocation(shaderProg, "gColor");
    if (uGTransLocation == 0xFFFFFFFF || uGColorLocation == 0xFFFFFFFF) {
        cerr << "Uniform location not found!!" << endl;
        exit(1);
    }
}

void RenderCB()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   // 화면과 깊이버퍼 지움

    TransMat4f  viewMat;
    viewMat.viewingTrans(Vec3f(2.0f, 2.0f, 2.0f), Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f));
    TransMat4f  orthoMat;
    orthoMat.orthgonalPrj(-2.0f, 2.0f, -2.0f, 2.0f, 1.0f, 5.0f);
    TransMat4f  tMat;

    TransMat4f  modelMat;
    modelMat.translate(Vec3f(1.0f, 1.0f, 0.0f));
    tMat = orthoMat * viewMat * modelMat;
    glUniformMatrix4fv(uGTransLocation, 1, GL_TRUE, tMat.getPMat());

    VecColor4f   col = VecColor4f(0.0f, 0.5f, 1.0f, 1.0f);
    glUniform4fv(uGColorLocation, 1, &col.r);
    glutSolidSphere(0.75, 20, 20);
    col = VecColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    glUniform4fv(uGColorLocation, 1, &col.r);
    glutWireSphere(0.75, 20, 20);

    modelMat.translate(Vec3f(1.0f, -0.5f, 0.5f));
    tMat = orthoMat * viewMat * modelMat;
    glUniformMatrix4fv(uGTransLocation, 1, GL_TRUE, tMat.getPMat());

    col = VecColor4f(0.7f, 0.0f, 1.0f, 1.0f);
    glUniform4fv(uGColorLocation, 1, &col.r);
    glutSolidCone(0.7, 2.0, 20, 20);
    col = VecColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    glUniform4fv(uGColorLocation, 1, &col.r);
    glutWireCone(0.7, 2.0, 20, 20);

    modelMat.translate(Vec3f(0.0f, 1.0f, 1.5f));
    tMat = orthoMat * viewMat * modelMat;
    glUniformMatrix4fv(uGTransLocation, 1, GL_TRUE, tMat.getPMat());

    col = VecColor4f(0.5f, 0.7f, 1.0f, 1.0f);
    glUniform4fv(uGColorLocation, 1, &col.r);
    glutSolidTorus(0.2, 0.6, 20, 20);
    col = VecColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    glUniform4fv(uGColorLocation, 1, &col.r);
    glutWireTorus(0.2, 0.6, 20, 20);

    glutSwapBuffers();
}

void Init()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition(50, 100);
    glutInitWindowSize(400, 400);
    glutCreateWindow("Wire-Frame Qadric Surfaces");

    GLenum s = glewInit();
    if (s != GLEW_OK) {
        cerr << "오류 - " << glewGetErrorString(s) << endl;
        return 1;
    }

    SetUpShaders();
    Init();

    glutDisplayFunc(RenderCB);
    glutIdleFunc(RenderCB);
    glutMainLoop();

    return 0;
}
