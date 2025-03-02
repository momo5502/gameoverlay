#pragma once

#include <span>
#include <vector>
#include <dimensions.hpp>

namespace gameoverlay
{
    class frame_buffer
    {
      public:
        frame_buffer(const dimensions dim = {})
        {
            this->resize(dim);
        }

        void resize(const dimensions new_dimensions)
        {
            const auto old_dimensions = this->dimensions_;
            this->dimensions_ = new_dimensions;

            const auto old_data = std::move(this->data_);

            constexpr size_t pixel_size = 4;

            this->data_ = std::vector<uint8_t>(pixel_size * new_dimensions.width * new_dimensions.height);

            if (this->data_.empty() || old_data.empty())
            {
                return;
            }

            for (uint32_t row = 0; row < std::min(new_dimensions.height, old_dimensions.height); ++row)
            {
                auto* old_row = old_data.data() + pixel_size * row * old_dimensions.width;
                auto* new_row = this->data_.data() + pixel_size * row * new_dimensions.width;

                memcpy(new_row, old_row, pixel_size * std::min(new_dimensions.width, old_dimensions.width));
            }
        }

        dimensions get_dimensions() const
        {
            return dimensions_;
        }

        std::span<uint8_t> get_buffer()
        {
            return this->data_;
        }

        std::span<const uint8_t> get_buffer() const
        {
            return this->data_;
        }

      private:
        dimensions dimensions_{};
        std::vector<uint8_t> data_{};
    };
}
