#pragma once

#include <literally/io.hpp>
#include <literally/library.hpp>
#include <irenderer.hpp>

namespace gameoverlay
{
	class renderer
	{
	public:
		renderer(literally::dynlib library);
		~renderer();

		operator bool();

		literally::dynlib library;
		irenderer* iface = nullptr;
	};

	class renderer_handler
	{
	public:
		renderer_handler();
		~renderer_handler();

		void on_renderer_available(std::function<void()> callback);

		std::shared_ptr<renderer> get_renderer();

	private:
		std::function<void()> available_callback;

		std::shared_ptr<renderer> main_renderer;
		std::vector<std::shared_ptr<renderer>> renderers;

		void set_main_renderer(renderer* available_renderer);
	};
}
