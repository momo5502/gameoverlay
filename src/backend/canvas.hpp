#pragma once
#include <cstdint>

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
  public:
    canvas() = default;

    canvas(canvas&&) = delete;
    canvas(const canvas&) = delete;
    canvas& operator=(canvas&&) = delete;
    canvas& operator=(const canvas&) = delete;

    virtual ~canvas() = default;
    virtual dimensions get_dimensions() const = 0;
    virtual void paint(const void* image) = 0;

    uint32_t get_width() const;
    uint32_t get_height() const;
};

class fixed_canvas : public canvas
{
  public:
    fixed_canvas(uint32_t width, uint32_t height);

    dimensions get_dimensions() const override;

  private:
    uint32_t width_{};
    uint32_t height_{};
};
