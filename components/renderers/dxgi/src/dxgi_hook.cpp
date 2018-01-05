#include "std_include.hpp"

#define COBJMACROS
#define CINTERFACE
#define D3D11_NO_HELPERS
#include <dxgi.h>
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include "dxgi_hook.hpp"

namespace gameoverlay
{
	dxgi_hook* dxgi_hook::instance = nullptr;

	HRESULT WINAPI dxgi_hook::present(void* swap_chain, UINT sync_interval, UINT flags)
	{
		if (this->callback) this->callback(swap_chain);
		return this->present_hook.invoke_pascal<HRESULT>(swap_chain, sync_interval, flags);
	}

	HRESULT WINAPI dxgi_hook::present_stub(void* swap_chain, UINT sync_interval, UINT flags)
	{
		if (!dxgi_hook::instance) return E_FAIL;
		return dxgi_hook::instance->present(swap_chain, sync_interval, flags);
	}

	void dxgi_hook::on_frame(std::function<void(void*)> _callback)
	{
		this->callback = _callback;
	}

	dxgi_hook::dxgi_hook()
	{
		dxgi_hook::instance = this;

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

		this->present_hook.create(swap_chain->lpVtbl->Present, dxgi_hook::present_stub);

		IDXGISwapChain_Release(swap_chain);
	}

	dxgi_hook::~dxgi_hook()
	{
		dxgi_hook::instance = nullptr;
	}
}