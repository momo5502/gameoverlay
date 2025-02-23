#pragma once

#include "d3d9_backend.hpp"
#include "d3d9_renderer.hpp"

namespace gameoverlay::d3d9
{
    struct d3d9_backend : public typed_backed<backend_type::d3d9, d3d9_renderer, IDirect3DDevice9*>
    {
        d3d9_backend(owned_handler h);
        ~d3d9_backend() override;
    };
}
