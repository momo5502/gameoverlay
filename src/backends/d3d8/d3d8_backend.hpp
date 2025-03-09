#pragma once

#include <backend.hpp>
#include "d3d8_renderer.hpp"

namespace gameoverlay::d3d8
{
    struct d3d8_backend : public typed_backed<backend_type::d3d8, d3d8_renderer, IDirect3DDevice8*>
    {
        d3d8_backend(owned_handler h);
        ~d3d8_backend() override;
    };
}
