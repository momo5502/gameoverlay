#pragma once

#include <memory>
#include <renderer.hpp>

#include "vulkan_canvas.hpp"

namespace gameoverlay::vulkan
{
    class vulkan_renderer : public typed_renderer<renderer_type::vulkan>
    {
      public:
        vulkan_renderer(owned_handler h, VkDevice device, VkSwapchainKHR swap_chain);
        void draw_frame(VkQueue queue);

        HWND get_window() const override
        {
            return nullptr;
        }

      private:
        VkDevice device_{};
        VkSwapchainKHR swap_chain_{};
        std::unique_ptr<vulkan_canvas> canvas_{};
    };
}
