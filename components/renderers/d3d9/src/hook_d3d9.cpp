#include "std_include.hpp"

#undef DECLARE_INTERFACE
#define DECLARE_INTERFACE(iface)        \
typedef interface iface {               \
	struct iface##Vtbl FAR* lpVtbl;     \
} iface;                                \
typedef struct iface##Vtbl iface##Vtbl; \
struct iface##Vtbl

#undef DECLARE_INTERFACE_
#define DECLARE_INTERFACE_(iface, base) DECLARE_INTERFACE(iface)

#undef STDMETHOD
#define STDMETHOD(method) HRESULT (STDMETHODCALLTYPE * method)

#undef STDMETHOD_
#define STDMETHOD_(type,method) type (STDMETHODCALLTYPE * method)

#undef PURE
#define PURE

#undef THIS
#define THIS INTERFACE FAR* This

#undef THIS_
#define THIS_ INTERFACE FAR* This,

#define CINTERFACE
#include <d3d9.h>
#include "hook_d3d9.hpp"
#pragma comment(lib, "d3d9.lib")

namespace gameoverlay
{
	hook_d3d9* hook_d3d9::instance = nullptr;

	void hook_d3d9::frame_handler(void* device)
	{
		if (!device) return;
		if (this->frame_callback) this->frame_callback(device);
	}

	void hook_d3d9::reset_handler(void* device)
	{
		if (!device) return;
		if (this->reset_callback) this->reset_callback(device);
	}

	HRESULT WINAPI hook_d3d9::reset(void* device, void* presentation_parameters)
	{
		this->reset_handler(device);
		return this->reset_hook.invoke_pascal<HRESULT>(device, presentation_parameters);
	}

	HRESULT WINAPI hook_d3d9::reset_stub(void* device, void* presentation_parameters)
	{
		if (!hook_d3d9::instance) return E_FAIL;
		return hook_d3d9::instance->reset(device, presentation_parameters);
	}

	HRESULT WINAPI hook_d3d9::swap_chain_present(void* swap_chain, CONST RECT* source_rect, CONST RECT* dest_rect, HWND dest_window_override, CONST RGNDATA* dirty_region, DWORD flags)
	{
		this->present_hook.remove();

		IDirect3DDevice9* device;
		IDirect3DSwapChain9* swap_chain9 = reinterpret_cast<IDirect3DSwapChain9*>(swap_chain);
		IDirect3DSwapChain9_GetDevice(swap_chain9, &device);

		this->frame_handler(device);

		return this->swap_chain_present_hook.invoke_pascal<HRESULT>(swap_chain, source_rect, dest_rect, dest_window_override, dirty_region, flags);
	}

	HRESULT WINAPI hook_d3d9::swap_chain_present_stub(void* swap_chain, CONST RECT* source_rect, CONST RECT* dest_rect, HWND dest_window_override, CONST RGNDATA* dirty_region, DWORD flags)
	{
		if (!hook_d3d9::instance) return E_FAIL;
		return hook_d3d9::instance->swap_chain_present(swap_chain, source_rect, dest_rect, dest_window_override, dirty_region, flags);
	}

	HRESULT WINAPI hook_d3d9::present(void* device, CONST RECT* source_rect, CONST RECT* dest_rect, HWND dest_window_override, CONST RGNDATA* dirty_region)
	{
		this->swap_chain_present_hook.remove();

		this->frame_handler(device);

		return this->present_hook.invoke_pascal<HRESULT>(device, source_rect, dest_rect, dest_window_override, dirty_region);
	}

	HRESULT WINAPI hook_d3d9::present_stub(void* device, CONST RECT* source_rect, CONST RECT* dest_rect, HWND dest_window_override, CONST RGNDATA* dirty_region)
	{
		if (!hook_d3d9::instance) return E_FAIL;
		return hook_d3d9::instance->present(device, source_rect, dest_rect, dest_window_override, dirty_region);
	}

	HRESULT WINAPI hook_d3d9::endscene(void* device)
	{
		IDirect3DDevice9* device9 = reinterpret_cast<IDirect3DDevice9*>(device);

		this->endscene_hook.remove();
		this->reset_hook.create(device9->lpVtbl->Reset, hook_d3d9::reset_stub);
		this->present_hook.create(device9->lpVtbl->Present, hook_d3d9::present_stub);

		IDirect3DSwapChain9* swap_chain;
		IDirect3DDevice9_GetSwapChain(device9, 0, &swap_chain);
		if (swap_chain)
		{
			this->swap_chain_present_hook.create(swap_chain->lpVtbl->Present, hook_d3d9::swap_chain_present_stub);
		}

		return IDirect3DDevice9_EndScene(device9);
	}

	HRESULT WINAPI hook_d3d9::endscene_stub(void* device)
	{
		if(!hook_d3d9::instance) return E_FAIL;
		return hook_d3d9::instance->endscene(device);
	}

	void hook_d3d9::on_frame(std::function<void(void*)> callback)
	{
		this->frame_callback = callback;
	}

	void hook_d3d9::on_reset(std::function<void(void*)> callback)
	{
		this->reset_callback = callback;
	}

	void hook_d3d9::unhook()
	{
		this->endscene_hook.remove();
		this->reset_hook.remove();
		this->present_hook.remove();
		this->swap_chain_present_hook.remove();
	}

	hook_d3d9::hook_d3d9()
	{
		hook_d3d9::instance = this;

		IDirect3D9* direct3d = Direct3DCreate9(D3D_SDK_VERSION);
		if (!direct3d) return;

		D3DPRESENT_PARAMETERS presParams;

		ZeroMemory(&presParams, sizeof(presParams));
		presParams.Windowed = TRUE;
		presParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
		presParams.BackBufferFormat = D3DFMT_UNKNOWN;

		IDirect3DDevice9* device = nullptr;
		IDirect3D9_CreateDevice(direct3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, GetDesktopWindow(), D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &presParams, &device);
		if (!device) { IDirect3D9_Release(direct3d); return; }

		this->endscene_hook.create(device->lpVtbl->EndScene, hook_d3d9::endscene_stub);

		IDirect3DDevice9_Release(device);
		IDirect3D9_Release(direct3d);
	}

	hook_d3d9::~hook_d3d9()
	{
		hook_d3d9::instance = nullptr;
	}
}
