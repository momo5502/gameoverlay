#include "std_include.hpp"
#include "renderer_handler.hpp"

using namespace literally;

namespace gameoverlay
{
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

	void renderer_handler::on_renderer_available(std::function<void()> callback)
	{
		this->available_callback = callback;
		if (this->get_renderer()) this->available_callback();
	}

	void renderer_handler::set_main_renderer(renderer* available_renderer)
	{
		for (auto& renderer : this->renderers)
		{
			if (renderer.get() == available_renderer)
			{
				this->main_renderer = renderer;
				break;
			}

			renderer->iface->unregister_frame_callback();
		}

		this->renderers.clear();

		if (this->available_callback) this->available_callback();
	}

	std::shared_ptr<renderer> renderer_handler::get_renderer()
	{
		return this->main_renderer;
	}

	renderer_handler::renderer_handler()
	{
		dynlib self = dynlib::get_by_address(dynlib::get_by_address);
		auto search_path = std::experimental::filesystem::path(self.get_folder()) / "renderers/";

		for (auto& lib : files(search_path.generic_string()).filter(".*renderer_.*\\.dll"))
		{
			if (main_renderer) break;

			auto new_renderer = std::make_shared<renderer>(dynlib::load(lib));
			if (*new_renderer)
			{
				// Make sure to only pass a pointer to the renderer, not the shared_ptr object itself,
				// otherwise the renderer will hold a reference to itself and keep itself alive.
				auto renderer_ptr = new_renderer.get();
				new_renderer->iface->register_frame_callback([renderer_ptr, this]()
				{
					this->set_main_renderer(renderer_ptr);
				});

				this->renderers.push_back(new_renderer);
			}
		}
	}

	renderer_handler::~renderer_handler()
	{
	}
}
