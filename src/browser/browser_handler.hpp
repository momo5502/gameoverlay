#pragma once

#include <span>
#include <dimensions.hpp>

struct browser_handler
{
    virtual ~browser_handler() = default;

    virtual dimensions get_dimensions() = 0;
    virtual void paint(std::span<const uint8_t> buffer, dimensions dim) = 0;
};
