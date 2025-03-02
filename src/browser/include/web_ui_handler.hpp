#pragma once

#include <span>
#include <dimensions.hpp>

namespace gameoverlay
{
    struct web_ui_handler
    {
        virtual ~web_ui_handler() = default;

        virtual dimensions get_dimensions() = 0;
        virtual void paint(std::span<const uint8_t> buffer, dimensions dim) = 0;
    };
}
