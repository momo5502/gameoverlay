#pragma once
#include "canvas.hpp"
#include "backend_type.hpp"

#include <memory>
#include <utils/win.hpp>

namespace gameoverlay
{
    class renderer
    {
      public:
        struct handler
        {
            CLASS_DECLARE_INTERFACE(handler);

            virtual void on_frame(const renderer& r, canvas& c)
            {
                (void)r;
                (void)c;
            }
        };

        using owned_handler = std::unique_ptr<handler>;
        renderer(owned_handler h)
            : handler_(std::move(h))
        {
        }

        CLASS_DISABLE_COPY_AND_MOVE(renderer);

        virtual ~renderer() = default;

        virtual backend_type get_backend_type() const = 0;
        virtual HWND get_window() const = 0;

      protected:
        void handle_new_frame(canvas& c) const
        {
            this->handler_->on_frame(*this, c);
        }

      private:
        owned_handler handler_{};
    };

    template <backend_type Type>
    class typed_renderer : public renderer
    {
      public:
        using renderer::renderer;

        backend_type get_backend_type() const override
        {
            return Type;
        }
    };

    template <backend_type Type>
    class window_renderer : public typed_renderer<Type>
    {
      public:
        window_renderer(renderer::owned_handler h, const HWND window)
            : typed_renderer<Type>(std::move(h)),
              window_(window)
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
