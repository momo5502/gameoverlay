#include "std_include.hpp"

#include <literally/io.hpp>
#include <literally/library.hpp>

using namespace literally;

#include <irenderer.hpp>

#include "renderer_handler.hpp"

namespace gameoverlay
{
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

	renderer_handler::renderer_handler()
	{
		for (auto& lib : "./backend/"_files.filter(".*\\.dll"))
		{
			auto new_renderer = std::make_shared<renderer>(dynlib{ lib });
			if (*new_renderer)
			{
				this->renderers.push_back(new_renderer);
			}
		}
	}
}
