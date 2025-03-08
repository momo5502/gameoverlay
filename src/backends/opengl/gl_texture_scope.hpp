#pragma once

#include "gl_include.hpp"

#include <utils/class_helper.hpp>

namespace detail
{
    inline GLint* gl_ref(GLuint& ref)
    {
        return reinterpret_cast<GLint*>(&ref);
    }
}

struct gl_texture_scope
{
    GLuint texture{0};
    GLint alignment{0};

    gl_texture_scope()
    {
        glGetIntegerv(GL_TEXTURE_BINDING_2D, detail::gl_ref(this->texture));
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &this->alignment);
    }

    ~gl_texture_scope()
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, this->alignment);
        glBindTexture(GL_TEXTURE_2D, this->texture);
    }

    CLASS_DISABLE_COPY_AND_MOVE(gl_texture_scope);
};
