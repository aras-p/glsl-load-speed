#define _CRT_SECURE_NO_WARNINGS // shut up MSVC

#ifdef _MSC_VER
#define TEST_D3D9 1
#include <d3d9.h>
#include <d3dx9shader.h>

extern "C" {
typedef HRESULT (WINAPI * D3DXAssembleShaderFunc)(
				   LPCSTR                          pSrcData,
				   UINT                            SrcDataLen,
				   CONST D3DXMACRO*                pDefines,
				   LPD3DXINCLUDE                   pInclude,
				   DWORD                           Flags,
				   LPD3DXBUFFER*                   ppShader,
				   LPD3DXBUFFER*                   ppErrorMsgs);
static HINSTANCE s_D3DXDll;
static D3DXAssembleShaderFunc s_D3DXAssemble;
}
#endif

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

	const GLubyte* renderer = glGetString (GL_RENDERER);
	const GLubyte* version = glGetString (GL_VERSION);
	printf ("GL renderer: %s\n", renderer);
	printf ("GL version: %s\n", version);

	glViewport (0,0,1,1);
	
	return hasGLSL;
}



#ifdef TEST_D3D9

static IDirect3D9* s_D3D;
static IDirect3DDevice9* s_D3DDevice;
static IDirect3DQuery9* s_D3DQuery;

static bool InitializeD3D9 ()
{
	s_D3D = Direct3DCreate9(D3D_SDK_VERSION);
	if(!s_D3D)
		return false;
	D3DDISPLAYMODE mode;
	if (FAILED(s_D3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode)))
		return false;

	HWND window = ::GetDesktopWindow();

	D3DPRESENT_PARAMETERS params;
	ZeroMemory(&params, sizeof(params));
	params.BackBufferWidth = 64;
	params.BackBufferHeight = 64;
	params.BackBufferFormat = mode.Format;
	params.BackBufferCount = 1;
	params.hDeviceWindow = window;

	params.AutoDepthStencilFormat = D3DFMT_D16;
	params.EnableAutoDepthStencil = TRUE;

	params.Windowed = TRUE;
	params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	if (FAILED(s_D3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &params, &s_D3DDevice)))
	{
		if (FAILED(s_D3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &params, &s_D3DDevice)))
			return false;
	}

	s_D3DXDll = ::LoadLibraryA ("d3dx9_43.dll");
	if (!s_D3DXDll)
		return false;

	s_D3DXAssemble = (D3DXAssembleShaderFunc)::GetProcAddress (s_D3DXDll, "D3DXAssembleShader");
	if (!s_D3DXAssemble)
		return false;

	D3DVIEWPORT9 vp;
	vp.X = 0;
	vp.Y = 0;
	vp.Width = 1;
	vp.Height = 1;
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;
	s_D3DDevice->SetViewport (&vp);

	s_D3DDevice->CreateQuery (D3DQUERYTYPE_EVENT, &s_D3DQuery);

	s_D3DDevice->BeginScene();

	return true;
}

static ID3DXBuffer* AssembleD3DShader (const std::string& source)
{
	ID3DXBuffer *compiledShader, *compileErrors;
	DWORD flags = 0;
	HRESULT hr = s_D3DXAssemble (source.c_str(), source.size(), NULL, NULL, flags, &compiledShader, &compileErrors);
	if(FAILED(hr))
	{
		if (compileErrors && compileErrors->GetBufferSize() > 0)
		{
			printf ("ERROR: d3d shader assembly failed: %s\n", (const char*)compileErrors->GetBufferPointer());
			compileErrors->Release();
		}
		if(compiledShader)
			compiledShader->Release();
		return NULL;
	}
	return compiledShader;
}

#endif // TEST_D3D9



