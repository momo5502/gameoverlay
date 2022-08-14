#include "std_include.hpp"
#include "../../utils/hook.hpp"
#include "../../gameoverlay.hpp"
#include "../../utils/concurrency.hpp"
#include <d3d9.h>
#include <atlbase.h>

#include <shared_mutex>

#pragma comment(lib, "d3d9.lib")

namespace gameoverlay::d3d9
{
	namespace
	{
		class renderer
		{
		public:
			void draw_frame()
			{
			}
		};

		using shared_lock_t = std::shared_lock<std::shared_mutex>;
		using renderer_map = std::unordered_map<IDirect3DDevice9*, std::unique_ptr<renderer>>;
		utils::concurrency::container<renderer_map, std::shared_mutex> renderers{};

		utils::hook::detour device_reset_hook;
		utils::hook::detour device_release_hook;
		utils::hook::detour device_present_hook;
		utils::hook::detour swap_chain_present_hook;

		bool try_access_renderer(IDirect3DDevice9* device, const std::function<void(renderer&)>& callback = {})
		{
			return renderers.access<bool, shared_lock_t>([device, &callback](const renderer_map& r)
			{
				const auto entry = r.find(device);
				if (entry != r.end())
				{
					if (callback)
					{
						callback(*entry->second);
					}
					return true;
				}

				return false;
			});
		}

		void access_renderer(IDirect3DDevice9* device, const std::function<void(renderer&)>& callback)
		{
			if (try_access_renderer(device, callback))
			{
				return;
			}

			renderers.access([device](renderer_map& r)
			{
				r[device] = std::make_unique<renderer>();
			});

			if (!try_access_renderer(device, callback))
			{
				throw std::runtime_error("WAT?");
			}
		}

		void draw_frame(IDirect3DDevice9* device)
		{
			if (!device) return;

			access_renderer(device, [](renderer& renderer)
			{
				renderer.draw_frame();
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
				renderers.access([device](renderer_map& r)
				{
					const auto entry = r.find(device);
					if (entry != r.end())
					{
						r.erase(entry);
					}
				});
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
			CComPtr<IDirect3DDevice9> device;
			swap_chain->GetDevice(&device);

			draw_frame(device);

			return swap_chain_present_hook.invoke_stdcall<HRESULT>(swap_chain, source_rect, dest_rect,
			                                                       dest_window_override, dirty_region, flags);
		}
	}

	void initialize()
	{
		const CComPtr<IDirect3D9> direct3d = Direct3DCreate9(D3D_SDK_VERSION);
		if (!direct3d) return;

		D3DPRESENT_PARAMETERS pres_params{};
		ZeroMemory(&pres_params, sizeof(pres_params));
		pres_params.Windowed = TRUE;
		pres_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
		pres_params.BackBufferFormat = D3DFMT_UNKNOWN;
		pres_params.BackBufferCount = 1;

		CComPtr<IDirect3DDevice9> device{};
		direct3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, GetDesktopWindow(),
		                       D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &pres_params, &device);
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
			auto* swap_chain_present = *utils::hook::get_vtable_entry(&*swap_chain, &IDirect3DSwapChain9::Present);
			swap_chain_present_hook.create(swap_chain_present, swap_chain_present_stub);
		}
	}

	static register_backend _(initialize);
}
