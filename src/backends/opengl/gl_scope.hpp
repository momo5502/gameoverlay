#pragma once

#include "gl_texture_scope.hpp"

struct gl_scope : gl_texture_scope
{
    GLuint program{};
    GLuint vao{};

    gl_scope()
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
        glGetIntegerv(GL_CURRENT_PROGRAM, detail::gl_ref(this->program));
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, detail::gl_ref(this->vao));

        glMatrixMode(GL_TEXTURE);
        glPushMatrix();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
    }

    ~gl_scope()
    {
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_TEXTURE);
        glPopMatrix();

        glBindVertexArray(this->vao);
        glUseProgram(this->program);
        glPopClientAttrib();
        glPopAttrib();
    }

    CLASS_DISABLE_COPY_AND_MOVE(gl_scope);
};
