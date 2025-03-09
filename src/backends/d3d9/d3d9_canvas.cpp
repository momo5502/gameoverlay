#include "d3d9_canvas.hpp"

#include <cassert>
#include <stdexcept>

#include <utils/finally.hpp>

namespace gameoverlay::d3d9
{
    d3d9_canvas::d3d9_canvas(IDirect3DDevice9* device, const uint32_t width, const uint32_t height)
        : fixed_canvas(width, height),
          device_(device)
    {
        if (FAILED(this->device_->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
                                                &this->texture_, nullptr)))
        {
            throw std::runtime_error("Failed to create texture");
        }

        if (FAILED(this->device_->CreateStateBlock(D3DSBT_ALL, &this->state_block_)))
        {
            throw std::runtime_error("Failed to create state block");
        }
    }

    void d3d9_canvas::paint(const std::span<const uint8_t> image)
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

    void d3d9_canvas::draw() const
    {
        this->state_block_->Capture();
        const auto _ = utils::finally([&] {
            this->state_block_->Apply(); //
        });

        this->device_->BeginScene();

        this->device_->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
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

        this->device_->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
        this->device_->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

        this->device_->SetPixelShader(nullptr);
        this->device_->SetVertexShader(nullptr);

        struct Vertex
        {
            float x, y, z, rhw;
            float tu, tv;
        };

        const Vertex vertices[4] = {{0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
                                    {static_cast<float>(this->get_width() - 1), 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
                                    {0.0f, static_cast<float>(this->get_height() - 1), 0.0f, 1.0f, 0.0f, 1.0f},
                                    {static_cast<float>(this->get_width() - 1),
                                     static_cast<float>(this->get_height() - 1), 0.0f, 1.0f, 1.0f, 1.0f}};

        this->device_->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
        this->device_->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(Vertex));

        this->device_->EndScene();
    }
}
