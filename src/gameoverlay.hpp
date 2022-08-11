#pragma once
#include <functional>

namespace gameoverlay
{
	/*****************************************************************************
	 *
	 ****************************************************************************/

	class register_backend
	{
	public:
		register_backend(std::function<void()> backend_initializer);
	};
}
