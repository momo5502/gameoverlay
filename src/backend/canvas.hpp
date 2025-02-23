#pragma once

#include <span>
#include <cstddef>
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
  protected:
    CLASS_DECLARE_INTERFACE(canvas);

  public:
    virtual dimensions get_dimensions() const = 0;
    virtual void paint(std::span<const uint8_t> image) = 0;

    uint32_t get_width() const
    {
        return this->get_dimensions().width;
    }

    uint32_t get_height() const
    {
        return this->get_dimensions().height;
    }

    size_t get_buffer_size() const
    {
        const auto dim = this->get_dimensions();
        return static_cast<size_t>(4) * dim.width * dim.height;
    }
};

struct fixed_canvas : canvas
{
  protected:
    fixed_canvas(const dimensions dim)
        : dimensions_(dim)
    {
    }

    fixed_canvas(const uint32_t width, const uint32_t height)
        : fixed_canvas({
              .width = width,
              .height = height,
          })
    {
    }

  public:
    dimensions get_dimensions() const override
    {
        return this->dimensions_;
    }

  private:
    dimensions dimensions_{};
};
