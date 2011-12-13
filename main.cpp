#define _CRT_SECURE_NO_WARNINGS // shut up MSVC
#include <cstdio>
#include <string>
#include <time.h>

#ifdef __APPLE__
#include <sys/time.h>
#endif

using std::string;


#ifdef _MSC_VER
#include <windows.h>
#include <gl/GL.h>
extern "C" {
	typedef char GLcharARB;		/* native character */
	typedef unsigned int GLhandleARB;	/* shader object handle */
#define GL_VERTEX_SHADER_ARB              0x8B31
#define GL_FRAGMENT_SHADER_ARB            0x8B30
#define GL_OBJECT_COMPILE_STATUS_ARB      0x8B81
#define GL_OBJECT_LINK_STATUS_ARB         0x8B82
	typedef void (WINAPI * PFNGLDELETEOBJECTARBPROC) (GLhandleARB obj);
	typedef GLhandleARB (WINAPI * PFNGLCREATESHADEROBJECTARBPROC) (GLenum shaderType);
	typedef void (WINAPI * PFNGLSHADERSOURCEARBPROC) (GLhandleARB shaderObj, GLsizei count, const GLcharARB* *string, const GLint *length);
	typedef void (WINAPI * PFNGLCOMPILESHADERARBPROC) (GLhandleARB shaderObj);
	typedef void (WINAPI * PFNGLGETINFOLOGARBPROC) (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
	typedef void (WINAPI * PFNGLGETOBJECTPARAMETERIVARBPROC) (GLhandleARB obj, GLenum pname, GLint *params);
	typedef GLhandleARB (WINAPI * PFNGLCREATEPROGRAMOBJECTARBPROC) (void);
	typedef void (WINAPI * PFNGLATTACHOBJECTARBPROC) (GLhandleARB containerObj, GLhandleARB obj);
	typedef void (WINAPI * PFNGLLINKPROGRAMARBPROC) (GLhandleARB programObj);
	typedef void (WINAPI * PFNGLUSEPROGRAMOBJECTARBPROC) (GLhandleARB programObj);
	static PFNGLDELETEOBJECTARBPROC glDeleteObjectARB;
	static PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB;
	static PFNGLSHADERSOURCEARBPROC glShaderSourceARB;
	static PFNGLCOMPILESHADERARBPROC glCompileShaderARB;
	static PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
	static PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
	static PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB;
	static PFNGLATTACHOBJECTARBPROC glAttachObjectARB;
	static PFNGLLINKPROGRAMARBPROC glLinkProgramARB;
	static PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB;
}
#else
#include <OpenGL/OpenGL.h>
#include <AGL/agl.h>
#include <dirent.h>
#endif


static bool InitializeOpenGL ()
{
	bool hasGLSL = false;
	
#ifdef _MSC_VER
	// setup minimal required GL
	HWND wnd = CreateWindowA(
							 "STATIC",
							 "GL",
							 WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS |	WS_CLIPCHILDREN,
							 0, 0, 16, 16,
							 NULL, NULL,
							 GetModuleHandle(NULL), NULL );
	HDC dc = GetDC( wnd );
	
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR), 1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL,
		PFD_TYPE_RGBA, 32,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		16, 0,
		0, PFD_MAIN_PLANE, 0, 0, 0, 0
	};
	
	int fmt = ChoosePixelFormat( dc, &pfd );
	SetPixelFormat( dc, fmt, &pfd );
	
	HGLRC rc = wglCreateContext( dc );
	wglMakeCurrent( dc, rc );
	
#else
	GLint attributes[16];
	int i = 0;
	attributes[i++]=AGL_RGBA;
	attributes[i++]=AGL_PIXEL_SIZE;
	attributes[i++]=32;
	attributes[i++]=AGL_NO_RECOVERY;
	attributes[i++]=AGL_NONE;
	
	AGLPixelFormat pixelFormat = aglChoosePixelFormat(NULL,0,attributes);
	AGLContext agl = aglCreateContext(pixelFormat, NULL);
	aglSetCurrentContext (agl);
	
#endif
	
	// check if we have GLSL
	const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
	hasGLSL = strstr(extensions, "GL_ARB_shader_objects") && strstr(extensions, "GL_ARB_vertex_shader") && strstr(extensions, "GL_ARB_fragment_shader");
	
#ifdef _MSC_VER
	if (hasGLSL)
	{
		glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)wglGetProcAddress("glDeleteObjectARB");
		glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)wglGetProcAddress("glCreateShaderObjectARB");
		glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)wglGetProcAddress("glShaderSourceARB");
		glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)wglGetProcAddress("glCompileShaderARB");
		glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)wglGetProcAddress("glGetInfoLogARB");
		glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)wglGetProcAddress("glGetObjectParameterivARB");
		glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)wglGetProcAddress("glCreateProgramObjectARB");
		glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)wglGetProcAddress("glAttachObjectARB");
		glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)wglGetProcAddress("glLinkProgramARB");
		glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)wglGetProcAddress("glUseProgramObjectARB");
	}
#endif

	glViewport (0,0,1,1);
	
	return hasGLSL;
}

static void CheckErrors (const char* op)
{
	#if 0
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		printf ("ERROR: GL error doing %s: %i (%x)\n", op, err, err);
	}
	#endif
}


