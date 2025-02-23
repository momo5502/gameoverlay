#include "d3d9_renderer.hpp"

#include <stdexcept>

namespace gameoverlay::d3d9
{
    namespace
    {
        dimensions get_device_dimensions(IDirect3DDevice9& device)
        {
            D3DVIEWPORT9 viewport;
            device.GetViewport(&viewport);

            dimensions dimensions{};
            dimensions.width = static_cast<uint32_t>(viewport.Width);
            dimensions.height = static_cast<uint32_t>(viewport.Height);

            return dimensions;
        }

        HWND get_device_window(IDirect3DDevice9& device)
        {
            D3DDEVICE_CREATION_PARAMETERS params{};
            device.GetCreationParameters(&params);
            return params.hFocusWindow;
        }
    }

    d3d9_renderer::d3d9_renderer(owned_handler h, IDirect3DDevice9& device)
        : window_renderer(std::move(h), get_device_window(device)),
          device_(&device)
    {
    }

    void d3d9_renderer::draw_frame()
    {
        const auto current_dim = get_device_dimensions(*this->device_);
        if (!this->canvas_ || this->canvas_->get_dimensions() != current_dim)
        {
            this->canvas_ = std::make_unique<d3d9_canvas>(this->device_, current_dim.width, current_dim.height);
        }

        this->handle_new_frame(*this->canvas_);
        this->canvas_->draw();
    }

    void d3d9_renderer::reset()
    {
        this->canvas_ = {};
    }
}
