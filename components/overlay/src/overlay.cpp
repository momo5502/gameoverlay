#include "std_include.hpp"
#include "overlay.hpp"

using namespace literally::library;

namespace gameoverlay
{
	std::unique_ptr<renderer_handler> overlay::handler;
	std::unique_ptr<cef_ui> overlay::ui;

	size_t test_size = 0;

	void overlay::initialize()
	{
		overlay::handler = std::make_unique<renderer_handler>();
		overlay::handler->on_renderer_available([]()
		{
			overlay::ui = std::make_unique<cef_ui>();

			overlay::handler->get_renderer()->iface->register_frame_callback([]()
			{
				if (!overlay::handler) return;

				auto renderer = overlay::handler->get_renderer();
				auto canvas = renderer->iface->get_canvas();
				if (canvas && canvas->is_available())
				{
					if (!overlay::ui) return;

					auto app = overlay::ui->get_app();
					if (!app) return;

					auto client = app->get_client();
					if (!client) return;

					client->set_canvas(canvas);

					auto canvas_size = canvas->get_width() | (canvas->get_height() << 16);
					if (test_size != canvas_size)
					{
						test_size = canvas_size;
						client->trigger_resize();
					}
				}
			});
		});
	}

	void overlay::uninitialize()
	{
		overlay::handler = {};
		overlay::ui = {};
	}
}
