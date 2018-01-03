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
			if (auto creator = this->library.get<create_interface_t>("create_interface"))
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
		dynlib self = dynlib::get_by_address(renderer_handler::load_renderers);
		auto search_path = std::experimental::filesystem::path(self.get_folder()) / "renderers/";

		for (auto& lib : files(search_path.generic_string()).filter(".*renderer_.*\\.dll"))
		{
			auto new_renderer = std::make_shared<renderer>(dynlib::load(lib));
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
