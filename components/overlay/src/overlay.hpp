#pragma once

#include "renderer_handler.hpp"

namespace gameoverlay
{
	class overlay
	{
	public:
		static void initialize();
		static void uninitialize();

	private:
		static std::unique_ptr<renderer_handler> handler;
	};
}
