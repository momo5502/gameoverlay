#include "overlay.hpp"

#include <list>
#include <string>

#include <backend.hpp>

#include <backend_d3d9.hpp>
#include <backend_dxgi.hpp>
#include <backend_opengl.hpp>

#include <utils/concurrency.hpp>

#include <web_ui.hpp>

#include "frame_buffer.hpp"

namespace gameoverlay
{
    namespace
    {
        struct overlay : utils::object, web_ui_handler
        {
            using concurrent_buffer = utils::concurrency::container<frame_buffer>;
            using shared_buffer = std::shared_ptr<concurrent_buffer>;

            shared_buffer buffer_{std::make_shared<concurrent_buffer>()};
            std::list<std::unique_ptr<backend>> backends{};

            web_ui ui{};

            dimensions get_dimensions() override
            {
                return this->buffer_->access<dimensions>([](const frame_buffer& b) {
                    return b.get_dimensions(); //
                });
            }

            void paint(const std::span<const uint8_t> data, const dimensions dim) override
            {
                return this->buffer_->access([&](frame_buffer& b) {
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

                ui.create_browser(*this, "file:///C:/Users/mauri/Desktop/gameoverlay/src/data/main.html");
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
                        this->buffer_->access([&](frame_buffer& b) {
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
