#pragma once

#include "d3d8_canvas.hpp"

#include <memory>
#include <renderer.hpp>

namespace gameoverlay::d3d8
{
    class d3d8_renderer : public window_renderer<backend_type::d3d8>
    {
      public:
        d3d8_renderer(owned_handler h, IDirect3DDevice8& device);
        void draw_frame();
        void reset();

      private:
        IDirect3DDevice8* device_{};
        std::unique_ptr<d3d8_canvas> canvas_{};
    };
}
