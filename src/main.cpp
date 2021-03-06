#include "opengl.h"
#include "direct3d9.h"

#include <stdio.h>
#include <string>
#include <time.h>
#include <algorithm>

#ifdef __APPLE__
#include <sys/time.h>
#endif

using std::string;



static bool TestShader (const string& vs, const string& fs)
{
	// compile
	GLhandleARB shaderVS = CreateShaderGL (GL_VERTEX_SHADER_ARB, vs.c_str());
	GLhandleARB shaderFS = CreateShaderGL (GL_FRAGMENT_SHADER_ARB, fs.c_str());
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
	if (!CheckStatusGL("use program"))
		return false;
	
    const GLfloat squareVertices[] = {
        -1.0f, -1.0f,
        1.0f,  -1.0f,
        -1.0f,  1.0f,
        1.0f,   1.0f,
    };
	glVertexPointer (2, GL_FLOAT, 0, squareVertices);
	glEnableClientState (GL_VERTEX_ARRAY);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	if (!CheckStatusGL("draw"))
		return false;
	
	// cleanup
	glDeleteObjectARB (shaderVS);
	glDeleteObjectARB (shaderFS);
	glDeleteObjectARB (program);
	
	glFinish ();
	if (!CheckStatusGL("finish"))
		return false;
	
	return true;
}


#ifdef D3D9_AVAILABLE
static bool TestShaderD3D (const MOJOSHADER_parseData* vscode, const MOJOSHADER_parseData* pscode)
{
	HRESULT hr;
	IDirect3DVertexShader9* vs;
	IDirect3DPixelShader9* ps;
	hr = g_D3D9Device->CreateVertexShader ((const DWORD*)vscode->output, &vs);
	if (FAILED(hr))
		return false;
	hr = g_D3D9Device->CreatePixelShader ((const DWORD*)pscode->output, &ps);
	if (FAILED(hr))
		return false;

	hr = g_D3D9Device->SetVertexShader (vs);
	if (FAILED(hr))
		return false;
	hr = g_D3D9Device->SetPixelShader (ps);
	if (FAILED(hr))
		return false;

	const float squareVertices[] = {
		-1.0f, -1.0f, 0,
		1.0f,  -1.0f, 0,
		-1.0f,  1.0f, 0,
		1.0f,   1.0f, 0,
	};
	hr = g_D3D9Device->SetFVF (D3DFVF_XYZ);
	hr = g_D3D9Device->DrawPrimitiveUP (D3DPT_TRIANGLESTRIP, 2, squareVertices, 12);
	if (FAILED(hr))
		return false;

	g_D3D9Device->SetVertexShader (NULL);
	g_D3D9Device->SetPixelShader (NULL);
	vs->Release();
	ps->Release();

	// emulate glFinish
	hr = g_D3D9Query->Issue (D3DISSUE_END);
	if (FAILED(hr))
		return false;
	do 
	{
		hr = g_D3D9Query->GetData (NULL, 0, D3DGETDATA_FLUSH);
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

const int kRunTimes = 10;


static float BenchmarkGL (const string& vs, const string& fs)
{
	float times[kRunTimes];
	for (int i = 0; i < kRunTimes; ++i)
	{
		TimerBegin ();
		if (!TestShader (vs, fs))
		{
			printf ("ERROR: error testing shaders\n");
			return 0.0f;
		}
		float timeTaken = TimerEnd();
		times[i] = timeTaken;
	}
	std::sort (times, times+kRunTimes);
	return times[kRunTimes/2] * 1000.0f;
}


#ifdef D3D9_AVAILABLE
static float BenchmarkD3D9 (const string& vs, const string& fs)
{
	float times[kRunTimes];

	for (int i = 0; i < kRunTimes; ++i)
	{
		const MOJOSHADER_parseData* vsCode = AssembleShaderD3D9 (vs.c_str(), vs.size());
		const MOJOSHADER_parseData* psCode = AssembleShaderD3D9 (fs.c_str(), fs.size());

		TimerBegin ();

		if (!TestShaderD3D (vsCode, psCode))
		{
			printf ("ERROR: error testing D3D9 shaders\n");
			return 0.0f;
		}

		ReleaseAssembledShaderD3D9 (vsCode);
		ReleaseAssembledShaderD3D9 (psCode);

		float timeTaken = TimerEnd();	
		times[i] = timeTaken;
	}
	std::sort (times, times+kRunTimes);
	return times[kRunTimes/2] * 1000.0f;
}
#endif


int main (int argc, char * const argv[])
{
	if (!InitializeOpenGL())
	{
		printf ("ERROR: no OpenGL/GLSL\n");
		return 1;
	}
	#ifdef D3D9_AVAILABLE
	bool hasD3D = InitializeD3D9 (::GetDesktopWindow());
	#endif

	// create dummy shaders to prewarm/load the compiler
	string dummyVS = "void main(void) { gl_Position = vec4(0.0); }";
	string dummyFS = "void main(void) { gl_FragColor = vec4(1.0,0.0,0.0,1.0); }";
	if (!TestShader (dummyVS, dummyFS))
	{
		printf ("ERROR: failed to load dummy shaders\n");
		return 1;
	}

	const char* shaders[] = {
		"shaders/treeleaf",
		"shaders/fxaa311pc39",
		"shaders/prepasslight",
		"shaders/prlxspec",
		"shaders/miatest",
		"shaders/ssao",
	};
	for (int i = 0; i < sizeof(shaders)/sizeof(shaders[0]); ++i)
	{
		string basename = shaders[i];

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
		
		printf ("\n**** Shader %s:\n", basename.c_str());
				
		float timeGL = BenchmarkGL (vs, fs);
		float timeGLOpt = BenchmarkGL (vsopt, fsopt);

		#ifdef D3D9_AVAILABLE
		float timeD3D = 0.0f;
		if (hasD3D && !d3dvs.empty() && !d3dps.empty())
			timeD3D = BenchmarkD3D9(d3dvs, d3dps);
		#endif

		printf ("GL:           %.2fms\n", timeGL);
		printf ("GL Optimized: %.2fms (%.2fms less, or %.2f times faster)\n", timeGLOpt, timeGL-timeGLOpt, timeGL/timeGLOpt);
		#ifdef D3D9_AVAILABLE
		if (timeD3D != 0.0f)
		{
			printf ("D3D9:         %.2fms (%.2fms less, or %.2f times faster)\n", timeD3D, timeGL-timeD3D, timeGL/timeD3D);
		}
		#endif
	}
	
    return 0;
}
