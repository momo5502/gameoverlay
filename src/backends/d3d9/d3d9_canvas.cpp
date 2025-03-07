#include "d3d9_canvas.hpp"

#include <cassert>
#include <stdexcept>

#include "utils/finally.hpp"
#include "utils/string.hpp"

namespace gameoverlay::d3d9
{
    d3d9_canvas::d3d9_canvas(IDirect3DDevice9* device, const uint32_t width, const uint32_t height)
        : fixed_canvas(width, height),
          device_(device)
    {
        if (FAILED(D3DXCreateSprite(this->device_, &this->sprite_)))
        {
            throw std::runtime_error("Failed to create sprite");
        }

        if (FAILED(this->device_->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
                                                &this->texture_, nullptr)))
        {
            throw std::runtime_error("Failed to create texture");
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

            for (size_t i = 0; i < width; ++i)
            {
                auto* dest_pixel = dest_row_start + (i * 4);
                auto* src_pixel = src_row_start + (i * 4);

                dest_pixel[0] = src_pixel[2];
                dest_pixel[1] = src_pixel[1];
                dest_pixel[2] = src_pixel[0];
                dest_pixel[3] = src_pixel[3];
            }
        }
    }

    void d3d9_canvas::draw() const
    {
        D3DVIEWPORT9 vp{};
        this->device_->GetViewport(&vp);

        this->device_->BeginScene();

        const D3DXVECTOR3 position(0.0, 0.0, 0.0f);

        this->sprite_->Begin(D3DXSPRITE_ALPHABLEND);
        this->sprite_->Draw(this->texture_, nullptr, nullptr, &position, 0xFFFFFFFF);
        this->sprite_->End();

        this->device_->EndScene();
    }
}
