#pragma once
#include "canvas.hpp"
#include "backend.hpp"

#include <functional>

namespace gameoverlay
{
    using frame_handler = std::function<void(canvas& canvas)>;

    class renderer
    {
      public:
        CLASS_DECLARE_INTERFACE(renderer);

        virtual backend::type get_backend_type() const = 0;
        virtual HWND get_window() const = 0;

        void on_frame(frame_handler handler)
        {
            this->handler_ = std::move(handler);
        }

      protected:
        void handle_new_frame(canvas& c) const
        {
            if (this->handler_)
            {
                this->handler_(c);
            }
        }

      private:
        frame_handler handler_{};
    };

    template <backend::type Type>
    class typed_renderer : public renderer
    {
      public:
        backend::type get_backend_type() const override
        {
            return Type;
        }
    };

    template <backend::type Type>
    class window_renderer : public typed_renderer<Type>
    {
      public:
        window_renderer(const HWND window)
            : window_(window)
        {
        }

        HWND get_window() const override
        {
            return this->window_;
        }

      private:
        HWND window_{};
    };
}
