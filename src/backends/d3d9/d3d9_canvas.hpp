#pragma once

#include <canvas.hpp>
#include "d3d9_win.hpp"

namespace gameoverlay::d3d9
{
    class d3d9_canvas : public fixed_canvas
    {
      public:
        d3d9_canvas(IDirect3DDevice9* device, uint32_t width, uint32_t height);
        ~d3d9_canvas() override = default;

        void paint(std::span<const uint8_t> image) override;

        void draw() const;

      private:
        IDirect3DDevice9* device_{};
        CComPtr<ID3DXSprite> sprite_{};
        CComPtr<IDirect3DTexture9> texture_{};
    };
}