static void CheckErrors (const char* op)
{
	#if 1
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


#ifdef TEST_D3D9
static bool TestShaderD3D (ID3DXBuffer* vscode, ID3DXBuffer* pscode)
{
	HRESULT hr;
	IDirect3DVertexShader9* vs;
	IDirect3DPixelShader9* ps;
	hr = s_D3DDevice->CreateVertexShader ((const DWORD*)vscode->GetBufferPointer(), &vs);
	if (FAILED(hr))
		return false;
	hr = s_D3DDevice->CreatePixelShader ((const DWORD*)pscode->GetBufferPointer(), &ps);
	if (FAILED(hr))
		return false;

	hr = s_D3DDevice->SetVertexShader (vs);
	if (FAILED(hr))
		return false;
	hr = s_D3DDevice->SetPixelShader (ps);
	if (FAILED(hr))
		return false;

	const float squareVertices[] = {
		-1.0f, -1.0f, 0,
		1.0f,  -1.0f, 0,
		-1.0f,  1.0f, 0,
		1.0f,   1.0f, 0,
	};
	hr = s_D3DDevice->SetFVF (D3DFVF_XYZ);
	hr = s_D3DDevice->DrawPrimitiveUP (D3DPT_TRIANGLESTRIP, 2, squareVertices, 12);
	if (FAILED(hr))
		return false;

	// emulate glFinish
	hr = s_D3DQuery->Issue (D3DISSUE_END);
	if (FAILED(hr))
		return false;
	do 
	{
		hr = s_D3DQuery->GetData (NULL, 0, D3DGETDATA_FLUSH);
	} while (hr == S_FALSE);

	return true;
}
#endif


static bool ReadStringFromFile (const string& pathName, std::string& output)
{
	FILE* file = fopen(pathName.c_str(), "rb");
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



#ifdef _MSC_VER
static LARGE_INTEGER s_Time0;
#else
static timeval s_Time0;
#endif


static void TimerBegin()
{
	#ifdef _MSC_VER
	QueryPerformanceCounter (&s_Time0);
	#else
	gettimeofday(&s_Time0, NULL);
	#endif
}


static float TimerEnd()
{
	#ifdef _MSC_VER

	static bool timerInited = false;
	static LARGE_INTEGER ticksPerSec;
	if (!timerInited) {
		QueryPerformanceFrequency(&ticksPerSec);
		timerInited = true;
	}
	LARGE_INTEGER ttt1;
	QueryPerformanceCounter (&ttt1);
	float timeTaken = float(double(ttt1.QuadPart-s_Time0.QuadPart) / double(ticksPerSec.QuadPart));

	#else

	timeval ttt1;
	gettimeofday( &ttt1, NULL );
	timeval ttt2;
	timersub( &ttt1, &s_Time0, &ttt2 );
	float timeTaken = ttt2.tv_sec + ttt2.tv_usec * 1.0e-6f;

	#endif

	return timeTaken;
}


static void BenchmarkGL (const string& name, const string& vs, const string& fs)
{
	TimerBegin ();

	if (!TestShader (vs, fs))
	{
		printf ("ERROR: error testing shaders\n");
		return;
	}
	
	float timeTaken = TimerEnd();	
	printf ("%s %.2fms\n", name.c_str(), timeTaken*1000.0f);
}


static void BenchmarkD3D9 (const string& name, const string& vs, const string& fs)
{
	#ifdef TEST_D3D9

	ID3DXBuffer* vsCode = AssembleD3DShader (vs);
	ID3DXBuffer* psCode = AssembleD3DShader (fs);

	TimerBegin ();

	if (!TestShaderD3D (vsCode, psCode))
	{
		printf ("ERROR: error testing D3D9 shaders\n");
		return;
	}

	float timeTaken = TimerEnd();	
	printf ("%s %.2fms\n", name.c_str(), timeTaken*1000.0f);

	#endif
}


int main (int argc, char * const argv[])
{
	if (argc < 2)
	{
		printf ("usage: GlslLoadSpeed <filepat>\n");
		return 1;
	}
	
	if (!InitializeOpenGL())
	{
		printf ("ERROR: no OpenGL/GLSL\n");
		return 1;
	}
	#ifdef TEST_D3D9
	bool hasD3D = InitializeD3D9 ();
	#endif

	string basename = argv[1];

	string vs, fs, vsopt, fsopt, d3dvs, d3dps;
	if (!ReadStringFromFile (basename+"-vs.txt", vs))
	{
		printf ("ERROR: can't read VS file %s\n", (basename+"-vs.txt").c_str());
		return 1;
	}
	if (!ReadStringFromFile (basename+"-fs.txt", fs))
	{
		printf ("ERROR: can't read FS file %s\n", (basename+"-fs.txt").c_str());
		return 1;
	}
	if (!ReadStringFromFile (basename+"-opt-vs.txt", vsopt))
	{
		vsopt = vs;
	}
	if (!ReadStringFromFile (basename+"-opt-fs.txt", fsopt))
	{
		printf ("ERROR: can't read FS optimized file %s\n", (basename+"-opt-fs.txt").c_str());
		return 1;
	}
	ReadStringFromFile (basename+"-d3d9-vs.txt", d3dvs);
	ReadStringFromFile (basename+"-d3d9-ps.txt", d3dps);
	
	printf ("testing %s\n", basename.c_str());
	
	// create dummy shaders to prewarm/load the compiler
	string dummyVS = "void main(void) { gl_Position = vec4(0.0); }";
	string dummyFS = "void main(void) { gl_FragColor = vec4(1.0,0.0,0.0,1.0); }";
	if (!TestShader (dummyVS, dummyFS))
	{
		printf ("ERROR: failed to load dummy shaders\n");
	}
	
	BenchmarkGL ("Raw: ", vs, fs);
	BenchmarkGL ("Opt: ", vsopt, fsopt);	

	#ifdef TEST_D3D9
	if (hasD3D && !d3dvs.empty() && !d3dps.empty())
		BenchmarkD3D9("D3D: ", d3dvs, d3dps);
	#endif
	
    return 0;
}
