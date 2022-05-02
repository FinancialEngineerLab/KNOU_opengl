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

enum { B_SURFACE, N_VBOs };
GLuint VBO[N_VBOs];         // 꼭짓점 버퍼 객체
GLuint uGTransLocation;     // 기하변환 행렬 유니폼의 위치
GLuint uGColorLocation;     // 색 속성 유니폼의 위치

static const char* pVS =    // 꼭짓점 쉐이더 소스
"#version 400 core                                                 \n"
"layout (location = 0) in vec3 Position;                           \n"
"                                                                  \n"
"void main()                                                       \n"
"{                                                                 \n"
"    gl_Position = vec4(Position, 1.0);                            \n"
"}";

static const char* pTCS =   // 테셀레이션 제어 쉐이더 소스
"#version 400 core                                                 \n"
"layout(vertices = 16) out;                                        \n"
"                                                                  \n"
"void main()                                                       \n"
"{                                                                 \n"
"    gl_TessLevelInner[0] = 16;                                    \n"
"    gl_TessLevelInner[1] = 16;                                    \n"
"    gl_TessLevelOuter[0] = 16;                                    \n"
"    gl_TessLevelOuter[1] = 16;                                    \n"
"    gl_TessLevelOuter[2] = 16;                                    \n"
"    gl_TessLevelOuter[3] = 16;                                    \n"
"                                                                  \n"
"    gl_out[gl_InvocationID].gl_Position =                         \n"
"        gl_in[gl_InvocationID].gl_Position;                       \n"
"}";

static const char* pTES =   // 테셀레이션 계산 쉐이더 소스
"#version 400 core                                                 \n"
"layout(quads, equal_spacing, ccw) in;                             \n"
"uniform    mat4    gTransform;                                    \n"
"                                                                  \n"
"float B(int i, float u)                                           \n"
"{                                                                 \n"
"    const vec4 bc = vec4(1, 3, 3, 1);                             \n"
"    return bc[i] * pow(u, i) * pow(1.0 - u, 3 - i);               \n"
"}                                                                 \n"
"                                                                  \n"
"void main()                                                       \n"
"{                                                                 \n"
"    vec4  pos = vec4(0.0);                                        \n"
"    vec4  col = vec4(0.0);                                        \n"
"    float  u = gl_TessCoord.x;                                    \n"
"    float  v = gl_TessCoord.y;                                    \n"
"                                                                  \n"
"    for (int k = 0; k < 4; ++k) {                                 \n"
"      for (int j = 0; j < 4; ++j) {                               \n"
"          pos += B(j, u) * B(k, v) * gl_in[4 * k + j].gl_Position;\n"
"      }                                                           \n"
"    }                                                             \n"
"    gl_Position = gTransform * pos;                               \n"
"}";

static const char* pFS =    // 프래그먼트 쉐이더 소스
"#version 400 core                                                 \n"
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
    AddShader(shaderProg, pTCS, GL_TESS_CONTROL_SHADER);
    AddShader(shaderProg, pTES, GL_TESS_EVALUATION_SHADER);
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
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);   // 화면과 깊이버퍼 지움

    TransMat4f  viewMat;
    viewMat.viewingTrans(Vec3f(1.5f, 2.0f, 2.5f), Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f));
    TransMat4f  orthoMat;
    orthoMat.orthgonalPrj(-3.0f, 3.0f, -3.0f, 3.0f, -3.0f, 3.0f);
    TransMat4f  tMat(orthoMat*viewMat);
    glUniformMatrix4fv(uGTransLocation, 1, GL_TRUE, tMat.getPMat());

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[B_SURFACE]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    VecColor4f   col = VecColor4f(0.0f, 1.0f, 1.0f, 1.0f);
    glUniform4fv(uGColorLocation, 1, &col.r);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArrays(GL_PATCHES, 0, 16);
    col = VecColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    glUniform4fv(uGColorLocation, 1, &col.r);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_PATCHES, 0, 16);
    glDisableVertexAttribArray(0);

    glutSwapBuffers();
}

void InitVBOs()
{
    Vec3f Vertices[16] = {     // 곡면의 제어점 좌표
        Vec3f(1.5, -1.5, 0.0), Vec3f(1.5, -0.5, 0.2),
        Vec3f(1.5, 0.5, 0.0), Vec3f(1.5, 1.5, -1.0),

        Vec3f(0.5, -1.5, 0.5), Vec3f(0.5, -0.5, 2.0),
        Vec3f(0.5, 0.5, 2.0), Vec3f(0.5, 1.5, 1.0),

        Vec3f(-0.5, -1.5, 0.5), Vec3f(-0.5, -0.5, 2.0),
        Vec3f(-0.5, 0.5, 2.0), Vec3f(-0.5, 1.5, 1.0),

        Vec3f(-1.5, -1.5, 0.0), Vec3f(-1.5, -0.5, 1.0),
        Vec3f(-1.5, 0.5, 0.5), Vec3f(-1.5, 1.5, -1.0) };

    glGenBuffers(N_VBOs, VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[B_SURFACE]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    glPatchParameteri(GL_PATCH_VERTICES, 16);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(50, 100);
    glutInitWindowSize(400, 400);
    glutCreateWindow("베지어 스플라인 곡면");

    GLenum s = glewInit();
    if (s != GLEW_OK) {
        cerr << "오류 - " << glewGetErrorString(s) << endl;
        return 1;
    }
    cout << "GL version: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    SetUpShaders();
    InitVBOs();

    glutDisplayFunc(RenderCB);
    glutMainLoop();

    return 0;
}
