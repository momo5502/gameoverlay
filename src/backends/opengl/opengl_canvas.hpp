#pragma once
#include <GL/glew.h>

#include <canvas.hpp>

namespace gameoverlay::opengl
{
    class opengl_canvas : public fixed_canvas
    {
      public:
        opengl_canvas(uint32_t width, uint32_t height);
        ~opengl_canvas() override;

        void paint(std::span<const uint8_t> image) override;

        void draw() const;

      private:
        GLuint texture_{};
        GLuint program_{};
    };
}
