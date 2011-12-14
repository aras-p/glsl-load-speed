#pragma once

#ifdef _MSC_VER
#define D3D9_AVAILABLE 1
#endif

#ifdef D3D9_AVAILABLE

#include "include/d3d9.h"
#include "mojoshader/mojoshader.h"

extern IDirect3DDevice9* g_D3D9Device;
extern IDirect3DQuery9* g_D3D9Query;

bool InitializeD3D9 (HWND window);
void ShutdownD3D9 ();

const MOJOSHADER_parseData* AssembleShaderD3D9 (const char* src, unsigned size);
void ReleaseAssembledShaderD3D9 (const MOJOSHADER_parseData* data);

#endif // D3D9_AVAILABLE
