#pragma once
#include <GL/glew.h>

#include <canvas.hpp>

namespace gameoverlay::opengl
{
    class canvas : public fixed_canvas
    {
      public:
        canvas(uint32_t width, uint32_t height);
        ~canvas() override;

        void paint(const void* image) override;

        void draw() const;

      private:
        GLuint texture_{};
        GLuint program_{};
    };
}
