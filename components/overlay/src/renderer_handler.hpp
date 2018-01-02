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
		static void load_renderers();
		static void unload_renderers();

		static std::vector<std::shared_ptr<renderer>> renderers;
	};
}
