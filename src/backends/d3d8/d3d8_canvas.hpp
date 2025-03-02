#pragma once

#include <canvas.hpp>
#include "d3d8_win.hpp"

namespace gameoverlay::d3d8
{
    class d3d8_canvas : public fixed_canvas
    {
      public:
        d3d8_canvas(IDirect3DDevice8* device, uint32_t width, uint32_t height);
        ~d3d8_canvas() override = default;

        void paint(std::span<const uint8_t> image) override;

        void draw() const;

      private:
        IDirect3DDevice8* device_{};
        CComPtr<ID3DXSprite> sprite_{};
        CComPtr<IDirect3DTexture8> texture_{};
    };
}
