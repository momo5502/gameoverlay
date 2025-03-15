#pragma once

#include <backend.hpp>

#include "d3d8_traits.hpp"
#include "d3d_legacy_renderer.hpp"

namespace gameoverlay::d3d8
{
    using d3d8_renderer = d3d_legacy::d3d_legacy_renderer<renderer_type::d3d8, d3d8_traits>;

    struct d3d8_backend : typed_backed<d3d8_renderer, IDirect3DDevice8*>
    {
        d3d8_backend(owned_handler h);
        ~d3d8_backend() override;
    };
}
