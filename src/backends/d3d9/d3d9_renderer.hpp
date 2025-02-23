#pragma once

#include "d3d9_canvas.hpp"

#include <memory>
#include <renderer.hpp>

namespace gameoverlay::d3d9
{
    class d3d9_renderer : public window_renderer<backend_type::d3d9>
    {
      public:
        d3d9_renderer(owned_handler h, IDirect3DDevice9& device);
        void draw_frame();
        void reset();

      private:
        IDirect3DDevice9* device_{};
        std::unique_ptr<d3d9_canvas> canvas_{};
    };
}
