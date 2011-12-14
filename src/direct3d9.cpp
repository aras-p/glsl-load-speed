#include "direct3d9.h"
#include <stdio.h>

static IDirect3D9* s_D3D;
IDirect3DQuery9* g_D3D9Query;
IDirect3DDevice9* g_D3D9Device;


bool InitializeD3D9 (HWND window)
{
	s_D3D = Direct3DCreate9(D3D_SDK_VERSION);
	if(!s_D3D)
		return false;
	D3DDISPLAYMODE mode;
	if (FAILED(s_D3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode)))
		return false;

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

	if (FAILED(s_D3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &params, &g_D3D9Device)))
	{
		if (FAILED(s_D3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &params, &g_D3D9Device)))
			return false;
	}

	D3DVIEWPORT9 vp;
	vp.X = 0;
	vp.Y = 0;
	vp.Width = 1;
	vp.Height = 1;
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;
	g_D3D9Device->SetViewport (&vp);

	g_D3D9Device->CreateQuery (D3DQUERYTYPE_EVENT, &g_D3D9Query);
	g_D3D9Device->BeginScene();

	return true;
}


void ShutdownD3D9 ()
{
	if (g_D3D9Query) {
		g_D3D9Query->Release();
		g_D3D9Query = NULL;
	}
	if (g_D3D9Device) {
		g_D3D9Device->Release();
		g_D3D9Device = NULL;
	}
	if (s_D3D) {
		s_D3D->Release();
		s_D3D = NULL;
	}
}


const MOJOSHADER_parseData* AssembleShaderD3D9 (const char* src, unsigned size)
{
	const MOJOSHADER_parseData* code = MOJOSHADER_assemble(
		NULL,
		src, size,
		NULL, 0, // comments
		NULL, 0, // symbols
		NULL, 0, // defines
		NULL, NULL, // include
		NULL, NULL, NULL // alloc
		);
	if (code->error_count)
	{
		for (int i = 0; i < code->error_count; ++i)
		{
			printf ("ERROR: d3d shader assembly failed: %s\n", code->errors[i]);
		}
		MOJOSHADER_freeParseData (code);
		return NULL;
	}
	return code;
}

void ReleaseAssembledShaderD3D9 (const MOJOSHADER_parseData* data)
{
	MOJOSHADER_freeParseData (data);
}
