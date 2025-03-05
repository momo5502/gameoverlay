#pragma once

#include <canvas.hpp>

namespace gameoverlay::dxgi
{
    class dxgi_canvas : public canvas
    {
      public:
        dxgi_canvas() = default;
        ~dxgi_canvas() override = default;

        dimensions get_dimensions() const override
        {
            return this->dimensions_;
        }

        virtual void before_resize()
        {
        }

        void resize(const dimensions new_dimensions)
        {
            if (new_dimensions.is_zero() || new_dimensions == this->dimensions_)
            {
                return;
            }

            this->resize_texture(new_dimensions);
            this->dimensions_ = new_dimensions;
        }

        virtual void draw() const = 0;

      private:
        dimensions dimensions_{};

        virtual void resize_texture(dimensions new_dimensions) = 0;
    };
}
