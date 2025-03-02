#include "overlay.hpp"

#include <list>
#include <string>

#include <backend.hpp>

#include <backend_d3d9.hpp>
#include <backend_dxgi.hpp>
#include <backend_opengl.hpp>

#include <utils/concurrency.hpp>

#include <cef/cef_ui.hpp>

namespace gameoverlay
{
    namespace
    {
        class buffer
        {
          public:
            buffer(const dimensions dim = {})
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

        struct overlay : utils::object, browser_handler
        {
            using concurrent_buffer = utils::concurrency::container<buffer>;
            using shared_buffer = std::shared_ptr<concurrent_buffer>;

            shared_buffer buffer_{std::make_shared<concurrent_buffer>()};
            std::list<std::unique_ptr<backend>> backends{};

            cef_ui ui{*this};

            dimensions get_dimensions() override
            {
                return this->buffer_->access<dimensions>([](const buffer& b) {
                    return b.get_dimensions(); //
                });
            }

            void paint(const std::span<const uint8_t> data, const dimensions dim) override
            {
                return this->buffer_->access([&](buffer& b) {
                    auto image_buffer = b.get_buffer();

                    if (b.get_dimensions() != dim             //
                        || data.size() != image_buffer.size() //
                        || data.size() != dim.get_area() * 4)
                    {
                        return;
                    }

                    for (size_t i = 0; i < dim.get_area(); ++i)
                    {
                        auto* dest_pixel = image_buffer.data() + (i * 4);
                        auto* src_pixel = data.data() + (i * 4);

                        dest_pixel[0] = src_pixel[2];
                        dest_pixel[1] = src_pixel[1];
                        dest_pixel[2] = src_pixel[0];
                        dest_pixel[3] = src_pixel[3];
                    }
                });
            }

            overlay()
            {
                backends.emplace_back(d3d9::create_backend(this->make_handler()));
                backends.emplace_back(dxgi::create_backend(this->make_handler()));
                backends.emplace_back(opengl::create_backend(this->make_handler()));
            }

            backend::owned_handler make_handler()
            {
                struct overlay_renderer_handler : renderer::handler
                {
                    shared_buffer buffer_{};

                    overlay_renderer_handler(shared_buffer b)
                        : buffer_(std::move(b))
                    {
                    }

                    ~overlay_renderer_handler() override
                    {
                        OutputDebugStringA("Destroying renderer");
                    }

                    void on_frame(const renderer&, canvas& c) override
                    {
                        this->buffer_->access([&](buffer& b) {
                            const auto target_dim = c.get_dimensions();
                            if (target_dim != b.get_dimensions())
                            {
                                b.resize(target_dim);
                                return;
                            }

                            c.paint(b.get_buffer());
                        });
                    }
                };

                struct overlay_backend_handler : backend::handler
                {
                    shared_buffer buffer_{};

                    overlay_backend_handler(shared_buffer b)
                        : buffer_(std::move(b))
                    {
                    }

                    std::unique_ptr<renderer::handler> create_renderer_handler() override
                    {
                        return std::make_unique<overlay_renderer_handler>(this->buffer_);
                    }

                    void on_new_renderer(renderer& r) override
                    {
                        OutputDebugStringA(
                            ("New renderer: " + std::to_string(static_cast<int>(r.get_backend_type()))).data());
                    }
                };

                return std::make_unique<overlay_backend_handler>(this->buffer_);
            }
        };
    }

    std::unique_ptr<utils::object> create_overlay()
    {
        return std::make_unique<overlay>();
    }
}
