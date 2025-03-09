#include "d3d9_backend.hpp"

#include <backend_d3d9.hpp>

#include <utils/hook.hpp>
#include <utils/dummy_window.hpp>

namespace gameoverlay::d3d9
{
    namespace
    {
        struct hooks
        {
            utils::hook::detour device_reset{};
            utils::hook::detour device_present{};
            utils::hook::detour swap_chain_present{};
        };

        hooks& get_hooks()
        {
            static hooks h{};
            return h;
        }

        // TODO: Synchronize access with object destruction
        d3d9_backend* g_backend{nullptr};

        void draw_frame(IDirect3DDevice9* device)
        {
            if (!device || !g_backend)
            {
                return;
            }

            g_backend->create_or_access_renderer(
                device,
                [](d3d9_renderer& r) {
                    r.draw_frame(); //
                },
                *device);
        }

        void reset_device(IDirect3DDevice9* device)
        {
            if (!device || !g_backend)
            {
                return;
            }

            g_backend->access_renderer(device, [](d3d9_renderer& r) {
                r.reset(); //
            });
        }

        HRESULT WINAPI device_reset_stub(IDirect3DDevice9* device, void* presentation_parameters)
        {
            reset_device(device);
            return get_hooks().device_reset.invoke_stdcall<HRESULT>(device, presentation_parameters);
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
    }

    d3d9_backend::~d3d9_backend()
    {
        g_backend = nullptr;
    }

    d3d9_backend::d3d9_backend(owned_handler h)
        : typed_backed(std::move(h))
    {
        g_backend = this;

        const CComPtr direct3d = Direct3DCreate9(D3D_SDK_VERSION);
        if (!direct3d)
        {
            return;
        }

        utils::dummy_window window{};

        D3DDISPLAYMODE display_mode{};
        direct3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &display_mode);

        D3DPRESENT_PARAMETERS pres_params{};
        ZeroMemory(&pres_params, sizeof(pres_params));
        pres_params.Windowed = TRUE;
        pres_params.BackBufferWidth = 640;
        pres_params.BackBufferHeight = 480;
        pres_params.SwapEffect = D3DSWAPEFFECT_FLIP;
        pres_params.BackBufferFormat = display_mode.Format;
        pres_params.BackBufferCount = 1;

        CComPtr<IDirect3DDevice9> device{};
        direct3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window.get(), D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                               &pres_params, &device);
        if (!device)
        {
            return;
        }

        CComPtr<IDirect3DSwapChain9> swap_chain{};
        device->GetSwapChain(0, &swap_chain);

        auto& hooks = get_hooks();

        auto* device_present = *utils::hook::get_vtable_entry(&*device, &IDirect3DDevice9::Present);
        hooks.device_present.create(device_present, &device_present_stub);

        auto* device_reset = *utils::hook::get_vtable_entry(&*device, &IDirect3DDevice9::Reset);
        hooks.device_reset.create(device_reset, &device_reset_stub);

        if (swap_chain)
        {
            auto* swap_chain_present = *utils::hook::get_vtable_entry(&*swap_chain, &IDirect3DSwapChain9::Present);
            hooks.swap_chain_present.create(swap_chain_present, &swap_chain_present_stub);
        }
    }

    std::unique_ptr<backend> create_backend(backend::owned_handler h)
    {
        return std::make_unique<d3d9_backend>(std::move(h));
    }
}
