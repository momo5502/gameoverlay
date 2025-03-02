#include "dxgi_renderer.hpp"
#include "dxgi_utils.hpp"

#include "d3dx_canvas.hpp"
#include "d3d12_canvas.hpp"

namespace gameoverlay::dxgi
{
    namespace
    {
        dimensions get_d3d10_dimensions(IDXGISwapChain& swap_chain)
        {
            const auto swap_chain_dim = get_swap_chain_dimensions<ID3D10Texture2D>(swap_chain);
            if (!swap_chain_dim.is_zero())
            {
                return swap_chain_dim;
            }

            const auto device = get_device<ID3D10Device>(swap_chain);
            if (!device)
            {
                return {};
            }

            UINT num_viewports = 1;
            D3D10_VIEWPORT viewport{};

            device->RSGetViewports(&num_viewports, &viewport);

            return {
                viewport.Width,
                viewport.Height,
            };
        }

        dimensions get_d3d11_dimensions(IDXGISwapChain& swap_chain)
        {
            const auto swap_chain_dim = get_swap_chain_dimensions<ID3D11Texture2D>(swap_chain);
            if (!swap_chain_dim.is_zero())
            {
                return swap_chain_dim;
            }

            const auto device = get_device<ID3D11Device>(swap_chain);
            if (!device)
            {
                return {};
            }

            CComPtr<ID3D11DeviceContext> context{};
            device->GetImmediateContext(&context);

            UINT num_viewports = 1;
            D3D11_VIEWPORT viewport{};

            context->RSGetViewports(&num_viewports, &viewport);

            return {
                static_cast<uint32_t>(viewport.Width),
                static_cast<uint32_t>(viewport.Height),
            };
        }

        dimensions get_d3d12_dimensions(IDXGISwapChain& swap_chain)
        {
            const auto back_buffer = get_back_buffer<ID3D12Resource>(swap_chain);
            if (!back_buffer)
            {
                return {};
            }

            const auto desc = back_buffer->GetDesc();

            return {
                static_cast<uint32_t>(desc.Width),
                static_cast<uint32_t>(desc.Height),
            };
        }

        dimensions get_buffer_dimensions(IDXGISwapChain& swap_chain, const backend_type type)
        {
            switch (type)
            {
            case backend_type::d3d10:
                return get_d3d10_dimensions(swap_chain);
            case backend_type::d3d11:
                return get_d3d11_dimensions(swap_chain);
            case backend_type::d3d12:
                return get_d3d12_dimensions(swap_chain);
            default:
                return {};
            }
        }

        std::unique_ptr<dxgi_canvas> create_canvas(IDXGISwapChain& swap_chain, const backend_type type,
                                                   const dimensions dim)
        {
            switch (type)
            {
            case backend_type::d3d10:
                return std::make_unique<d3dx_canvas<d3d10_traits>>(swap_chain, dim);
            case backend_type::d3d11:
                return std::make_unique<d3dx_canvas<d3d11_traits>>(swap_chain, dim);
            case backend_type::d3d12: {
                return std::make_unique<d3d12_canvas>(*query_interface<IDXGISwapChain3>(swap_chain), dim);
            }

            default:
                throw std::runtime_error("Failed to create dxgi canvas");
            }
        }
    }

    dxgi_renderer::dxgi_renderer(owned_handler h, IDXGISwapChain& swap_chain)
        : renderer(std::move(h)),
          window_(get_swap_chain_window(swap_chain)),
          swap_chain_(&swap_chain)
    {
        const auto device10 = get_device<ID3D10Device>(swap_chain);
        if (device10)
        {
            this->type_ = backend_type::d3d10;
            return;
        }

        const auto device11 = get_device<ID3D11Device>(swap_chain);
        if (device11)
        {
            this->type_ = backend_type::d3d11;
            return;
        }

        const auto device12 = get_device<ID3D12Device>(swap_chain);
        if (device12)
        {
            this->type_ = backend_type::d3d12;
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
        const auto dim = get_buffer_dimensions(*this->swap_chain_, this->type_);

        if (!this->canvas_)
        {
            if (this->canvas_failed_)
            {
                return;
            }

            try
            {
                this->canvas_ = create_canvas(*this->swap_chain_, this->type_, dim);
                this->canvas_failed_ = false;
            }
            catch (const std::exception& e)
            {
                this->canvas_failed_ = true;
                OutputDebugStringA(e.what());
            }
        }

        if (!this->canvas_)
        {
            return;
        }

        this->canvas_->resize(dim);
        this->handle_new_frame(*this->canvas_);
        this->canvas_->draw();
    }

    void dxgi_renderer::before_resize() const
    {
        if (this->canvas_)
        {
            this->canvas_->before_resize();
        }
    }

    void dxgi_renderer::after_frame() const
    {
        if (this->canvas_)
        {
            this->canvas_->after_draw();
        }
    }
}
