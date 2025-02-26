#include "dxgi_renderer.hpp"

#include "d3d10_canvas.hpp"
#include "d3d11_canvas.hpp"

namespace gameoverlay::dxgi
{
    namespace
    {
        template <typename Device>
        CComPtr<Device> get_device(IDXGISwapChain& swap_chain)
        {
            CComPtr<Device> device{};
            if (FAILED(swap_chain.GetDevice(__uuidof(Device), reinterpret_cast<void**>(&device))))
            {
                return {};
            }

            return device;
        }

        HWND get_swap_chain_window(IDXGISwapChain& swap_chain)
        {
            DXGI_SWAP_CHAIN_DESC desc{};
            if (FAILED(swap_chain.GetDesc(&desc)))
            {
                return {};
            }

            return desc.OutputWindow;
        }

        template <typename Texture, typename Description>
        dimensions get_swap_chain_dimensions(IDXGISwapChain& swap_chain)
        {
            CComPtr<Texture> back_buffer{};
            HRESULT hr = swap_chain.GetBuffer(0, __uuidof(Texture), reinterpret_cast<void**>(&back_buffer));
            if (FAILED(hr) || !back_buffer)
            {
                return {};
            }

            Description desc{};
            back_buffer->GetDesc(&desc);

            return {
                desc.Width,
                desc.Height,
            };
        }

        dimensions get_d3d10_dimensions(IDXGISwapChain& swap_chain)
        {
            const auto swap_chain_dim = get_swap_chain_dimensions<ID3D10Texture2D, D3D10_TEXTURE2D_DESC>(swap_chain);
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
            const auto swap_chain_dim = get_swap_chain_dimensions<ID3D11Texture2D, D3D11_TEXTURE2D_DESC>(swap_chain);
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

        dimensions get_swap_chain_dimensions(IDXGISwapChain& swap_chain, const backend_type type)
        {
            switch (type)
            {
            case backend_type::d3d10:
                return get_d3d10_dimensions(swap_chain);
            case backend_type::d3d11:
                return get_d3d11_dimensions(swap_chain);
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
                return std::make_unique<d3d10_canvas>(swap_chain, dim);
            case backend_type::d3d11:
                return std::make_unique<d3d11_canvas>(swap_chain, dim);
            default: {
                static const auto x = [] {
                    OutputDebugStringA("Failed to create dxgi canvas");
                    return 0;
                }();
                (void)x;

                return {};
            }
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
        const auto dim = get_swap_chain_dimensions(*this->swap_chain_, this->type_);

        if (!this->canvas_)
        {
            this->canvas_ = create_canvas(*this->swap_chain_, this->type_, dim);
        }

        if (!this->canvas_)
        {
            return;
        }

        if (this->canvas_->get_dimensions() != dim)
        {
            this->canvas_->resize(dim);
        }

        this->handle_new_frame(*this->canvas_);
        this->canvas_->draw();
    }
}
