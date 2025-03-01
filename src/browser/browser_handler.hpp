#pragma once

#include <cstdint>

struct browser_handler
{
    virtual uint32_t get_width() = 0;
    virtual uint32_t get_height() = 0;
    virtual void paint(const void* buffer, uint32_t width, uint32_t height) = 0;
};
