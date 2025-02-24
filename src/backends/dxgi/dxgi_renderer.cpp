#include "dxgi_renderer.hpp"

#include <stdexcept>

namespace gameoverlay::dxgi
{
    namespace
    {
        HWND get_swap_chain_window(IDXGISwapChain& swap_chain)
        {
            DXGI_SWAP_CHAIN_DESC desc{};
            if (FAILED(swap_chain.GetDesc(&desc)))
            {
                return {};
            }

            return desc.OutputWindow;
        }
    }

    dxgi_renderer::dxgi_renderer(owned_handler h, IDXGISwapChain& swap_chain)
        : renderer(std::move(h)),
          window_(get_swap_chain_window(swap_chain)),
          swap_chain_(&swap_chain)
    {
        CComPtr<ID3D10Device> device10{};
        swap_chain.GetDevice(__uuidof(ID3D10Device), reinterpret_cast<void**>(&device10));

        if (device10)
        {
            this->type_ = backend_type::d3d10;
            return;
        }

        CComPtr<ID3D11Device> device11{};
        swap_chain.GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&device11));

        if (device11)
        {
            this->type_ = backend_type::d3d11;
        }
    }

    HWND dxgi_renderer::get_window() const
    {
        return this->window_;
    }

    backend_type dxgi_renderer::get_backend_type() const
    {
        return this->type_;
    }

    void dxgi_renderer::draw_frame()
    {
        /* const auto current_dim = get_device_dimensions(*this->device_);
         if (!this->canvas_ || this->canvas_->get_dimensions() != current_dim)
         {
             this->canvas_ = std::make_unique<d3d9_canvas>(this->device_, current_dim.width, current_dim.height);
         }

         this->handle_new_frame(*this->canvas_);
         this->canvas_->draw();*/
    }
}
