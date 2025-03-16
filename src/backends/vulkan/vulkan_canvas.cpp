#include "vulkan_canvas.hpp"

namespace gameoverlay::vulkan
{
    namespace
    {

    }

    vulkan_canvas::vulkan_canvas(const uint32_t width, const uint32_t height)
        : fixed_canvas(width, height)
    {
    }

    void vulkan_canvas::paint(const std::span<const uint8_t> image)
    {
        if (image.size() != this->get_buffer_size())
        {
            return;
        }
    }

    void vulkan_canvas::draw() const
    {
        /* VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(get_width());
        viewport.height = static_cast<float>(get_height());
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent.width = get_width();
        scissor.extent.height = get_height();
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // Draw the texture as a full-screen quad
        vkCmdDraw(commandBuffer, 6, 1, 0, 0);*/
    }
}
