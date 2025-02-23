#include "std_include.hpp"

#include "../../utils/hook.hpp"
#include "../../backend_registry.hpp"
#include "../../utils/concurrent_map.hpp"

#include "d3d9_renderer.hpp"

namespace gameoverlay::d3d9
{
	namespace
	{
		utils::concurrency::map<IDirect3DDevice9*, std::unique_ptr<renderer>> renderers{};

		utils::hook::detour device_reset_hook;
		utils::hook::detour device_release_hook;
		utils::hook::detour device_present_hook;
		utils::hook::detour swap_chain_present_hook;

		void draw_frame(IDirect3DDevice9* device)
		{
			if (!device) return;

			renderers.access(device, [](const std::unique_ptr<renderer>& renderer)
			{
				renderer->draw_frame();
			}, [device]
			{
				return std::make_unique<renderer>(device);
			});
		}

		ULONG WINAPI release_device(IDirect3DDevice9* device)
		{
			return device_release_hook.invoke_stdcall<ULONG>(device);
		}

		ULONG get_device_ref_count(IDirect3DDevice9* device)
		{
			if (!device)
			{
				return 0;
			}

			device->AddRef();
			return release_device(device);
		}

		HRESULT WINAPI device_reset_stub(IDirect3DDevice9* device, void* presentation_parameters)
		{
			return device_reset_hook.invoke_stdcall<HRESULT>(device, presentation_parameters);
		}

		ULONG WINAPI device_release_stub(IDirect3DDevice9* device)
		{
			if (get_device_ref_count(device) == 1)
			{
				renderers.remove(device);
			}

			return release_device(device);
		}

		HRESULT WINAPI device_present_stub(IDirect3DDevice9* device, const RECT* source_rect, const RECT* dest_rect,
		                                   const HWND dest_window_override, const RGNDATA* dirty_region)
		{
			draw_frame(device);
			return device_present_hook.invoke_stdcall<HRESULT>(device, source_rect, dest_rect, dest_window_override,
			                                                   dirty_region);
		}

		HRESULT WINAPI swap_chain_present_stub(IDirect3DSwapChain9* swap_chain, const RECT* source_rect,
		                                       const RECT* dest_rect,
		                                       const HWND dest_window_override, const RGNDATA* dirty_region,
		                                       const DWORD flags)
		{
			CComPtr<IDirect3DDevice9> device{};
			swap_chain->GetDevice(&device);

			draw_frame(device);

			return swap_chain_present_hook.invoke_stdcall<HRESULT>(swap_chain, source_rect, dest_rect,
			                                                       dest_window_override, dirty_region, flags);
		}

		class backend : public ::gameoverlay::backend
		{
			void initialize() override
			{
				const CComPtr direct3d = Direct3DCreate9(D3D_SDK_VERSION);
				if (!direct3d) return;

				D3DPRESENT_PARAMETERS pres_params{};
				ZeroMemory(&pres_params, sizeof(pres_params));
				pres_params.Windowed = TRUE;
				pres_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
				pres_params.BackBufferFormat = D3DFMT_UNKNOWN;
				pres_params.BackBufferCount = 1;

				CComPtr<IDirect3DDevice9> device{};
				direct3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, GetDesktopWindow(),
				                       D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &pres_params,
				                       &device);
				if (!device)
				{
					return;
				}

				CComPtr<IDirect3DSwapChain9> swap_chain{};
				device->GetSwapChain(0, &swap_chain);

				auto* device_release = *utils::hook::get_vtable_entry(&*device, &IDirect3DDevice9::Release);
				device_release_hook.create(device_release, device_release_stub);

				auto* device_present = *utils::hook::get_vtable_entry(&*device, &IDirect3DDevice9::Present);
				device_present_hook.create(device_present, device_present_stub);

				auto* device_reset = *utils::hook::get_vtable_entry(&*device, &IDirect3DDevice9::Reset);
				device_reset_hook.create(device_reset, device_reset_stub);

				if (swap_chain)
				{
					auto* swap_chain_present = *utils::hook::get_vtable_entry(
						&*swap_chain, &IDirect3DSwapChain9::Present);
					swap_chain_present_hook.create(swap_chain_present, swap_chain_present_stub);
				}
			}

			void on_window_destruction(const HWND window) override
			{
				renderers.remove_if([&](IDirect3DDevice9*, const std::unique_ptr<renderer>& renderer) -> bool
				{
					return renderer->get_window() == window;
				});
			}
		};
	}
}

REGISTER_BACKEND(gameoverlay::d3d9::backend);
