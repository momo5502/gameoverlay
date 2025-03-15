#include "vulkan_canvas.hpp"

#include <memory>
#include <vector>

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
    }
}
