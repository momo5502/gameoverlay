#pragma once

#include <canvas.hpp>
#include "dxgi_win.hpp"

namespace gameoverlay::dxgi
{
    class d3d10_canvas : public canvas
    {
      public:
        d3d10_canvas(ID3D10Device& device);
        d3d10_canvas(ID3D10Device& device, dimensions dim);
        ~d3d10_canvas() override = default;

        dimensions get_dimensions() const override;
        void paint(std::span<const uint8_t> image) override;

        void draw() const;

        void resize(dimensions dim);

      private:
        dimensions dimensions_{};

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
    };
}
