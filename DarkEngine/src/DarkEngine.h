#define WIN32_LEAN_AND_MEAN
int _fltused;
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <shellapi.h>
#include <shlobj_core.h>
#include <shobjidl_core.h>
#include <shlguid.h>
#include <commdlg.h>
#include <wincodec.h>
#include <shlwapi.h>
#pragma comment (lib, "kernel32.lib")
#pragma comment (lib, "shell32.lib")
#pragma comment (lib, "gdi32.lib")
#pragma comment (lib, "user32.lib")
#pragma comment (lib, "ole32.lib")
#pragma comment (lib, "uuid.lib")
#pragma comment (lib, "dwmapi.lib")
#pragma comment (lib, "comdlg32.lib")
#pragma comment (lib, "uxtheme.lib")
#pragma comment (lib, "advapi32.lib")
#pragma comment (lib, "comctl32.lib")
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "windowscodecs.lib")
#pragma comment (lib, "shlwapi.lib")
#pragma comment (lib, "legacy_stdio_definitions.lib")

#include <GL/gl.h>
#include "mgl.h"
#include "stb_image.h"
#include "cgltf.h"
#include "ttf2mesh.h"
#include "portaudio.h"
#include "minimp3_ex.h"
#include "perlin_noise.h"

#undef near
#undef far
#define CLAMP(a,min,max) ((a) < (min) ? (min) : (a) > (max) ? (max) : (a))
#define COUNT(arr) (sizeof(arr)/sizeof(*arr))
#define SWAP(temp,a,b) (temp)=(a); (a)=(b); (b)=temp
#define RGBA(r,g,b,a) (r | (g<<8) | (b<<16) | (a<<24))
#define RED(c) (c & 0xff)
#define GREEN(c) ((c>>8) & 0xff)
#define BLUE(c) ((c>>16) & 0xff)
#define ALPHA(c) ((c>>24) & 0xff)
#define REDF(c) (RED(c)/255.0f)
#define BLUEF(c) (BLUE(c)/255.0f)
#define GREENF(c) (GREEN(c)/255.0f)
#define ALPHAF(c) (ALPHA(c)/255.0f)

/* API: */
extern PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
extern PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
extern PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
#define GL_FUNCTIONS(X) \
	X(PFNGLCREATESHADERPROC,             glCreateShader             ) \
	X(PFNGLSHADERSOURCEPROC,             glShaderSource             ) \
	X(PFNGLGETSHADERIVPROC,              glGetShaderiv              ) \
	X(PFNGLGETSHADERINFOLOGPROC,         glGetShaderInfoLog         ) \
	X(PFNGLGETPROGRAMIVPROC,             glGetProgramiv             ) \
	X(PFNGLGETPROGRAMINFOLOGPROC,        glGetProgramInfoLog        ) \
	X(PFNGLCOMPILESHADERPROC,            glCompileShader            ) \
	X(PFNGLCREATEPROGRAMPROC,            glCreateProgram            ) \
	X(PFNGLATTACHSHADERPROC,             glAttachShader             ) \
	X(PFNGLLINKPROGRAMPROC,              glLinkProgram              ) \
	X(PFNGLDELETESHADERPROC,             glDeleteShader             ) \
	X(PFNGLUSEPROGRAMPROC,				 glUseProgram				) \
	X(PFNGLGETATTRIBLOCATIONPROC,        glGetAttribLocation        ) \
	X(PFNGLGETUNIFORMLOCATIONPROC,       glGetUniformLocation       ) \
	X(PFNGLGENVERTEXARRAYSPROC,          glGenVertexArrays          ) \
	X(PFNGLBINDVERTEXARRAYPROC,          glBindVertexArray          ) \
	X(PFNGLGENBUFFERSPROC,               glGenBuffers               ) \
	X(PFNGLBINDBUFFERPROC,               glBindBuffer               ) \
	X(PFNGLBUFFERDATAPROC,               glBufferData               ) \
	X(PFNGLDELETEBUFFERSPROC,            glDeleteBuffers            ) \
	X(PFNGLENABLEVERTEXATTRIBARRAYPROC,  glEnableVertexAttribArray  ) \
	X(PFNGLVERTEXATTRIBPOINTERPROC,      glVertexAttribPointer      ) \
	X(PFNGLUNIFORM1FPROC,                glUniform1f                ) \
	X(PFNGLUNIFORM2FPROC,                glUniform2f                ) \
	X(PFNGLUNIFORM3FPROC,                glUniform3f                ) \
	X(PFNGLUNIFORM4FPROC,                glUniform4f                ) \
	X(PFNGLUNIFORM1FVPROC,               glUniform1fv               ) \
	X(PFNGLUNIFORM2FVPROC,               glUniform2fv               ) \
	X(PFNGLUNIFORM3FVPROC,               glUniform3fv               ) \
	X(PFNGLUNIFORM4FVPROC,               glUniform4fv               ) \
	X(PFNGLUNIFORM1IPROC,                glUniform1i                ) \
	X(PFNGLUNIFORM2IPROC,                glUniform2i                ) \
	X(PFNGLUNIFORM3IPROC,                glUniform3i                ) \
	X(PFNGLUNIFORM4IPROC,                glUniform4i                ) \
	X(PFNGLUNIFORM1IVPROC,               glUniform1iv               ) \
	X(PFNGLUNIFORM2IVPROC,               glUniform2iv               ) \
	X(PFNGLUNIFORM3IVPROC,               glUniform3iv               ) \
	X(PFNGLUNIFORM4IVPROC,               glUniform4iv               ) \
	X(PFNGLUNIFORMMATRIX4FVPROC,         glUniformMatrix4fv         ) \
	X(PFNGLDEBUGMESSAGECALLBACKPROC,     glDebugMessageCallback     )
