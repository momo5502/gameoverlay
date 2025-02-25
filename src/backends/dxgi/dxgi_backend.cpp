#include "dxgi_backend.hpp"

#include <backend_dxgi.hpp>
#include <utils/hook.hpp>
#include <utils/dummy_window.hpp>

namespace gameoverlay::dxgi
{
    namespace
    {
        struct hooks
        {
            utils::hook::detour swap_chain_present{};
        };

        hooks& get_hooks()
        {
            static hooks h{};
            return h;
        }

        // TODO: Synchronize access with object destruction
        dxgi_backend* g_backend{nullptr};

        void draw_frame(IDXGISwapChain* swap_chain)
        {
            if (!swap_chain || !g_backend)
            {
                return;
            }

            g_backend->create_or_access_renderer(
                swap_chain,
                [](dxgi_renderer& r) {
                    r.draw_frame(); //
                },
                *swap_chain);
        }

        HRESULT WINAPI swap_chain_present_stub(IDXGISwapChain* swap_chain, const UINT sync_interval, const UINT flags)
        {
            draw_frame(swap_chain);
            return get_hooks().swap_chain_present.invoke_stdcall<HRESULT>(swap_chain, sync_interval, flags);
        }
    }

    dxgi_backend::~dxgi_backend()
    {
        g_backend = nullptr;
    }

    dxgi_backend::dxgi_backend(owned_handler h)
        : typed_backed(std::move(h))
    {
        g_backend = this;

        const utils::dummy_window window{};

        CComPtr<IDXGISwapChain> swap_chain{};
        DXGI_SWAP_CHAIN_DESC swap_chain_desc{};

        constexpr D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };

        swap_chain_desc.BufferCount = 1;
        swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_chain_desc.OutputWindow = window.get();
        swap_chain_desc.SampleDesc.Count = 1;
        swap_chain_desc.Windowed = TRUE;
        swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_NULL, NULL, 0, featureLevels,
                                                 _ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &swap_chain_desc,
                                                 &swap_chain, NULL, NULL, NULL)) ||
            !swap_chain)
        {
            return;
        }

        auto* swap_chain_present = *utils::hook::get_vtable_entry(&*swap_chain, &IDXGISwapChain::Present);
        get_hooks().swap_chain_present.create(swap_chain_present, swap_chain_present_stub);
    }

    std::unique_ptr<backend> create_backend(backend::owned_handler h)
    {
        return std::make_unique<dxgi_backend>(std::move(h));
    }
}
