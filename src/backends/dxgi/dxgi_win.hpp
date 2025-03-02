#pragma once

#include <utils/win.hpp>

#define D3D_FEATURE_LEVEL_1_0_GENERIC

#define D3D11_NO_HELPERS
#include <dxgi.h>
#include <dxgi1_5.h>
#include <d3d11.h>
#include <d3d12.h>
#include "directx/d3dx12.h"
#include <atlbase.h>

#include <D3Dcompiler.h>
#include <DirectXMath.h>

#pragma comment(lib, "d3d10.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "dxguid.lib")
