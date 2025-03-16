#include "vulkan_renderer.hpp"

#include <stdexcept>

namespace gameoverlay::vulkan
{
    namespace
    {
        dimensions get_swap_chain_dimensions(const VkDevice device, const VkSwapchainKHR swap_chain)
        {
            VkImage image{};
            uint32_t count = 1;

            const auto res = vkGetSwapchainImagesKHR(device, swap_chain, &count, &image);

            if (count < 1)
            {
                throw std::runtime_error("Failed to get dimensions");
            }

            VkSubresourceLayout layout{};
            VkImageSubresource subresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0};

            vkGetImageSubresourceLayout(device, image, &subresource, &layout);

            return {};
        }
    }

    vulkan_renderer::vulkan_renderer(owned_handler h, const VkDevice device, const VkSwapchainKHR swap_chain,
                                     const VkSwapchainCreateInfoKHR& info)
        : typed_renderer(std::move(h)),
          device_(device),
          swap_chain_(swap_chain),
          info_(info)
    {
    }

    void vulkan_renderer::draw_frame(const VkQueue queue)
    {
        get_swap_chain_dimensions(device_, swap_chain_);

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
