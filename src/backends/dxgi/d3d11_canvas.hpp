#pragma once

#include "dxgi_win.hpp"
#include "dxgi_canvas.hpp"

namespace gameoverlay::dxgi
{
    class d3d11_canvas : public dxgi_canvas
    {
      public:
        d3d11_canvas(ID3D10Device& device);
        d3d11_canvas(ID3D10Device& device, dimensions dim);

        void paint(std::span<const uint8_t> image) override;

        void draw() const override;

      private:
        CComPtr<ID3D10Device> device_{};

        CComPtr<ID3D10Effect> effect_{};
        ID3D10EffectTechnique* effect_technique_{};
        ID3D10EffectShaderResourceVariable* effect_shader_resource_variable_{};

        CComPtr<ID3D10InputLayout> input_layout_{};
        CComPtr<ID3D10Buffer> index_buffer_{};
        CComPtr<ID3D10Buffer> vertex_buffer_{};
        CComPtr<ID3D10BlendState> blend_state_{};
        CComPtr<ID3D10DepthStencilState> depth_stencil_state_{};

        CComPtr<ID3D10Texture2D> texture_{};
        CComPtr<ID3D10ShaderResourceView> shader_resource_view_{};

        void resize_texture(dimensions new_dimensions) override;
    };
}
