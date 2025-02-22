#pragma once

#include <cstdint>
#include <utils/class_helper.hpp>

struct dimensions
{
    uint32_t width{};
    uint32_t height{};

    bool operator==(const dimensions& obj) const
    {
        return this->width == obj.width && this->height == obj.height;
    }

    bool operator!=(const dimensions& obj) const
    {
        return !this->operator==(obj);
    }
};

class canvas
{
    CLASS_DECLARE_INTERFACE(canvas);

  public:
    virtual dimensions get_dimensions() const = 0;
    virtual void paint(const void* image) = 0;

    uint32_t get_width() const
    {
        return this->get_dimensions().width;
    }

    uint32_t get_height() const
    {
        return this->get_dimensions().height;
    }
};
