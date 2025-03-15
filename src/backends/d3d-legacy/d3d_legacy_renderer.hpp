#pragma once

#include "d3d_legacy_canvas.hpp"

#include <memory>
#include <renderer.hpp>

namespace gameoverlay::d3d_legacy
{
    namespace detail
    {
        template <typename traits>
        dimensions get_device_dimensions(typename traits::device& device)
        {
            typename traits::viewport vp{};
            device.GetViewport(&vp);

            dimensions dimensions{};
            dimensions.width = static_cast<uint32_t>(vp.Width);
            dimensions.height = static_cast<uint32_t>(vp.Height);

            return dimensions;
        }

        template <typename Device>
        HWND get_device_window(Device& device)
        {
            D3DDEVICE_CREATION_PARAMETERS params{};
            device.GetCreationParameters(&params);
            return params.hFocusWindow;
        }
    }

    template <renderer_type Type, typename d3d_legacy_traits>
    class d3d_legacy_renderer : public window_renderer<Type>
    {
      public:
        using traits = d3d_legacy_traits;

        d3d_legacy_renderer(renderer::owned_handler h, typename traits::device& dev)
            : window_renderer<Type>(std::move(h), detail::get_device_window(dev)),
              device_(&dev)
        {
        }

        void draw_frame()
        {
            const auto current_dim = detail::get_device_dimensions<traits>(*this->device_);
            if (!this->canvas_ || this->canvas_->get_dimensions() != current_dim)
            {
                this->canvas_ =
                    std::make_unique<d3d_legacy_canvas<traits>>(*this->device_, current_dim.width, current_dim.height);
            }

            this->handle_new_frame(*this->canvas_);
            this->canvas_->draw();
        }

        void reset()
        {
            this->canvas_ = {};
        }

      private:
        CComPtr<typename traits::device> device_{};
        std::unique_ptr<d3d_legacy_canvas<traits>> canvas_{};
    };
}
