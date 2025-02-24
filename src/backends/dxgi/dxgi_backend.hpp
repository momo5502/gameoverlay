#pragma once

#include <backend.hpp>
#include "dxgi_renderer.hpp"

namespace gameoverlay::dxgi
{
    struct dxgi_backend : public typed_backed<backend_type::dxgi, dxgi_renderer, IDXGISwapChain*>
    {
        dxgi_backend(owned_handler h);
        ~dxgi_backend() override;
    };
}
