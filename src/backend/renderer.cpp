#include "renderer.hpp"

#include <memory>

struct pixel
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

void renderer::on_frame(canvas& canvas)
{
    static int i = 0;
    ++i;

    const auto dimensions = canvas.get_dimensions();
    std::unique_ptr<pixel[]> buffer(new pixel[dimensions.height * dimensions.width]);
    for (auto y = 0u; y < dimensions.height; ++y)
    {
        for (auto x = 0u; x < dimensions.width; ++x)
        {
            auto& p = buffer[y * dimensions.width + x];
            p.a = static_cast<uint8_t>(i + y * dimensions.width + x + 0);
            p.r = static_cast<uint8_t>(i + y * dimensions.width + x + 1);
            p.g = static_cast<uint8_t>(i + y * dimensions.width + x + 2);
            p.b = static_cast<uint8_t>(i + y * dimensions.width + x + 3);
        }
    }

    canvas.paint(buffer.get());
}
