#include "vulkan_renderer.hpp"

#include <stdexcept>

namespace gameoverlay::vulkan
{
    namespace
    {
        dimensions get_dimensions(const HDC hdc)
        {
            const HWND window = WindowFromDC(hdc);

            RECT rect;
            GetClientRect(window, &rect);

            dimensions dim{};
            dim.width = std::abs(rect.left - rect.right);
            dim.height = std::abs(rect.bottom - rect.top);
            return dim;
        }
    }

    vulkan_renderer::vulkan_renderer(owned_handler h, const HDC hdc)
        : window_renderer(std::move(h), WindowFromDC(hdc)),
          hdc_(hdc)
    {
    }

    void vulkan_renderer::draw_frame()
    {
        const auto current_dim = get_dimensions(this->hdc_);
        if (!this->canvas_ || this->canvas_->get_dimensions() != current_dim)
        {
            this->canvas_ = std::make_unique<vulkan_canvas>(current_dim.width, current_dim.height);
        }

        this->handle_new_frame(*this->canvas_);
        this->canvas_->draw();
    }
}
