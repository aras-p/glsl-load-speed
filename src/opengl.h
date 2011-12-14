#pragma once

#ifdef _MSC_VER
#include <windows.h>
#include <gl/GL.h>

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
extern PFNGLDELETEOBJECTARBPROC glDeleteObjectARB;
extern PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB;
extern PFNGLSHADERSOURCEARBPROC glShaderSourceARB;
extern PFNGLCOMPILESHADERARBPROC glCompileShaderARB;
extern PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
extern PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
extern PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB;
extern PFNGLATTACHOBJECTARBPROC glAttachObjectARB;
extern PFNGLLINKPROGRAMARBPROC glLinkProgramARB;
extern PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB;

#else

#include <OpenGL/OpenGL.h>
#include <AGL/agl.h>

#endif


bool InitializeOpenGL ();
bool CheckStatusGL (const char* op);

GLhandleARB CreateShaderGL (GLenum target, const char* src);
