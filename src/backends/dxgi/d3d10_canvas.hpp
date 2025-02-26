#pragma once

#include "dxgi_win.hpp"
#include "dxgi_canvas.hpp"

namespace gameoverlay::dxgi
{
    class d3d10_canvas : public dxgi_canvas
    {
      public:
        d3d10_canvas(IDXGISwapChain& swap_chain);
        d3d10_canvas(IDXGISwapChain& swap_chain, dimensions dim);

        void paint(std::span<const uint8_t> image) override;

        void draw() const override;

      private:
        CComPtr<IDXGISwapChain> swap_chain_{};

        CComPtr<ID3D10Device> device_{};

        CComPtr<ID3D10VertexShader> vertex_shader_{};
        CComPtr<ID3D10PixelShader> pixel_shader_{};
        CComPtr<ID3D10InputLayout> input_layout_{};

        CComPtr<ID3D10Buffer> index_buffer_{};
        CComPtr<ID3D10Buffer> vertex_buffer_{};
        CComPtr<ID3D10BlendState> blend_state_{};
        CComPtr<ID3D10RasterizerState> rasterizer_state_{};
        CComPtr<ID3D10DepthStencilState> depth_stencil_state_{};

        CComPtr<ID3D10Texture2D> texture_{};
        CComPtr<ID3D10ShaderResourceView> shader_resource_view_{};

        void resize_texture(dimensions new_dimensions) override;
    };
}
