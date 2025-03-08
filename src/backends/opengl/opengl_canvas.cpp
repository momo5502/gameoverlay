#include "opengl_canvas.hpp"

#include <memory>
#include <vector>

#include "gl_scope.hpp"
#include "gl_shader.hpp"

#include <utils/win.hpp>
#include <utils/finally.hpp>

#define GLSL(version, shader) "#version " #version "\n" #shader

namespace gameoverlay::opengl
{
    namespace
    {
        gl_object get_legacy_shader_program()
        {
            return create_program(GLSL(
                                      120, //
                                      void main() {
                                          gl_TexCoord[0] = gl_MultiTexCoord0;
                                          gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
                                      }),
                                  GLSL(
                                      120, //
                                      uniform sampler2D tex_sampler;

                                      void main() {
                                          gl_FragColor = texture2D(tex_sampler, gl_TexCoord[0].st); //
                                      }));
        }

        void delete_texture(const GLuint texture)
        {
            glDeleteTextures(1, &texture);
        }

        gl_object create_texture()
        {
            GLuint new_texture{};
            glGenTextures(1, &new_texture);

            return {new_texture, delete_texture};
        }

        gl_object create_texture(const uint32_t width, const uint32_t height)
        {
            gl_texture_scope _{};
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            auto texture = create_texture();
            glBindTexture(GL_TEXTURE_2D, texture);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

            const auto bytes = width * height * 4;
            const std::vector<uint8_t> buffer(bytes, 0xFF);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());

            return texture;
        }
    }

    opengl_canvas::opengl_canvas(const uint32_t width, const uint32_t height)
        : fixed_canvas(width, height),
          texture_(create_texture(width, height)),
          program_(get_legacy_shader_program())
    {
    }

    void opengl_canvas::paint(const std::span<const uint8_t> image)
    {
        if (image.size() != this->get_buffer_size())
        {
            return;
        }

        gl_texture_scope _{};

        glBindTexture(GL_TEXTURE_2D, this->texture_);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, static_cast<GLsizei>(this->get_width()),
                        static_cast<GLsizei>(this->get_height()), GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }

    void opengl_canvas::draw() const
    {
        gl_scope _{};

        glUseProgram(this->program_);

        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1, 1, 1, -1, -1, 2);
        glTranslatef(0.0, 0.0, 0.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glEnable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glCullFace(GL_FRONT_AND_BACK);

        glColor4d(1.0, 1.0, 1.0, 1.0);

        glBindTexture(GL_TEXTURE_2D, this->texture_);
        glActiveTexture(GL_TEXTURE0);

        glBegin(GL_QUADS);
        glTexCoord2i(0, 0);
        glVertex3i(-1, -1, 1);
        glTexCoord2i(0, 1);
        glVertex3i(-1, 1, 1);
        glTexCoord2i(1, 1);
        glVertex3i(1, 1, 1);
        glTexCoord2i(1, 0);
        glVertex3i(1, -1, 1);
        glEnd();
    }
}
