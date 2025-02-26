#pragma once

#include "dxgi_win.hpp"
#include "dxgi_canvas.hpp"

namespace gameoverlay::dxgi
{
    class d3d11_canvas : public dxgi_canvas
    {
      public:
        d3d11_canvas(IDXGISwapChain& swap_chain);
        d3d11_canvas(IDXGISwapChain& swap_chain, dimensions dim);

        void paint(std::span<const uint8_t> image) override;

        void draw() const override;

      private:
        CComPtr<IDXGISwapChain> swap_chain_{};

        CComPtr<ID3D11Device> device_{};

        CComPtr<ID3D11RenderTargetView> render_target_view_{};

        CComPtr<ID3D11VertexShader> vertex_shader_{};
        CComPtr<ID3D11PixelShader> pixel_shader_{};
        CComPtr<ID3D11InputLayout> input_layout_{};

        CComPtr<ID3D11Buffer> index_buffer_{};
        CComPtr<ID3D11Buffer> vertex_buffer_{};
        CComPtr<ID3D11BlendState> blend_state_{};
        CComPtr<ID3D11SamplerState> sampler_state_{};
        CComPtr<ID3D11RasterizerState> rasterizer_state_{};
        CComPtr<ID3D11DepthStencilState> depth_stencil_state_{};

        CComPtr<ID3D11Texture2D> texture_{};
        CComPtr<ID3D11ShaderResourceView> shader_resource_view_{};

        void resize_texture(dimensions new_dimensions) override;

        CComPtr<ID3D11DeviceContext> get_context() const;
    };
}
