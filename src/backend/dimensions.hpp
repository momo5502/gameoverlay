#pragma once

#include <cstdint>

struct dimensions
{
    uint32_t width{0};
    uint32_t height{0};

    bool operator==(const dimensions& obj) const
    {
        return this->width == obj.width && this->height == obj.height;
    }

    bool operator!=(const dimensions& obj) const
    {
        return !this->operator==(obj);
    }

    bool is_zero() const
    {
        return this->width == 0 && this->height == 0;
    }
};
