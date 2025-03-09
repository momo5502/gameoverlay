#include "d3d8_backend.hpp"

#include <backend_d3d8.hpp>

#include <utils/hook.hpp>
#include <utils/dummy_window.hpp>

// #define HOOK_SWAP_CHAIN_PRESENT

namespace gameoverlay::d3d8
{
    namespace
    {
        struct hooks
        {
            utils::hook::detour device_reset{};
            utils::hook::detour device_present{};
#ifdef HOOK_SWAP_CHAIN_PRESENT
            utils::hook::detour swap_chain_present{};
#endif
        };

        hooks& get_hooks()
        {
            static hooks h{};
            return h;
        }

        // TODO: Synchronize access with object destruction
        d3d8_backend* g_backend{nullptr};

        void draw_frame(IDirect3DDevice8* device)
        {
            if (!device || !g_backend)
            {
                return;
            }

            g_backend->create_or_access_renderer(
                device,
                [](d3d8_renderer& r) {
                    r.draw_frame(); //
                },
                *device);
        }

        void reset_device(IDirect3DDevice8* device)
        {
            if (!device || !g_backend)
            {
                return;
            }

            g_backend->access_renderer(device, [](d3d8_renderer& r) {
                r.reset(); //
            });
        }

        HRESULT WINAPI device_reset_stub(IDirect3DDevice8* device, void* presentation_parameters)
        {
            reset_device(device);
            return get_hooks().device_reset.invoke_stdcall<HRESULT>(device, presentation_parameters);
        }

        HRESULT WINAPI device_present_stub(IDirect3DDevice8* device, const RECT* source_rect, const RECT* dest_rect,
                                           const HWND dest_window_override, const RGNDATA* dirty_region)
        {
            draw_frame(device);
            return get_hooks().device_present.invoke_stdcall<HRESULT>(device, source_rect, dest_rect,
                                                                      dest_window_override, dirty_region);
        }

#ifdef HOOK_SWAP_CHAIN_PRESENT
        HRESULT WINAPI swap_chain_present_stub(IDirect3DSwapChain8* swap_chain, const RECT* source_rect,
                                               const RECT* dest_rect, const HWND dest_window_override,
                                               const RGNDATA* dirty_region)
        {
            CComPtr<IDirect3DDevice8> device{};
            swap_chain->QueryInterface(IID_PPV_ARGS(&device));

            draw_frame(device);

            return get_hooks().swap_chain_present.invoke_stdcall<HRESULT>(swap_chain, source_rect, dest_rect,
                                                                          dest_window_override, dirty_region);
        }
#endif
    }

    d3d8_backend::~d3d8_backend()
    {
        g_backend = nullptr;
    }

    d3d8_backend::d3d8_backend(owned_handler h)
        : typed_backed(std::move(h))
    {
        g_backend = this;

        const CComPtr direct3d = Direct3DCreate8(D3D_SDK_VERSION);
        if (!direct3d)
        {
            return;
        }

        D3DCAPS8 aps;
        direct3d->GetDeviceCaps(0, D3DDEVTYPE_HAL, &aps);

        utils::dummy_window window{};

        D3DPRESENT_PARAMETERS pres_params{};
        ZeroMemory(&pres_params, sizeof(pres_params));
        pres_params.Windowed = TRUE;
        pres_params.BackBufferWidth = 640;
        pres_params.BackBufferHeight = 480;
        pres_params.SwapEffect = D3DSWAPEFFECT_FLIP;
        pres_params.BackBufferFormat = D3DFMT_A8R8G8B8;
        pres_params.BackBufferCount = 1;

        CComPtr<IDirect3DDevice8> device{};
        direct3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window.get(), D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                               &pres_params, &device);
        if (!device)
        {
            return;
        }

        auto& hooks = get_hooks();

        auto* device_present = *utils::hook::get_vtable_entry(&*device, &IDirect3DDevice8::Present);
        hooks.device_present.create(device_present, &device_present_stub);

        auto* device_reset = *utils::hook::get_vtable_entry(&*device, &IDirect3DDevice8::Reset);
        hooks.device_reset.create(device_reset, &device_reset_stub);

#ifdef HOOK_SWAP_CHAIN_PRESENT
        CComPtr<IDirect3DSwapChain8> swap_chain{};
        device->CreateAdditionalSwapChain(&pres_params, &swap_chain);

        if (swap_chain)
        {
            auto* swap_chain_present = *utils::hook::get_vtable_entry(&*swap_chain, &IDirect3DSwapChain8::Present);
            hooks.swap_chain_present.create(swap_chain_present, &swap_chain_present_stub);
        }
#endif
    }

    std::unique_ptr<backend> create_backend(backend::owned_handler h)
    {
        return std::make_unique<d3d8_backend>(std::move(h));
    }
}
