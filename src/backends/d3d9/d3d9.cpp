#include <utils/hook.hpp>

#include "d3d9_renderer.hpp"
#include "utils/concurrency.hpp"

namespace gameoverlay::d3d9
{
    namespace
    {
        struct hooks
        {
            utils::hook::detour device_reset{};
            utils::hook::detour device_release{};
            utils::hook::detour device_present{};
            utils::hook::detour swap_chain_present{};
        };

        hooks& get_hooks()
        {
            static hooks h{};
            return h;
        }

        using renderer_map = std::unordered_map<IDirect3DDevice9*, renderer>;

        utils::concurrency::container<renderer_map, std::recursive_mutex> renderers;

        void draw_frame(IDirect3DDevice9* device)
        {
            if (!device)
            {
                return;
            }

            renderers.access([&](renderer_map& m) {
                auto entry = m.find(device);
                if (entry == m.end())
                {
                    entry = m.emplace(device, *device).first;
                }

                entry->second.draw_frame();
            });
        }

        ULONG WINAPI release_device(IDirect3DDevice9* device)
        {
            return get_hooks().device_release.invoke_stdcall<ULONG>(device);
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
            return get_hooks().device_reset.invoke_stdcall<HRESULT>(device, presentation_parameters);
        }

        ULONG WINAPI device_release_stub(IDirect3DDevice9* device)
        {
            renderers.access([&](renderer_map& m) {
                if (get_device_ref_count(device) > 1)
                {
                    return;
                }

                const auto entry = m.find(device);
                if (entry != m.end())
                {
                    m.erase(entry);
                }
            });

            return release_device(device);
        }

        HRESULT WINAPI device_present_stub(IDirect3DDevice9* device, const RECT* source_rect, const RECT* dest_rect,
                                           const HWND dest_window_override, const RGNDATA* dirty_region)
        {
            draw_frame(device);
            return get_hooks().device_present.invoke_stdcall<HRESULT>(device, source_rect, dest_rect,
                                                                      dest_window_override, dirty_region);
        }

        HRESULT WINAPI swap_chain_present_stub(IDirect3DSwapChain9* swap_chain, const RECT* source_rect,
                                               const RECT* dest_rect, const HWND dest_window_override,
                                               const RGNDATA* dirty_region, const DWORD flags)
        {
            CComPtr<IDirect3DDevice9> device{};
            swap_chain->GetDevice(&device);

            draw_frame(device);

            return get_hooks().swap_chain_present.invoke_stdcall<HRESULT>(swap_chain, source_rect, dest_rect,
                                                                          dest_window_override, dirty_region, flags);
        }

        class backend : public ::gameoverlay::backend
        {
            void initialize() override
            {
                const CComPtr direct3d = Direct3DCreate9(D3D_SDK_VERSION);
                if (!direct3d)
                    return;

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
                get_hooks().device_release.create(device_release, device_release_stub);

                auto* device_present = *utils::hook::get_vtable_entry(&*device, &IDirect3DDevice9::Present);
                get_hooks().device_present.create(device_present, device_present_stub);

                auto* device_reset = *utils::hook::get_vtable_entry(&*device, &IDirect3DDevice9::Reset);
                get_hooks().device_reset.create(device_reset, device_reset_stub);

                if (swap_chain)
                {
                    auto* swap_chain_present =
                        *utils::hook::get_vtable_entry(&*swap_chain, &IDirect3DSwapChain9::Present);
                    get_hooks().swap_chain_present.create(swap_chain_present, swap_chain_present_stub);
                }
            }

            void on_window_destruction(const HWND window) override
            {
                renderers.access([&](renderer_map& m) {
                    std::erase_if(m, [&](const renderer_map::value_type& entry) {
                        return entry.second.get_window() == window; //
                    });
                });
            }
        };
    }
}

// REGISTER_BACKEND(gameoverlay::d3d9::backend);
