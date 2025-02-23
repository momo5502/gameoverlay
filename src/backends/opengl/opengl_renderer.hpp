#pragma once

#include "opengl_canvas.hpp"

#include <memory>

#include <renderer.hpp>

namespace gameoverlay::opengl
{
    class renderer : public window_renderer<backend::type::opengl>
    {
      public:
        renderer(HDC hdc);
        void draw_frame();

      private:
        HDC hdc_{};
        std::unique_ptr<canvas> canvas_{};
    };
}
