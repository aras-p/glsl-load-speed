#include "opengl.h"
#include <stdio.h>


#ifdef _MSC_VER
PFNGLDELETEOBJECTARBPROC glDeleteObjectARB;
PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB;
PFNGLSHADERSOURCEARBPROC glShaderSourceARB;
PFNGLCOMPILESHADERARBPROC glCompileShaderARB;
PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB;
PFNGLATTACHOBJECTARBPROC glAttachObjectARB;
PFNGLLINKPROGRAMARBPROC glLinkProgramARB;
PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB;
#endif


bool InitializeOpenGL ()
{
	bool hasGLSL = false;
	
#ifdef _MSC_VER
	// setup minimal required GL
	HWND wnd = CreateWindowA(
							 "STATIC",
							 "GL",
							 WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
							 0, 0, 16, 16,
							 NULL, NULL,
							 GetModuleHandle(NULL), NULL);
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

	const GLubyte* renderer = glGetString (GL_RENDERER);
	const GLubyte* version = glGetString (GL_VERSION);
	printf ("GL renderer: %s\n", renderer);
	printf ("GL version: %s\n", version);

	glViewport (0,0,1,1);
	
	return hasGLSL;
}




bool CheckStatusGL (const char* op)
{
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		printf ("ERROR: GL error doing %s: 0x%x\n", op, err);
		return false;
	}
	return true;
}


GLhandleARB CreateShaderGL (GLenum target, const char* src)
{
	GLhandleARB shader = glCreateShaderObjectARB (target);
	glShaderSourceARB (shader, 1, &src, NULL);
	glCompileShaderARB (shader);
	
	GLint status;
	glGetObjectParameterivARB (shader, GL_OBJECT_COMPILE_STATUS_ARB, &status);
	if (status == 0)
	{
		char log[4096];
		GLsizei logLength;
		glGetInfoLogARB (shader, sizeof(log), &logLength, log);
		printf ("ERROR: GLSL compile error:\n%s\n", log);
		glDeleteObjectARB (shader);
		return 0;
	}
	
	return shader;
}