#define X(type, name) extern type name;
GL_FUNCTIONS(X)
#undef X
extern void FatalErrorA(char *format, ...);
extern void FatalErrorW(WCHAR *format, ...);
extern void *MallocOrDie(size_t size);
extern void *ZallocOrDie(size_t size);
extern void *ReallocOrDie(void *ptr, size_t size);
extern int randint(int n);
extern char *LoadFileA(char *path, size_t *size);
extern char *LoadFileW(WCHAR *path, size_t *size);
extern int CharIsSuitableForFileName(char c);
typedef struct {
	char *bytes;
	size_t size;
} ByteArray;
extern ByteArray ByteArrayFromFileA(char *path);
extern ByteArray ByteArrayFromFileW(WCHAR *path);
extern BOOL FileOrFolderExists(LPCTSTR path);
typedef struct {
	int elementSize;
	int total;
	int used;
	int padding;//malloc returns 16 byte aligned pointers on x86-64, so making ListHeader 16 bytes means the elements will be 16 byte aligned. As for systems with a non-32 bit int, fuck them. kek.
} ListHeader;
extern void ListInit(ListHeader **listp, int elementSize, int total);
extern int ListIndexOf(ListHeader *list, void *element);
extern void *ListGet(ListHeader *list, int index);
extern void ListSet(ListHeader *list, int index, void *elements, int count);
extern void ListInsert(ListHeader **listp, int index, void *elements, int count);
extern void ListRemove(ListHeader **listp, int index, int count);
extern void ListAppend(ListHeader **listp, void *elements, int count);
extern int Find(int *indices, int index);
extern void Union(int *indices, int aIndex, int bIndex);
extern uint32_t fnv1a(uint8_t *key, int len);
typedef struct {
	int keylen;
	char key[];
} BucketHeader;
#define HASHLIST_STARTING_TOTAL 8
#define HASHLIST_MAX_LOAD 0.75f
#define HASHLIST_TOMBSTONE -1
extern void HashListInit(ListHeader **listp, size_t elementSize);
extern BucketHeader *HashListGet(ListHeader *list, char *key, int keylen);
extern void HashListSet(ListHeader **listp, BucketHeader *bucket);
extern int HashListRemove(ListHeader **listp, char *key, int keylen);
typedef struct {
	size_t elementSize;
	int total;
	int used;
	void *elements;
	int writeIndex;
	int readIndex;
} RingBuffer;
extern void RingBufferReset(RingBuffer *r);
extern void RingBufferInit(RingBuffer *r, size_t elementSize, int total, void *elements);
extern void *RingBufferGet(RingBuffer *r, int index);
extern void *RingBufferIncrementWriteHead(RingBuffer *r);
extern void RingBufferIncrementReadHead(RingBuffer *r);
typedef union IVec3{
	struct {int x,y,z;};
	int arr[3];
}IVec3;
typedef union FVec2{
	struct {float x,y;};
	float arr[2];
}FVec2;
typedef union FVec3{
	struct {float x,y,z;};
	float arr[3];
}FVec3;
typedef union FVec4{
	struct {FVec3 vec3; float ww;};
	struct {float x,y,z,w;};
	float arr[4];
}FVec4;
typedef union Mat4{
	struct {FVec4 a,b,c,d;};
	struct {float 
		a0,a1,a2,a3,
		b0,b1,b2,b3,
		c0,c1,c2,c3,
		d0,d1,d2,d3;};
	float arr[16];
}Mat4;
extern float Lerp(float a, float b, float t);
extern float fvec3Dot(FVec3 a, FVec3 b);
extern float fvec3Length(FVec3 v);
extern FVec3 fvec3Cross(FVec3 a, FVec3 b);
extern FVec3 fvec3Add(FVec3 a, FVec3 b);
extern FVec3 fvec3Sub(FVec3 a, FVec3 b);
extern FVec3 fvec3Negate(FVec3 a);
extern FVec3 fvec3Scale(FVec3 v, float s);
extern FVec3 fvec3Norm(FVec3 v);
extern FVec3 fvec3SetLength(FVec3 v, float length);
extern FVec3 fvec3Midpoint(FVec3 a, FVec3 b);
extern FVec3 fvec3Lerp(FVec3 a, FVec3 b, float t);
extern FVec4 fvec4Lerp(FVec4 a, FVec4 b, float t);
extern float fvec3Dist(FVec3 a, FVec3 b);
extern float fvec3AngleBetween(FVec3 a, FVec3 b);
extern FVec3 clampEuler(FVec3 e);
extern float fvec4Dot(FVec4 a, FVec4 b);
extern FVec4 quatSlerp(FVec4 a, FVec4 b, float t);
extern Mat4 mat4Identity(void);
extern Mat4 mat4Basis(FVec3 x, FVec3 y, FVec3 z);
extern Mat4 mat4Transpose(Mat4 m);
extern Mat4 mat4TransposeMat3(Mat4 m);
extern Mat4 mat4Ortho(float left, float right, float bottom, float top, float near, float far);
extern Mat4 mat4Persp(float fovRadians, float aspectRatio, float near, float far);
extern Mat4 mat4Scale(FVec3 v);
extern Mat4 mat4Pos(FVec3 v);
extern Mat4 mat4RotX(float angle);
extern Mat4 mat4RotY(float angle);
extern Mat4 mat4RotZ(float angle);
extern Mat4 mat4Mul(Mat4 a, Mat4 b);
extern FVec4 mat4MulFVec4(Mat4 m, FVec4 v);
extern Mat4 mat4LookAt(FVec3 eye, FVec3 target);
extern Mat4 mat4LookAtShake(FVec3 eye, FVec3 target, FVec3 shakeEuler);
extern Mat4 eulerToMat4(FVec3 e);
extern Mat4 quatToMat4(FVec4 q);
extern Mat4 mat4MtwInverse(Mat4 m);
extern void mat4Print(Mat4 m);
extern FVec3 fvec3Rotated(FVec3 v, FVec3 euler);
extern FVec3 normalFromTriangle(FVec3 a, FVec3 b, FVec3 c);
extern int manhattanDistance(IVec3 a, IVec3 b);
typedef struct Camera{
	FVec3 position;
	FVec3 euler;
	float aspect, fov;
}Camera;
extern Mat4 getVP(Camera *c);
extern void rotateCamera(Camera *c, float dx, float dy, float sens);
typedef struct {
	FVec3 halfExtents;
	FVec3 euler;
	FVec3 position;
} OBB;
extern bool OBBsColliding(OBB *a, OBB *b);
extern GLenum glCheckError_(const char *file, int line);
#define glCheckError() glCheckError_(__FILE__, __LINE__)
typedef struct {
	unsigned char *pixels;
	int width,height,comp;
} Image;
typedef struct {
	GLuint id;
	int width, height;
} Texture;
extern void ImageFromFileA(Image *i, char *path);
extern void ImageFromFileW(Image *i, WCHAR *path);
extern void TextureFromImage(Texture *t, Image *i, bool interpolated);
extern void TextureFromFileA(Texture *t, char *path, bool interpolated);
extern void TextureFromFileW(Texture *t, WCHAR *path, bool interpolated);
extern void CheckShader(GLuint id);
extern void CheckProgram(GLuint id, GLenum param);
extern GLuint CompileShader(char *name, char *vertSrc, char *fragSrc);
extern void DrawTriangles(ListHeader* verts, void (*prepBuffer)(void), bool wireframe);
extern struct ColorShader {
	char *vertSrc;
	char *fragSrc;
	GLuint id;
	GLint aPosition;
	GLint aColor;
	GLint proj;
} ColorShader;
typedef struct {
	FVec3 Position;
	uint32_t color;
} ColorVertex;
extern void ColorShaderPrepBuffer();
typedef struct {
	int index;
	int vertexCount;
	float width;
	float advance;
} GlyphMesh;
typedef struct {
	ttf_t *ttf;
	ListHeader *colorVerts;
	GlyphMesh glyphMeshes[65536];
	float height;
} CachedFont;
extern void GenFontMeshes(CachedFont *font);
extern void LoadFontA(CachedFont *font, char *path);
extern void LoadFontW(CachedFont *font, WCHAR *path);
extern float StringWidth(CachedFont *font, float size, WCHAR *str, int numChars);
extern void AppendStringMesh(ListHeader **colorVerts, CachedFont *font, int x, int y, int z, float size, uint32_t color, WCHAR *str, int numChars);
extern void AppendFormatStringMesh(ListHeader **colorVerts, CachedFont *font, int x, int y, int z, float size, uint32_t color, WCHAR *format, ...);
extern void AppendFormatCenteredStringMesh(ListHeader **colorVerts, CachedFont *font, int x, int y, int z, float size, uint32_t color, WCHAR *format, ...);
typedef struct {
	FVec3 Position;
	FVec3 Normal;
} NormalVertex;
extern struct UniformColorGouraudDirlightShader {
	char *vertSrc;
	char *fragSrc;
	GLuint id;
	GLint aPosition;
	GLint aNormal;
	GLint lightDir;
	GLint proj;
	GLint mtw;
	GLint color;
} UniformColorGouraudDirlightShader;
extern void UniformColorGouraudDirlightShaderPrepBuffer();
extern struct ColorGouraudDirlightShader {
	char *vertSrc;
	char *fragSrc;
	GLuint id;
	GLint aPosition;
	GLint aNormal;
	GLint aColor;
	GLint lightDir;
	GLint proj;
	GLint mtw;
} ColorGouraudDirlightShader;
typedef struct {
	FVec3 Position;
	FVec3 Normal;
	uint32_t color;
} ColorNormalVertex;
extern void ColorGouraudDirlightShaderPrepBuffer();
extern FVec3 cubeVerts[];
extern FVec3 cubeNormals[];
extern void AppendColoredCubeFace(ListHeader **colorNormalVerts, int faceIndex, FVec3 translation, uint32_t color);
extern void AppendCenteredCube(ListHeader **colorNormalVerts, uint32_t color);
extern void AppendCylinder(ListHeader **normalVerts, float innerRadius, float outerRadius, float height, int nVertices);
extern void AppendColoredCylinder(ListHeader **colorNormalVerts, float innerRadius, float outerRadius, float height, int nVertices, uint32_t color);
extern struct RoundedRectangleShader {
	char *vertSrc;
	char *fragSrc;
	GLuint id;
	GLint aPosition;
	GLint aRectangle;
	GLint aRoundingRadius;
	GLint aColor;
	GLint aIconColor;
	GLint proj;
} RoundedRectangleShader;
typedef struct {
	FVec3 Position;
	FVec4 Rectangle;
	float RoundingRadius;
	uint32_t color;
	uint32_t IconColor;
} RoundedRectangleVertex;
enum RoundedRectangleType {
	RR_CHANNEL=51,
	RR_FLAT=102,
	RR_DISH=153,
	RR_CONE=204
};
enum RoundedRectangleIconType {
	RR_ICON_NONE=36,
	RR_ICON_PLAY=72,
	RR_ICON_REVERSE_PLAY=108,
	RR_ICON_PAUSE=144,
	RR_ICON_NEXT=180,
	RR_ICON_PREVIOUS=216,
};
extern void RoundedRectangleShaderPrepBuffer();
extern void AppendRoundedRectangle(ListHeader **verts, int x, int y, int z, int halfWidth, int halfHeight, float RoundingRadius, uint32_t color, uint32_t IconColor);
typedef struct {
	FVec3 Position;
	FVec2 Texcoord;
} TextureVertex;
extern struct TextureShader {
	char *vertSrc;
	char *fragSrc;
	GLuint id;
	GLint aPosition;
	GLint aTexcoord;
	GLint uProj;
	GLint uTex;
} TextureShader;
extern void TextureShaderPrepBuffer();
extern HWND DarkGLMakeWindow(int iconId, WCHAR *title, int clientWidth, int clientHeight, WNDPROC windowProc);