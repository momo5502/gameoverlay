#include "gl_object.hpp"

gl_object::gl_object(const GLuint object, std::function<void(GLuint)> destructor)
    : destructor_(std::move(destructor)),
      object_(object)
{
}

gl_object::~gl_object()
{
    this->destroy();
}

gl_object::gl_object(gl_object&& obj) noexcept
{
    this->operator=(std::move(obj));
}

gl_object& gl_object::operator=(gl_object&& obj) noexcept
{
    if (this != &obj)
    {
        this->destroy();

        this->object_ = std::move(obj.object_);
        this->destructor_ = std::move(obj.destructor_);

        obj.destructor_ = {};
        obj.object_ = {};
    }

    return *this;
}

GLuint gl_object::get() const
{
    return *this->object_;
}

gl_object::operator unsigned() const
{
    return this->get();
}

void gl_object::destroy()
{
    if (this->object_)
    {
        if (!this->destructor_)
        {
            std::abort();
        }

        this->destructor_(*this->object_);

        this->destructor_ = {};
        this->object_ = {};
    }
}
