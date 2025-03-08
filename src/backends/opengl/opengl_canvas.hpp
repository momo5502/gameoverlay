#pragma once

#include <canvas.hpp>
#include "gl_object.hpp"

namespace gameoverlay::opengl
{
    class opengl_canvas : public fixed_canvas
    {
      public:
        opengl_canvas(uint32_t width, uint32_t height);

        void paint(std::span<const uint8_t> image) override;

        void draw() const;

      private:
        bool is_core_{};
        gl_object texture_{};
        gl_object program_{};
        gl_object index_buffer_{};
        gl_object vertex_buffer_{};
        gl_object vertex_array_object_{};
    };
}
