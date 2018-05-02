#include "std_include.hpp"
#include "overlay.hpp"

#include "cef/cef_ui.hpp"
#include "cef/cef_ui_app.hpp"
#include "cef/cef_ui_handler.hpp"

using namespace literally::library;

namespace gameoverlay
{
	std::unique_ptr<renderer_handler> overlay::handler;

	size_t test_size = 0;

	void overlay::initialize()
	{
		overlay::handler = std::make_unique<renderer_handler>();
		overlay::handler->on_renderer_available([]()
		{
			overlay::handler->get_renderer()->iface->register_frame_callback([]()
			{
				if (!overlay::handler) return;

				auto renderer = overlay::handler->get_renderer();
				auto canvas = renderer->iface->get_canvas();
				if (canvas && canvas->is_available())
				{
					auto ui_handler = cef_ui_handler::get_instance();
					if (ui_handler)
					{
						ui_handler->set_canvas(canvas);

						auto canvas_size = canvas->get_width() | (canvas->get_height() << 16);
						if (test_size != canvas_size)
						{
							test_size = canvas_size;
							ui_handler->trigger_resize();
						}
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
