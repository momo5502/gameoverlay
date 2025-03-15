#include "vulkan_renderer.hpp"

#include <stdexcept>

namespace gameoverlay::vulkan
{
    namespace
    {

    }

    vulkan_renderer::vulkan_renderer(owned_handler h, const VkDevice device, const VkSwapchainKHR swap_chain)
        : typed_renderer(std::move(h)),
          device_(device),
          swap_chain_(swap_chain)
    {
    }

    void vulkan_renderer::draw_frame(const VkQueue queue)
    {
        // const auto current_dim = get_dimensions(this->hdc_);
        // if (!this->canvas_ || this->canvas_->get_dimensions() != current_dim)
        //{
        //     this->canvas_ = std::make_unique<vulkan_canvas>(current_dim.width, current_dim.height);
        // }

        if (!this->canvas_)
        {
            return;
        }

        this->handle_new_frame(*this->canvas_);
        this->canvas_->draw();
    }
}
