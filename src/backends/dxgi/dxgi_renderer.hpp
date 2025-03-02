#pragma once

#include "dxgi_win.hpp"
#include "dxgi_canvas.hpp"

#include <memory>
#include <renderer.hpp>

namespace gameoverlay::dxgi
{
    class dxgi_renderer : public renderer
    {
      public:
        dxgi_renderer(owned_handler h, IDXGISwapChain& swap_chain);
        void draw_frame();

        HWND get_window() const override;
        backend_type get_backend_type() const override;

        void before_resize() const;
        void after_frame() const;

      private:
        bool canvas_failed_{false};
        HWND window_{};
        backend_type type_{backend_type::dxgi};

        CComPtr<IDXGISwapChain> swap_chain_{};
        std::unique_ptr<dxgi_canvas> canvas_{};
    };
}
