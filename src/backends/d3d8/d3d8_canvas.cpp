#include "d3d8_canvas.hpp"

#include <cassert>
#include <stdexcept>

#include <utils/finally.hpp>

namespace gameoverlay::d3d8
{
    d3d8_canvas::d3d8_canvas(IDirect3DDevice8* device, const uint32_t width, const uint32_t height)
        : fixed_canvas(width, height),
          device_(device)
    {
        if (FAILED(D3DXCreateSprite(this->device_, &this->sprite_)))
        {
            throw std::runtime_error("Failed to create sprite");
        }

        if (FAILED(this->device_->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
                                                &this->texture_)))
        {
            throw std::runtime_error("Failed to create texture");
        }
    }

    void d3d8_canvas::paint(const std::span<const uint8_t> image)
    {
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

        const auto bytes_per_pixel = static_cast<uint32_t>(locked_rect.Pitch) / this->get_width();

        if (bytes_per_pixel != 4)
        {
            assert(false && "Bad bytes_per_pixel value!");
            return;
        }

        memcpy(locked_rect.pBits, image.data(), std::min(image.size(), this->get_buffer_size()));
    }

    void d3d8_canvas::draw() const
    {
        this->device_->BeginScene();

        this->sprite_->Begin();
        this->sprite_->Draw(this->texture_, nullptr, nullptr, nullptr, 0.0f, nullptr, 0xFFFFFFFF);
        this->sprite_->End();

        this->device_->EndScene();
    }
}
