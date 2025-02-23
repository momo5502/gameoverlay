#include "overlay.hpp"

#include <list>
#include <backend_d3d9.hpp>
#include <backend_opengl.hpp>

namespace gameoverlay
{
    namespace
    {
        struct overlay : utils::object
        {
            std::list<std::unique_ptr<backend>> backends{};

            overlay()
            {
                backends.emplace_back(d3d9::create_backend(this->make_handler()));
                backends.emplace_back(opengl::create_backend(this->make_handler()));
            }

            backend::owned_handler make_handler()
            {
                struct overlay_renderer_handler : renderer::handler
                {
                    std::vector<uint8_t> data{};

                    void on_frame(const renderer&, canvas& c) override
                    {
                        const auto needed_size = c.get_buffer_size();
                        if (this->data.size() != needed_size)
                        {
                            this->data.resize(needed_size, 0xFF);

                            for (auto& b : this->data)
                            {
                                b = static_cast<uint8_t>(rand());
                            }
                        }

                        c.paint(this->data);
                    }
                };

                struct overlay_backend_handler : backend::handler
                {
                    std::unique_ptr<renderer::handler> create_renderer_handler() override
                    {
                        return std::make_unique<overlay_renderer_handler>();
                    }

                    void on_new_renderer(renderer&) override
                    {
                        OutputDebugStringA("New renderer!");
                    }
                };

                return std::make_unique<overlay_backend_handler>();
            }
        };
    }

    std::unique_ptr<utils::object> create_overlay()
    {
        return std::make_unique<overlay>();
    }
}
