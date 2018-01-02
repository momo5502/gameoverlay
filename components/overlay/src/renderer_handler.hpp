#pragma once

namespace gameoverlay
{
	class renderer
	{
	public:
		renderer(dynlib library);
		~renderer();

		operator bool();

		dynlib library;
		irenderer* iface = nullptr;
	};

	class renderer_handler
	{
	public:
		renderer_handler();

		std::vector<std::shared_ptr<renderer>> renderers;
	};
}
