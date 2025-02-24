#pragma once

// #include "d3d9_canvas.hpp"
#include "dxgi_win.hpp"

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

      private:
        HWND window_{};
        backend_type type_{backend_type::dxgi};
        IDXGISwapChain* swap_chain_{};
        std::unique_ptr<canvas> canvas_{};
    };
}
