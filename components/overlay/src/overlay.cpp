#include "std_include.hpp"
#include "renderer_handler.hpp"
#include "overlay.hpp"

namespace gameoverlay
{
	void overlay::initialize()
	{
		renderer_handler::load_renderers();
	}

	void overlay::uninitialize()
	{
		renderer_handler::unload_renderers();
	}
}
