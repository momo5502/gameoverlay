#pragma once

#include <memory>
#include <renderer.hpp>

#include "vulkan_canvas.hpp"

namespace gameoverlay::vulkan
{
    class vulkan_renderer : public window_renderer<backend_type::vulkan>
    {
      public:
        vulkan_renderer(owned_handler h, HDC hdc);
        void draw_frame();

      private:
        HDC hdc_{};
        std::unique_ptr<vulkan_canvas> canvas_{};
    };
}
