#pragma once
#include <string>

#include "win.hpp"
#include "class_helper.hpp"

namespace utils
{
    class dummy_window
    {
      public:
        dummy_window();
        ~dummy_window();

        HWND get() const
        {
            return this->window_;
        }

        CLASS_DISABLE_COPY_AND_MOVE(dummy_window);

      private:
        std::string window_class_{};
        HWND window_{};
    };
}