static GLhandleARB CreateShader (GLenum target, const string& s)
{
	const char* ptr = s.c_str();
	GLhandleARB shader = glCreateShaderObjectARB (target);
	glShaderSourceARB (shader, 1, &ptr, NULL);
	glCompileShaderARB (shader);
	
	GLint status;
	glGetObjectParameterivARB (shader, GL_OBJECT_COMPILE_STATUS_ARB, &status);
	if (status == 0)
	{
		char log[4096];
		GLsizei logLength;
		glGetInfoLogARB (shader, sizeof(log), &logLength, log);
		printf ("  glsl compile error:\n%s\n", log);
		glDeleteObjectARB (shader);
		return 0;
	}
	
	return shader;
}


static bool TestShader (const string& vs, const string& fs)
{
	// compile
	GLhandleARB shaderVS = CreateShader (GL_VERTEX_SHADER_ARB, vs);
	GLhandleARB shaderFS = CreateShader (GL_FRAGMENT_SHADER_ARB, fs);
	if (!shaderVS || !shaderFS)
		return false;
	
	// link
	GLhandleARB program = glCreateProgramObjectARB();
	glAttachObjectARB (program, shaderVS);
	glAttachObjectARB (program, shaderFS);
	glLinkProgramARB (program);
	
	GLint status;
	glGetObjectParameterivARB (program, GL_OBJECT_LINK_STATUS_ARB, &status);
	if (status == 0)
	{
		char log[4096];
		GLsizei logLength;
		glGetInfoLogARB (program, sizeof(log), &logLength, log);
		printf ("  glsl link error:\n%s\n", log);
		glDeleteObjectARB (shaderVS);
		glDeleteObjectARB (shaderFS);
		glDeleteObjectARB (program);
		return false;
	}
	
	// use it and render dummy thing
	glUseProgramObjectARB (program);
	CheckErrors ("use program");
	
    const GLfloat squareVertices[] = {
        -1.0f, -1.0f,
        1.0f,  -1.0f,
        -1.0f,  1.0f,
        1.0f,   1.0f,
    };
	glVertexPointer (2, GL_FLOAT, 0, squareVertices);
	glEnableClientState (GL_VERTEX_ARRAY);
	CheckErrors ("set geom");
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	CheckErrors ("draw");
	
	// cleanup
	glDeleteObjectARB (shaderVS);
	glDeleteObjectARB (shaderFS);
	glDeleteObjectARB (program);
	CheckErrors ("delete");
	
	glFinish ();
	CheckErrors ("finish");
	
	return true;
}


static bool ReadStringFromFile (const char* pathName, std::string& output)
{
	FILE* file = fopen( pathName, "rb" );
	if (file == NULL)
		return false;
	fseek(file, 0, SEEK_END);
	int length = ftell(file);
	fseek(file, 0, SEEK_SET);
	if (length < 0)
	{
		fclose( file );
		return false;
	}
	output.resize(length);
	int readLength = fread(&*output.begin(), 1, length, file);
	fclose(file);
	if (readLength != length)
	{
		output.clear();
		return false;
	}
	return true;
}


static void BenchmarkOnce (const string& vs, const string& fs)
{
	#ifdef _MSC_VER
	DWORD ttt0 = GetTickCount();
	#else
	timeval ttt0;
	gettimeofday( &ttt0, NULL );
	#endif
	
	if (!TestShader (vs, fs))
	{
		printf ("ERROR: error testing shaders\n");
		return;
	}
	
	
	#ifdef _MSC_VER
	DWORD ttt1 = GetTickCount();
	float timeTaken = (ttt1-ttt0) * 0.001f;
	#else
	timeval ttt1;
	gettimeofday( &ttt1, NULL );
	timeval ttt2;
	timersub( &ttt1, &ttt0, &ttt2 );
	float timeTaken = ttt2.tv_sec + ttt2.tv_usec * 1.0e-6f;
	#endif
	
	printf ("took %.3fms\n", timeTaken*1000.0f);
}


int main (int argc, char * const argv[])
{
	if (argc < 3)
	{
		printf ("usage: GlslLoadSpeed <vsfile> <fsfile>\n");
		return 1;
	}
	
	if (!InitializeOpenGL())
	{
		printf ("ERROR: no OpenGL/GLSL\n");
		return 1;
	}
	
	string vs, fs;
	if (!ReadStringFromFile (argv[1], vs))
	{
		printf ("ERROR: can't read VS file %s\n", argv[1]);
		return 1;
	}
	if (!ReadStringFromFile (argv[2], fs))
	{
		printf ("ERROR: can't read FS file %s\n", argv[2]);
		return 1;
	}
	
	printf ("testing %s,%s\n", argv[1], argv[2]);
	
	// create dummy shaders to prewarm/load the compiler
	string dummyVS = "void main(void) { gl_Position = vec4(0.0); }";
	string dummyFS = "void main(void) { gl_FragColor = vec4(1.0,0.0,0.0,1.0); }";
	if (!TestShader (dummyVS, dummyFS))
	{
		printf ("ERROR: failed to load dummy shaders\n");
	}
	
	BenchmarkOnce (vs, fs);
	BenchmarkOnce (vs, fs);
	BenchmarkOnce (vs, fs);
	
	
    return 0;
}
