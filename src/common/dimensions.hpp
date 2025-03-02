#pragma once

#include <cstdint>

struct dimensions
{
    uint32_t width{0};
    uint32_t height{0};

    dimensions() = default;

    dimensions(const uint32_t w, const uint32_t h)
        : width(w),
          height(h)
    {
    }

    dimensions(const int32_t w, const int32_t h)
        : dimensions(normalize(w), normalize(h))
    {
    }

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

    bool is_non_zero() const
    {
        return this->width != 0 && this->height != 0;
    }

    static uint32_t normalize(const int32_t value)
    {
        return static_cast<uint32_t>(std::max(0, value));
    }

    size_t get_area() const
    {
        return static_cast<size_t>(1) * this->width * this->height;
    }
};
