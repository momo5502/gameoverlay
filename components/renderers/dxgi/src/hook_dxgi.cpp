#include "std_include.hpp"

#define COBJMACROS
#define CINTERFACE
#define D3D11_NO_HELPERS
#include <dxgi.h>
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include "hook_dxgi.hpp"

namespace gameoverlay
{
	hook_dxgi* hook_dxgi::instance = nullptr;

	HRESULT WINAPI hook_dxgi::present(void* swap_chain, UINT sync_interval, UINT flags)
	{
		if (this->callback) this->callback(swap_chain);
		return this->present_hook.invoke_pascal<HRESULT>(swap_chain, sync_interval, flags);
	}

	HRESULT WINAPI hook_dxgi::present_stub(void* swap_chain, UINT sync_interval, UINT flags)
	{
		if (!hook_dxgi::instance) return E_FAIL;
		return hook_dxgi::instance->present(swap_chain, sync_interval, flags);
	}

	void hook_dxgi::on_frame(std::function<void(void*)> _callback)
	{
		this->callback = _callback;
	}

	hook_dxgi::hook_dxgi()
	{
		hook_dxgi::instance = this;

		IDXGISwapChain* swap_chain = nullptr;
		DXGI_SWAP_CHAIN_DESC swap_chain_desc;
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

		ZeroMemory(&swap_chain_desc, sizeof(swap_chain_desc));
		swap_chain_desc.OutputWindow = GetDesktopWindow();
		swap_chain_desc.BufferCount = 1;
		swap_chain_desc.BufferDesc.Width = 1;
		swap_chain_desc.BufferDesc.Height = 1;
		swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swap_chain_desc.SampleDesc.Count = 1;

		if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_NULL, NULL, 0, &featureLevel, 1, D3D11_SDK_VERSION, &swap_chain_desc, &swap_chain, NULL, NULL, NULL))
			|| !swap_chain) return;

		this->present_hook.create(swap_chain->lpVtbl->Present, hook_dxgi::present_stub);

		IDXGISwapChain_Release(swap_chain);
	}

	hook_dxgi::~hook_dxgi()
	{
		hook_dxgi::instance = nullptr;
	}
}