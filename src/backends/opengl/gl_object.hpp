#pragma once

#include <optional>
#include <functional>

#include "gl_include.hpp"

class gl_object
{
  public:
    gl_object() = default;
    gl_object(GLuint object, std::function<void(GLuint)> destructor);

    ~gl_object();

    gl_object(const gl_object&) = delete;
    gl_object& operator=(const gl_object&) = delete;

    gl_object(gl_object&& obj) noexcept;
    gl_object& operator=(gl_object&& obj) noexcept;

    GLuint get() const;
    operator GLuint() const;

    bool is_valid() const
    {
        return this->object_.has_value();
    }

  private:
    std::function<void(GLuint)> destructor_{};
    std::optional<GLuint> object_{};

    void destroy();
};
