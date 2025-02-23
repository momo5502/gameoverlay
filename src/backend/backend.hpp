#pragma once

#include <utils/win.hpp>
#include <utils/class_helper.hpp>

namespace gameoverlay
{
    struct backend
    {
        enum class type
        {
            d3d8,
            d3d9,
            d3d10,
            d3d11,
            d3d12,
            opengl,
            vulkan,
        };

        CLASS_DECLARE_INTERFACE(backend);

        virtual type get_type() const = 0;

        virtual void on_window_destruction(const HWND window)
        {
            (void)window;
        }
    };

    template <backend::type Type>
    struct typed_backed : backend
    {
        type get_type() const override
        {
            return Type;
        }
    };
}
