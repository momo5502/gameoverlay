#include "opengl_canvas.hpp"

#include <memory>
#include <vector>

#include "gl_scope.hpp"
#include "gl_shader.hpp"
#include "gl_vao_scope.hpp"

#define GLSL(version, shader) "#version " #version "\n" #shader

namespace gameoverlay::opengl
{
    namespace
    {
        gl_object get_modern_shader_program()
        {
            return create_program(GLSL(
                                      330,                                   //
                                      layout(location = 0) in vec2 position; //
                                      layout(location = 1) in vec2 texCoord; //
                                      out vec2 TexCoord;

                                      void main() {
                                          TexCoord = texCoord;
                                          gl_Position = vec4(position, 0.0, 1.0);
                                      }),
                                  GLSL(
                                      330,                //
                                      in vec2 TexCoord;   //
                                      out vec4 FragColor; //
                                      uniform sampler2D tex_sampler;

                                      void main() {
                                          FragColor = texture(tex_sampler, TexCoord); //
                                      }));
        }

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

        bool is_core_profile()
        {
            GLint profileMask = 0;
            glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profileMask);
            return profileMask & GL_CONTEXT_CORE_PROFILE_BIT;
        }

        void delete_vertex_array(const GLuint vao)
        {
            glDeleteVertexArrays(1, &vao);
        }

        gl_object create_vertex_array()
        {
            GLuint vao{};
            glGenVertexArrays(1, &vao);

            return {vao, delete_vertex_array};
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

        void delete_buffer(const GLuint buffer)
        {
            glDeleteBuffers(1, &buffer);
        }

        gl_object create_buffer()
        {
            GLuint buffer{};
            glGenBuffers(1, &buffer);
            return {buffer, delete_buffer};
        }

        gl_object create_texture(const uint32_t width, const uint32_t height)
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            auto texture = create_texture();
            glBindTexture(GL_TEXTURE_2D, texture);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            const auto bytes = width * height * 4;
            const std::vector<uint8_t> buffer(bytes, 0xFF);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());

            return texture;
        }

        gl_object create_vertex_buffer()
        {
            auto buffer = create_buffer();

            constexpr float vertices[] = {
                -1.0f, -1.0f, 0.0f, 1.0f, //
                1.0f,  -1.0f, 1.0f, 1.0f, //
                1.0f,  1.0f,  1.0f, 0.0f, //
                -1.0f, 1.0f,  0.0f, 0.0f, //
            };

            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            return buffer;
        }

        gl_object create_index_buffer()
        {
            auto buffer = create_buffer();

            constexpr unsigned int indices[] = {
                0, 1, 2, 2, 3, 0,
            };

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

            return buffer;
        }

        gl_object create_vao(const GLuint vertex_buffer, const GLuint index_buffer)
        {
            auto vao = create_vertex_array();

            glBindVertexArray(vao);

            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), static_cast<void*>(nullptr));
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                                  reinterpret_cast<void*>(2 * sizeof(float)));
            glEnableVertexAttribArray(1);

            return vao;
        }
    }

    opengl_canvas::opengl_canvas(const uint32_t width, const uint32_t height)
        : fixed_canvas(width, height),
          is_core_(is_core_profile())
    {
        gl_scope _{};

        this->texture_ = create_texture(width, height);

        if (this->is_core_)
        {
            gl_vao_scope _1{};

            this->program_ = get_modern_shader_program();
            this->index_buffer_ = create_index_buffer();
            this->vertex_buffer_ = create_vertex_buffer();
            this->vertex_array_object_ = create_vao(this->vertex_buffer_, this->index_buffer_);
        }
        else
        {
            this->program_ = get_legacy_shader_program();
        }
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

        if (this->is_core_)
        {
            gl_vao_scope _1{};

            glBindVertexArray(this->vertex_array_object_);
            glBindTexture(GL_TEXTURE_2D, this->texture_);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }
        else
        {
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
}
