#pragma once

#include <canvas.hpp>
#include "d3d9_win.hpp"

#include <stdexcept>
#include <utils/finally.hpp>

namespace gameoverlay::d3d_legacy
{
    template <typename d3d_legacy_traits>
    class d3d_legacy_canvas : public fixed_canvas
    {
      public:
        CLASS_DISABLE_COPY_AND_MOVE(d3d_legacy_canvas);

        using traits = d3d_legacy_traits;

        d3d_legacy_canvas(typename traits::device& device, const uint32_t width, const uint32_t height)
            : fixed_canvas(width, height),
              device_(&device),
              texture_(traits::create_texture_func(device, width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8,
                                                   D3DPOOL_DEFAULT)),
              scope_(device)
        {
        }

        ~d3d_legacy_canvas() override = default;

        void paint(const std::span<const uint8_t> image) override
        {
            if (image.size() != this->get_buffer_size())
            {
                return;
            }

            D3DSURFACE_DESC desc{};
            this->texture_->GetLevelDesc(0, &desc);

            if ((desc.Usage & D3DUSAGE_DYNAMIC) != D3DUSAGE_DYNAMIC)
            {
                return;
            }

            D3DLOCKED_RECT locked_rect{};
            if (FAILED(this->texture_->LockRect(0, &locked_rect, NULL, 0)))
            {
                return;
            }

            const auto _ = utils::finally([&] {
                this->texture_->UnlockRect(0); //
            });

            const auto width = this->get_width();
            const auto height = this->get_height();

            for (size_t row = 0; row < height; ++row)
            {
                auto* dest_row_start = static_cast<uint8_t*>(locked_rect.pBits) + (row * locked_rect.Pitch);
                auto* src_row_start = image.data() + (row * width * 4);

                memcpy(dest_row_start, src_row_start, width * 4);
            }
        }

        void draw() const
        {
            this->scope_.enter();
            const auto _ = utils::finally([&] {
                this->scope_.leave(); //
            });

            this->device_->BeginScene();

            this->device_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
            this->device_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            this->device_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

            this->device_->SetTexture(0, this->texture_);
            this->device_->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
            this->device_->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            this->device_->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
            this->device_->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            this->device_->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            this->device_->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

            this->device_->SetPixelShader(0);
            this->device_->SetVertexShader(0);

            struct vertex
            {
                float x, y, z, rhw;
                float tu, tv;
            };

            const vertex vertices[4] = {
                {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
                {static_cast<float>(this->get_width() - 1), 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
                {0.0f, static_cast<float>(this->get_height() - 1), 0.0f, 1.0f, 0.0f, 1.0f},
                {static_cast<float>(this->get_width() - 1), static_cast<float>(this->get_height() - 1), 0.0f, 1.0f,
                 1.0f, 1.0f},
            };

            traits::set_fvf_func(*this->device_, D3DFVF_XYZRHW | D3DFVF_TEX1);
            this->device_->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(vertex));

            this->device_->EndScene();
        }

      private:
        CComPtr<typename traits::device> device_{};
        CComPtr<typename traits::texture2d> texture_{};
        typename traits::scope scope_;
    };
}
