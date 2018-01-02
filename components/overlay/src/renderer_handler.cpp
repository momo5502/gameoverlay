#include "std_include.hpp"
#include "renderer_handler.hpp"

using namespace literally;

namespace gameoverlay
{
	std::vector<std::shared_ptr<renderer>> renderer_handler::renderers;

	renderer::renderer(dynlib _library) : library(_library)
	{
		if (this->library)
		{
			if (auto creator = this->library.get<create_interface>("create_interface"))
			{
				this->iface = creator();
			}
		}
	}

	renderer::~renderer()
	{
		this->library.free();
	}

	renderer::operator bool()
	{
		return this->library && this->iface;
	}

	void renderer_handler::load_renderers()
	{
		for (auto& lib : "./renderers/"_files.filter("renderer_.*\\.dll"))
		{
			auto new_renderer = std::make_shared<renderer>(dynlib{ lib });
			if (*new_renderer)
			{
				renderer_handler::renderers.push_back(new_renderer);
			}
		}
	}

	void renderer_handler::unload_renderers()
	{
		renderer_handler::renderers.clear();
	}
}
