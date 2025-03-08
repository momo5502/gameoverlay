#include "gl_shader.hpp"

#include <string>
#include <stdexcept>

namespace
{
    std::optional<std::string> get_compilation_error(const GLuint shader)
    {
        GLint is_compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);
        if (is_compiled != GL_FALSE)
        {
            return std::nullopt;
        }
        GLint max_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);

        std::string error_log{};
        error_log.resize(max_length);

        glGetShaderInfoLog(shader, max_length, &max_length, error_log.data());

        error_log.resize(max_length);
        return error_log;
    }

    gl_object create_shader(const GLenum type)
    {
        return {glCreateShader(type), glDeleteShader};
    }

    gl_object create_program()
    {
        return {glCreateProgram(), glDeleteProgram};
    }

    gl_object compile_shader(const GLenum type, const std::string_view code)
    {
        auto shader = create_shader(type);

        const auto* code_ptr = code.data();
        const auto length = static_cast<GLint>(code.size());

        glShaderSource(shader, 1, &code_ptr, &length);
        glCompileShader(shader);

        const auto compilation_error = get_compilation_error(shader);
        if (!compilation_error)
        {
            return shader;
        }

        throw std::runtime_error(*compilation_error);
    }
}

gl_object create_program(const std::string_view vertex, const std::string_view fragment)
{
    const auto vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex);
    const auto fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment);

    auto program = create_program();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    return program;
}
