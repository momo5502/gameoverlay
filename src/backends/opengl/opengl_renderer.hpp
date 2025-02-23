#pragma once

#include <memory>
#include <renderer.hpp>

#include "opengl_canvas.hpp"

namespace gameoverlay::opengl
{
    class opengl_renderer : public window_renderer<backend_type::opengl>
    {
      public:
        opengl_renderer(owned_handler h, HDC hdc);
        void draw_frame();

      private:
        HDC hdc_{};
        std::unique_ptr<opengl_canvas> canvas_{};
    };
}
