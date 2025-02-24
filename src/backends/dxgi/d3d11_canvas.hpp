#pragma once

#include "dxgi_win.hpp"
#include "dxgi_canvas.hpp"

namespace gameoverlay::dxgi
{
    class d3d11_canvas : public dxgi_canvas
    {
      public:
        d3d11_canvas(ID3D11Device& device);
        d3d11_canvas(ID3D11Device& device, dimensions dim);

        void paint(std::span<const uint8_t> image) override;

        void draw() const override;

      private:
        CComPtr<ID3D11Device> device_{};
        CComPtr<ID3D11DeviceContext> context_{};

        CComPtr<ID3DX11Effect> effect_{};
        ID3DX11EffectTechnique* effect_technique_{};
        ID3DX11EffectShaderResourceVariable* effect_shader_resource_variable_{};

        CComPtr<ID3D11InputLayout> input_layout_{};
        CComPtr<ID3D11Buffer> index_buffer_{};
        CComPtr<ID3D11Buffer> vertex_buffer_{};
        CComPtr<ID3D11BlendState> blend_state_{};
        CComPtr<ID3D11DepthStencilState> depth_stencil_state_{};

        CComPtr<ID3D11Texture2D> texture_{};
        CComPtr<ID3D11ShaderResourceView> shader_resource_view_{};

        void resize_texture(dimensions new_dimensions) override;
    };
}
