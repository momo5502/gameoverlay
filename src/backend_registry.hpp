#pragma once
#include <memory>
#include "backend.hpp"

namespace gameoverlay::backend_registry
{
	void register_backend(std::unique_ptr<backend>&& b);

	void initialize();
	void on_window_destruction(HWND window);

	/*****************************************************************************
	 *
	 ****************************************************************************/

	template <typename T>
	class installer final
	{
		static_assert(std::is_base_of_v<backend, T>, "backend has invalid base class");

	public:
		installer()
		{
			register_backend(std::make_unique<T>());
		}
	};
}

#define REGISTER_BACKEND(name)                                         \
namespace                                                              \
{                                                                      \
	static ::gameoverlay::backend_registry::installer<name> __backend; \
}
