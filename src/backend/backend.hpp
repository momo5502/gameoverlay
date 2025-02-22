#pragma once

#include <utils/win.hpp>

namespace gameoverlay
{
    class backend
    {
      public:
        virtual ~backend() = default;

        virtual void initialize()
        {
        }

        virtual void on_window_destruction(const HWND window)
        {
            (void)window;
        }
    };
}
