#pragma once

#include <backend.hpp>
#include "opengl_renderer.hpp"

namespace gameoverlay::opengl
{
    struct opengl_backend : public typed_backed<backend_type::d3d9, opengl_renderer, HDC>
    {
        opengl_backend(owned_handler h);
        ~opengl_backend() override;
    };
}
