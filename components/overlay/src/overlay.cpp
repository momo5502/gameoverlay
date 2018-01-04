#include "std_include.hpp"
#include "overlay.hpp"

using namespace literally::library;

namespace gameoverlay
{
	std::unique_ptr<renderer_handler> overlay::handler;

	size_t test_size = 0;
	std::unique_ptr<uint8_t[]> test_buffer;

	void overlay::initialize()
	{
		overlay::handler = std::make_unique<renderer_handler>();
		overlay::handler->on_renderer_available([]()
		{
			overlay::handler->get_renderer()->iface->register_frame_callback([]()
			{
				auto renderer = overlay::handler->get_renderer();
				auto canvas = renderer->iface->get_canvas();
				if (canvas && canvas->is_available())
				{
					auto canvas_size = canvas->get_width() * canvas->get_height() * 4;
					if (test_size != canvas_size || !test_buffer)
					{
						test_size = canvas_size;
						test_buffer = std::make_unique<uint8_t[]>(canvas_size);

						auto ptr = test_buffer.get();
						for (size_t i = 0; i < test_size; ++i)
						{
							ptr[i] = static_cast<unsigned char>(i);
						}

						canvas->paint(ptr);
					}
				}
			});
		});
	}

	void overlay::uninitialize()
	{
		overlay::handler = {};
	}
}
