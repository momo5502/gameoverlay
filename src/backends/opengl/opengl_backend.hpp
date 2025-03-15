#pragma once

#include <backend.hpp>
#include "opengl_renderer.hpp"

namespace gameoverlay::opengl
{
    struct opengl_backend : typed_backed<opengl_renderer, HDC>
    {
        opengl_backend(owned_handler h);
        ~opengl_backend() override;
    };
}
