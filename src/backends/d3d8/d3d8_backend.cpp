#include "d3d8_backend.hpp"

#include <backend_d3d8.hpp>
#include <utils/hook.hpp>

namespace gameoverlay::d3d8
{
    namespace
    {
        struct hooks
        {
            utils::hook::detour device_present{};
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

        HRESULT WINAPI device_present_stub(IDirect3DDevice8* device, const RECT* source_rect, const RECT* dest_rect,
                                           const HWND dest_window_override, const RGNDATA* dirty_region)
        {
            draw_frame(device);
            return get_hooks().device_present.invoke_stdcall<HRESULT>(device, source_rect, dest_rect,
                                                                      dest_window_override, dirty_region);
        }
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

        D3DPRESENT_PARAMETERS pres_params{};
        ZeroMemory(&pres_params, sizeof(pres_params));
        pres_params.Windowed = TRUE;
        pres_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
        pres_params.BackBufferFormat = D3DFMT_UNKNOWN;
        pres_params.BackBufferCount = 1;

        CComPtr<IDirect3DDevice8> device{};
        direct3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, GetDesktopWindow(),
                               D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &pres_params, &device);
        if (!device)
        {
            return;
        }

        auto& hooks = get_hooks();

        auto* device_present = *utils::hook::get_vtable_entry(&*device, &IDirect3DDevice8::Present);
        hooks.device_present.create(device_present, device_present_stub);

        // auto* device_reset = *utils::hook::get_vtable_entry(&*device, &IDirect3DDevice9::Reset);
        // hooks.device_reset.create(device_reset, device_reset_stub);
    }

    std::unique_ptr<backend> create_backend(backend::owned_handler h)
    {
        return std::make_unique<d3d8_backend>(std::move(h));
    }
}
