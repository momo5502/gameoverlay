#pragma once

#include <canvas.hpp>

namespace gameoverlay::vulkan
{
    class vulkan_canvas : public fixed_canvas
    {
      public:
        vulkan_canvas(uint32_t width, uint32_t height);

        void paint(std::span<const uint8_t> image) override;

        void draw() const;
    };
}
