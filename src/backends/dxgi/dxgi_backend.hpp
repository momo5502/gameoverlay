#pragma once

#include <backend.hpp>
#include "dxgi_renderer.hpp"
#include "d3d12_command_queue_store.hpp"

namespace gameoverlay::dxgi
{
    struct dxgi_backend : public typed_backed<backend_type::dxgi, dxgi_renderer, IDXGISwapChain*>
    {
        dxgi_backend(owned_handler h);
        ~dxgi_backend() override;

        d3d12_command_queue_store store{};
    };
}
