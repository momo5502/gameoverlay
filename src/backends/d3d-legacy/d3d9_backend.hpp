#pragma once

#include <backend.hpp>

#include "d3d9_traits.hpp"
#include "d3d_legacy_renderer.hpp"

namespace gameoverlay::d3d9
{
    using d3d9_renderer = d3d_legacy::d3d_legacy_renderer<backend_type::d3d9, d3d9_traits>;

    struct d3d9_backend : typed_backed<d3d9_renderer::type, d3d9_renderer, IDirect3DDevice9*>
    {
        d3d9_backend(owned_handler h);
        ~d3d9_backend() override;
    };
}
