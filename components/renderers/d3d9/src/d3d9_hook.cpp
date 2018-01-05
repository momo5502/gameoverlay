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
#include "d3d9_hook.hpp"
#pragma comment(lib, "d3d9.lib")

namespace gameoverlay
{
	d3d9_hook* d3d9_hook::instance = nullptr;

	void d3d9_hook::frame_handler(void* device)
	{
		if (!device) return;
		if (this->callback) this->callback(device);
	}

	HRESULT WINAPI d3d9_hook::reset(void* device, void* presentation_parameters)
	{
		return this->reset_hook.invoke_pascal<HRESULT>(device, presentation_parameters);
	}

	HRESULT WINAPI d3d9_hook::reset_stub(void* device, void* presentation_parameters)
	{
		if (!d3d9_hook::instance) return E_FAIL;
		return d3d9_hook::instance->reset(device, presentation_parameters);
	}

	HRESULT WINAPI d3d9_hook::swap_chain_present(void* swap_chain, CONST RECT* source_rect, CONST RECT* dest_rect, HWND dest_window_override, CONST RGNDATA* dirty_region, DWORD flags)
	{
		this->present_hook.remove();

		IDirect3DDevice9* device;
		IDirect3DSwapChain9* swap_chain9 = reinterpret_cast<IDirect3DSwapChain9*>(swap_chain);
		IDirect3DSwapChain9_GetDevice(swap_chain9, &device);

		this->frame_handler(device);

		return this->swap_chain_present_hook.invoke_pascal<HRESULT>(swap_chain, source_rect, dest_rect, dest_window_override, dirty_region, flags);
	}

	HRESULT WINAPI d3d9_hook::swap_chain_present_stub(void* swap_chain, CONST RECT* source_rect, CONST RECT* dest_rect, HWND dest_window_override, CONST RGNDATA* dirty_region, DWORD flags)
	{
		if (!d3d9_hook::instance) return E_FAIL;
		return d3d9_hook::instance->swap_chain_present(swap_chain, source_rect, dest_rect, dest_window_override, dirty_region, flags);
	}

	HRESULT WINAPI d3d9_hook::present(void* device, CONST RECT* source_rect, CONST RECT* dest_rect, HWND dest_window_override, CONST RGNDATA* dirty_region)
	{
		this->swap_chain_present_hook.remove();

		this->frame_handler(device);

		return this->present_hook.invoke_pascal<HRESULT>(device, source_rect, dest_rect, dest_window_override, dirty_region);
	}

	HRESULT WINAPI d3d9_hook::present_stub(void* device, CONST RECT* source_rect, CONST RECT* dest_rect, HWND dest_window_override, CONST RGNDATA* dirty_region)
	{
		if (!d3d9_hook::instance) return E_FAIL;
		return d3d9_hook::instance->present(device, source_rect, dest_rect, dest_window_override, dirty_region);
	}

	HRESULT WINAPI d3d9_hook::endscene(void* device)
	{
		IDirect3DDevice9* device9 = reinterpret_cast<IDirect3DDevice9*>(device);

		this->endscene_hook.remove();
		this->reset_hook.create(device9->lpVtbl->Reset, d3d9_hook::reset_stub);
		this->present_hook.create(device9->lpVtbl->Present, d3d9_hook::present_stub);

		IDirect3DSwapChain9* swap_chain;
		IDirect3DDevice9_GetSwapChain(device9, 0, &swap_chain);
		if (swap_chain)
		{
			this->swap_chain_present_hook.create(swap_chain->lpVtbl->Present, d3d9_hook::swap_chain_present_stub);
		}

		return IDirect3DDevice9_EndScene(device9);
	}

	HRESULT WINAPI d3d9_hook::endscene_stub(void* device)
	{
		if(!d3d9_hook::instance) return E_FAIL;
		return d3d9_hook::instance->endscene(device);
	}

	void d3d9_hook::on_frame(std::function<void(void*)> _callback)
	{
		this->callback = _callback;
	}

	void d3d9_hook::unhook()
	{
		this->endscene_hook.remove();
		this->reset_hook.remove();
		this->present_hook.remove();
		this->swap_chain_present_hook.remove();
	}

	d3d9_hook::d3d9_hook()
	{
		d3d9_hook::instance = this;

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

		this->endscene_hook.create(device->lpVtbl->EndScene, d3d9_hook::endscene_stub);

		IDirect3DDevice9_Release(device);
		IDirect3D9_Release(direct3d);
	}

	d3d9_hook::~d3d9_hook()
	{
		d3d9_hook::instance = nullptr;
	}
}
