#pragma once

#include "gl_texture_scope.hpp"

struct gl_vao_scope
{
    GLuint vao{};

    gl_vao_scope()
    {
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, detail::gl_ref(this->vao));
        glBindVertexArray(0);
    }

    ~gl_vao_scope()
    {
        glBindVertexArray(this->vao);
    }

    CLASS_DISABLE_COPY_AND_MOVE(gl_vao_scope);
};
