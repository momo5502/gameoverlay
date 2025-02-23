#pragma once

#include "d3d9_canvas.hpp"

#include <memory>
#include <renderer.hpp>

namespace gameoverlay::d3d9
{
    class renderer : public window_renderer<backend::type::d3d9>
    {
      public:
        renderer(IDirect3DDevice9& device);
        void draw_frame();
        void reset();

      private:
        IDirect3DDevice9* device_{};
        std::unique_ptr<canvas> canvas_{};
    };
}
